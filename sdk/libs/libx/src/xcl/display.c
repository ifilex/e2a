/* Copyright (C) 2003 Jamey Sharp.
 * This file is licensed under the MIT license. See the file COPYING. */

#include "Xlibint.h"
#include "xclint.h"
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <stdio.h>

void _XFreeDisplayStructure(Display *dpy);

static XCBAuthInfo xauth;

static void *alloc_copy(const void *src, size_t *dstn, size_t n)
{
	void *dst;
	if(n <= 0)
	{
		*dstn = 0;
		return 0;
	}
	dst = Xmalloc(n);
	if(!dst)
		return 0;
	memcpy(dst, src, n);
	*dstn = n;
	return dst;
}

void XSetAuthorization(char *name, int namelen, char *data, int datalen)
{
	_XLockMutex(_Xglobal_lock);
	Xfree(xauth.name);
	Xfree(xauth.data);

	/* if either of these allocs fail, _XConnectXCB won't use this auth
	 * data, so we don't need to check it here. */
	xauth.name = alloc_copy(name, &xauth.namelen, namelen);
	xauth.data = alloc_copy(data, &xauth.datalen, datalen);

#if 0 /* but, for the paranoid among us: */
	if((namelen > 0 && !xauth.name) || (datalen > 0 && !xauth.data))
	{
		Xfree(xauth.name);
		Xfree(xauth.data);
		xauth.name = xauth.data = 0;
		xauth.namelen = xauth.datalen = 0;
	}
#endif

	_XUnlockMutex(_Xglobal_lock);
}

static int _XAsyncReplyHandler(Display *dpy, XCBGenericRep *buf)
{
	_XAsyncHandler *async, *next;
	_XSetLastRequestRead(dpy, (xGenericReply *) buf);
	for(async = dpy->async_handlers; async; async = next)
	{
		next = async->next;
		if(async->handler(dpy, (xReply *) buf, (char *) buf, sizeof(xReply) + (buf->length << 2), async->data))
			return 1;
	}
	return 0;
}

int _XConnectXCB(Display *dpy, _Xconst char *display, char **fullnamep, int *screenp)
{
	char *host;
	int n = 0;
	int use_global;
	XCBAuthInfo auth;
	XCBConnection *c;

	dpy->xcl = Xcalloc(1, sizeof(XCLPrivate));
	if(!dpy->xcl)
		return 0;

	dpy->fd = -1;
	if(XCBParseDisplay(display, &host, &n, screenp))
	{
		int len;
		dpy->fd = XCBOpen(host, n);
		len = strlen(host) + (1 + 20 + 1 + 20 + 1);
		*fullnamep = Xmalloc(len);
		snprintf(*fullnamep, len, "%s:%d.%d", host, n, *screenp);
	}
	free(host);

	if(dpy->fd == -1)
		return 0;

	_XLockMutex(_Xglobal_lock);
	use_global = xauth.name && xauth.data;
	if(use_global)
		auth = xauth;
	_XUnlockMutex(_Xglobal_lock);

	if(!use_global)
		XCBGetAuthInfo(dpy->fd, XCBNextNonce(), &auth);
	c = XCBConnect(dpy->fd, &auth);
	if(!use_global)
	{
		free(auth.name);
		free(auth.data);
	}

	XCBSetUnexpectedReplyHandler(c, (XCBUnexpectedReplyFunc) _XAsyncReplyHandler, dpy);
	dpy->xcl->connection = c;
	return c != 0;
}

static int init_pixmap_formats(Display *dpy, XCBConnection *c)
{
	int i;
	ScreenFormat *fmtdst;
	FORMAT *fmtsrc;
	dpy->nformats = XCBConnSetupSuccessRepPixmapFormatsLength(c->setup);

	/* Now iterate down setup information... */
	fmtdst = Xmalloc(dpy->nformats * sizeof(ScreenFormat));
	if(!fmtdst)
		return 0;
	dpy->pixmap_format = fmtdst;
	fmtsrc = XCBConnSetupSuccessRepPixmapFormats(c->setup);

	/* First decode the Z axis Screen format information. */
	for(i = dpy->nformats; i; --i, ++fmtsrc, ++fmtdst)
	{
		fmtdst->depth = fmtsrc->depth;
		fmtdst->bits_per_pixel = fmtsrc->bits_per_pixel;
		fmtdst->scanline_pad = fmtsrc->scanline_pad;
		fmtdst->ext_data = NULL;
	}
	return 1;
}

static int init_visuals(int len, VISUALTYPE *vpsrc, Visual **dst)
{
	Visual *vpdst;

	*dst = vpdst = Xmalloc(len * sizeof(Visual));
	if(!vpdst)
		return 0;

	for(; len; --len, ++vpsrc, ++vpdst)
	{
		vpdst->visualid		= vpsrc->visual_id.id;
		vpdst->class		= vpsrc->class;
		vpdst->bits_per_rgb	= vpsrc->bits_per_rgb_value;
		vpdst->map_entries	= vpsrc->colormap_entries;
		vpdst->red_mask		= vpsrc->red_mask;
		vpdst->green_mask	= vpsrc->green_mask;
		vpdst->blue_mask	= vpsrc->blue_mask;
		vpdst->ext_data		= NULL;
	}
	return 1;
}

