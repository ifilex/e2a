/* Copyright (C) 2003 Jamey Sharp.
 * This file is licensed under the MIT license. See the file COPYING. */

#include "Xlibint.h"
#include "xclint.h"
#include <X11/XCB/xcbint.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void handle_event(Display *dpy, XCBGenericEvent *e)
{
	if(!e)
		_XIOError(dpy);
	if(e->response_type == X_Error)
		_XError(dpy, (xError *) e);
	else
		_XEnq(dpy, (xEvent *) e);
	free(e);
}

int _XEventsQueued(Display *dpy, int mode)
{
	XCBConnection *c = XCBConnectionOfDisplay(dpy);
	int ret;
	if(mode == QueuedAfterFlush)
		_XFlush(dpy);
	while((ret = XCBEventQueueLength(c)) > 0)
		handle_event(dpy, XCBWaitEvent(c));
	if(ret < 0)
		_XIOError(dpy);
	return dpy->qlen;
}

/* _XReadEvents - Flush the output queue,
 * then read as many events as possible (but at least 1) and enqueue them
 */
void _XReadEvents(Display *dpy)
{
	_XFlush(dpy);
	handle_event(dpy, XCBWaitEvent(XCBConnectionOfDisplay(dpy)));
	_XEventsQueued(dpy, QueuedAfterReading);
}

/*
 * _XSend - Flush the buffer and send the client data. 32 bit word aligned
 * transmission is used, if size is not 0 mod 4, extra bytes are transmitted.
 */
void _XSend(Display *dpy, const char *data, long size)
{
	static char const pad[3];
	static const xReq _dummy_request;

	struct iovec iov[2];
	_XExtension *ext;
	int count = 0;

	if(dpy->bufptr != dpy->buffer)
	{
		iov[count].iov_base = (caddr_t) dpy->buffer;
		iov[count].iov_len = dpy->bufptr - dpy->buffer;
		++count;
	}
	if(data && size)
	{
		iov[count].iov_base = (caddr_t) data;
		iov[count].iov_len = size;
		++count;
	}

	for (ext = dpy->flushes; ext; ext = ext->next_flush) {
		int i;
		for(i = 0; i < count; ++i)
		{
			ext->before_flush(dpy, &ext->codes, iov[i].iov_base, iov[i].iov_len);
			if((iov[i].iov_len & 3) != 0)
				ext->before_flush(dpy, &ext->codes, pad, XCB_PAD(iov[i].iov_len));
		}
	}

	if(XCBLockWrite(XCBConnectionOfDisplay(dpy), iov, count) < 0)
		_XIOError(dpy);
	if(XCBFlush(XCBConnectionOfDisplay(dpy)) < 0)
		_XIOError(dpy);

	dpy->last_req = (char *) & _dummy_request;
	dpy->bufptr = dpy->buffer;
}

/*
 * _XFlush - Flush the X request buffer.  If the buffer is empty, no
 * action is taken.
 */
void _XFlush(Display *dpy)
{
	_XSend(dpy, 0, 0);

	_XEventsQueued(dpy, QueuedAfterReading);
}

/* _XAllocID - resource ID allocation routine. */
XID _XAllocID(Display *dpy)
{
	return XCBGenerateID(XCBConnectionOfDisplay(dpy));
}

/* _XAllocIDs - multiple resource ID allocation routine. */
void _XAllocIDs(Display *dpy, XID *ids, int count)
{
	int i;
	for (i = 0; i < count; i++)
		ids[i] = XAllocID(dpy);
}

/*
 * The hard part about this is that we only get 16 bits from a reply.
 * We have three values that will march along, with the following invariant:
 *	dpy->last_request_read <= rep->sequenceNumber <= dpy->request
 * We have to keep
 *	dpy->request - dpy->last_request_read < 2^16
 * or else we won't know for sure what value to use in events.  We do this
 * by forcing syncs when we get close.
 */
unsigned long _XSetLastRequestRead(Display *dpy, xGenericReply *rep)
{
	unsigned long newseq;
	unsigned int xcb_seqnum = XCBConnectionOfDisplay(dpy)->seqnum_read;

	/*
	 * KeymapNotify has no sequence number, but is always guaranteed
	 * to immediately follow another event, except when generated via
	 * SendEvent (hmmm).
	 */
	if ((rep->type & 0x7f) == KeymapNotify)
		return(dpy->last_request_read);

	newseq = (xcb_seqnum & ~((unsigned long)0xffff)) | rep->sequenceNumber;

	if (newseq > xcb_seqnum)
		newseq -= 0x10000;
	assert(newseq <= dpy->request);

	dpy->last_request_read = newseq;
	return(newseq);
}

static void _XFreeReplyData(Display *dpy, Bool force)
{
	if(!force && dpy->xcl->reply_consumed < dpy->xcl->reply_length)
		return;
	free(dpy->xcl->reply_data);
	dpy->xcl->reply_data = 0;
}

/*
 * _XReply - Wait for a reply packet and copy its contents into the
 * specified rep.
 * extra: number of 32-bit words expected after the reply
 * discard: should I discard data following "extra" words?
 */
Status _XReply(Display *dpy, xReply *rep, int extra, Bool discard)
{
	XCBGenericError *error;
	XCBConnection *c = XCBConnectionOfDisplay(dpy);

	free(dpy->xcl->reply_data);
	XCBAddReplyData(c, dpy->request);
	_XFlush(dpy);

	dpy->xcl->reply_data = XCBWaitSeqnum(c, dpy->request, &error);
	dpy->last_request_read = dpy->request;

	if(!dpy->xcl->reply_data)
	{
		_XExtension *ext;
		xError *err = (xError *) error;
		int ret_code;

		if(!error)
		{
			_XIOError(dpy);
			return 0;
		}

		/* do not die on "no such font", "can't allocate",
		   "can't grab" failures */
		switch(err->errorCode)
		{
			case BadName:
				switch(err->majorCode)
				{
					case X_LookupColor:
					case X_AllocNamedColor:
						return 0;
				}
				break;
			case BadFont:
				if(err->majorCode == X_QueryFont)
					return 0;
				break;
			case BadAlloc:
			case BadAccess:
				return 0;
		}

		/* 
		 * we better see if there is an extension who may
		 * want to suppress the error.
		 */
		for(ext = dpy->ext_procs; ext; ext = ext->next)
			if(ext->error && ext->error(dpy, err, &ext->codes, &ret_code))
				return ret_code;

		_XError(dpy, (xError *) error);
		return 0;
	}

	dpy->xcl->reply_consumed = sizeof(xReply) + (extra * 4);
	dpy->xcl->reply_length = sizeof(xReply) + (((XCBGenericRep *) dpy->xcl->reply_data)->length * 4);

	/* error: Xlib asks too much. give them what we can anyway. */
	if(dpy->xcl->reply_length < dpy->xcl->reply_consumed)
		dpy->xcl->reply_consumed = dpy->xcl->reply_length;

	memcpy(rep, dpy->xcl->reply_data, dpy->xcl->reply_consumed);
	_XFreeReplyData(dpy, discard);
	return 1;
}

int _XRead(Display *dpy, char *data, long size)
{
	assert(size >= 0);
	if(size == 0)
		return 0;
	assert(dpy->xcl->reply_data != 0);
	assert(dpy->xcl->reply_consumed + size <= dpy->xcl->reply_length);
	memcpy(data, dpy->xcl->reply_data + dpy->xcl->reply_consumed, size);
	dpy->xcl->reply_consumed += size;
	_XFreeReplyData(dpy, False);
	return 0;
}

/*
 * _XReadPad - Read bytes from the socket taking into account incomplete
 * reads.  If the number of bytes is not 0 mod 4, read additional pad
 * bytes.
 */
void _XReadPad(Display *dpy, char *data, long size)
{
	_XRead(dpy, data, size);
	dpy->xcl->reply_consumed += XCB_PAD(size);
	_XFreeReplyData(dpy, False);
}

/* Read and discard "n" 8-bit bytes of data */
void _XEatData(Display *dpy, unsigned long n)
{
	dpy->xcl->reply_consumed += n;
	_XFreeReplyData(dpy, False);
}