static int init_depths(DEPTHIter dpsrc, Depth **dst)
{
	Depth *dpdst;

	*dst = dpdst = Xmalloc(dpsrc.rem * sizeof(Depth));
	if(!dpdst)
		return 0;

	/* for all depths on this screen. */
	for(; dpsrc.rem; DEPTHNext(&dpsrc), ++dpdst)
	{
		dpdst->depth		= dpsrc.data->depth;
		dpdst->nvisuals		= DEPTHVisualsLength(dpsrc.data);

		if(!init_visuals(dpdst->nvisuals, DEPTHVisuals(dpsrc.data), &dpdst->visuals))
			return 0;
	}
	return 1;
}

static int init_screens(Display *dpy, XCBConnection *c)
{
	Screen *spdst;
	XGCValues values;
	SCREENIter spsrc = XCBConnSetupSuccessRepRoots(c->setup);

	dpy->nscreens = spsrc.rem;

	spdst = Xmalloc(spsrc.rem * sizeof(Screen));
	if(!spdst)
		return 0;
	dpy->screens = spdst;

	/* Now go deal with each screen structure. */
	for(; spsrc.rem; SCREENNext(&spsrc), ++spdst)
	{
		spdst->display		= dpy;
		spdst->root 		= spsrc.data->root.xid;
		spdst->cmap 		= spsrc.data->default_colormap.xid;
		spdst->white_pixel	= spsrc.data->white_pixel;
		values.background	= spdst->white_pixel;
		spdst->black_pixel	= spsrc.data->black_pixel;
		values.foreground	= spdst->black_pixel;
		spdst->root_input_mask	= spsrc.data->current_input_masks;
		spdst->width		= spsrc.data->width_in_pixels;
		spdst->height		= spsrc.data->height_in_pixels;
		spdst->mwidth		= spsrc.data->width_in_millimeters;
		spdst->mheight		= spsrc.data->height_in_millimeters;
		spdst->min_maps		= spsrc.data->min_installed_maps;
		spdst->max_maps		= spsrc.data->max_installed_maps;
		spdst->backing_store	= spsrc.data->backing_stores;
		spdst->save_unders	= spsrc.data->save_unders;
		spdst->root_depth	= spsrc.data->root_depth;
		spdst->ndepths		= spsrc.data->allowed_depths_len;
		spdst->ext_data		= NULL;

		if(!init_depths(SCREENAllowedDepths(spsrc.data), &spdst->depths))
			return 0;

		spdst->root_visual = _XVIDtoVisual(dpy, spsrc.data->root_visual.id);

		/* Set up other stuff clients are always going to use. */
		spdst->default_gc = XCreateGC(dpy, spdst->root, GCForeground|GCBackground, &values);
		if(!spdst->default_gc)
			return 0;
	}
	return 1;
}

int _XConnectSetupXCB(Display *dpy)
{
	XCBConnection *c = dpy->xcl->connection;

	dpy->request = c->seqnum;

	dpy->proto_major_version	= c->setup->protocol_major_version;
	dpy->proto_minor_version	= c->setup->protocol_minor_version;
	dpy->release 			= c->setup->release_number;
	dpy->resource_base		= c->setup->resource_id_base;
	dpy->resource_mask		= c->setup->resource_id_mask;
	dpy->min_keycode		= c->setup->min_keycode.id;
	dpy->max_keycode		= c->setup->max_keycode.id;
	dpy->motion_buffer		= c->setup->motion_buffer_size;
	dpy->byte_order			= c->setup->image_byte_order;
	dpy->bitmap_unit		= c->setup->bitmap_format_scanline_unit;
	dpy->bitmap_pad			= c->setup->bitmap_format_scanline_pad;
	dpy->bitmap_bit_order		= c->setup->bitmap_format_bit_order;
	dpy->max_request_size		= c->setup->maximum_request_length;
	dpy->resource_shift		= 0;

	{
	    unsigned long mask;
	    for (mask = dpy->resource_mask; !(mask & 1); mask >>= 1)
		++dpy->resource_shift;
	}
	dpy->resource_max = (dpy->resource_mask >> dpy->resource_shift) - 5;

	{
		int len = XCBConnSetupSuccessRepVendorLength(c->setup);
		dpy->vendor = Xmalloc(len + 1);
		if(!dpy->vendor)
			return 0;
		memcpy(dpy->vendor, XCBConnSetupSuccessRepVendor(c->setup), len);
		dpy->vendor[len] = '\0';
	}

	if(!init_pixmap_formats(dpy, c))
		return 0;

	if(!init_screens(dpy, c))
		return 0;

	dpy->bigreq_size = XCBMaximumRequestLength(c);
	if(dpy->bigreq_size <= dpy->max_request_size)
		dpy->bigreq_size = 0;

	return 1;
}
