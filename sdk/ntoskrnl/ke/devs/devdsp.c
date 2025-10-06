/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "winebox.h"
#include "platform.h"
#include "kerror.h"
#include "log.h"
#include "kprocess.h"
#include "kscheduler.h"
#include "oss.h"
#include "kalloc.h"
#include <math.h>
#include <SDL.h>
#include <string.h>
#include "fsapi.h"

struct DspData {
	BOOL isDspOpen;
    BOOL closePending;
	U32 dspFmt;
    U32 silent;
    SDL_AudioSpec want;
    SDL_AudioSpec got;
    U32 sameFormat;
	U32 dspFragSize;
	U32 bytesPerSecond;
#define DSP_BUFFER_SIZE 1024*256
	U8 dspBuffer[DSP_BUFFER_SIZE];
	S32 dspBufferLen;
	U32 dspBufferPos;
	struct KThread* dspWaitingToWriteThread;
	S32 pauseAtLen;
    struct DspData* next;
    BOOL needToBuildConversion;
};
#define pauseEnabled() (data->pauseAtLen!=0xFFFFFFFF)

void closeAudio(struct DspData* data) {
    if (data->isDspOpen) {
        SDL_CloseAudio();
        data->isDspOpen = 0;
    }
}

static int cvtBufLen;
static unsigned char* cvtBuf;
struct DspData* pendingClose;

void audioCallback(void *userdata, U8* stream, S32 len) {
    S32 available;
    S32 result;
	struct DspData* data = userdata;

	if (data->dspBufferLen == 0) {
        memset(stream, data->got.silence, len);
        data->silent = 1;
		return;
	}
    data->silent = 0;
	available = DSP_BUFFER_SIZE - data->dspBufferPos;
	if (available>data->dspBufferLen)
		available = data->dspBufferLen;
    
	if (pauseEnabled() && available > data->pauseAtLen) {
		available = data->pauseAtLen;
    }
    if (data->sameFormat) {
        if (available > len)
            available = len;
        memcpy(stream, data->dspBuffer + data->dspBufferPos, available);
        len -= available;
    } else {
        S32 requested;
        static SDL_AudioCVT cvt;

        if (data->needToBuildConversion) {
            SDL_BuildAudioCVT(&cvt, data->want.format, data->want.channels, data->want.freq, data->got.format, data->got.channels, data->got.freq);
            data->needToBuildConversion = 0;
        }
        requested = (S32)(ceil(len / cvt.len_ratio));
        if (available > requested)
            available = requested;
     
        cvt.len = available;
        if (cvtBufLen && cvtBufLen < cvt.len * cvt.len_mult) {
            cvtBufLen = 0;
            SDL_free(cvtBuf);
            cvtBuf = NULL;
        }
        if (!cvtBufLen) {
            cvtBufLen = cvt.len * cvt.len_mult;
            cvtBuf = (Uint8 *)SDL_malloc(cvt.len * cvt.len_mult);
        }
        cvt.buf = cvtBuf;
        memcpy(cvt.buf, data->dspBuffer + data->dspBufferPos, available);
        SDL_ConvertAudio(&cvt);
        memcpy(stream, cvt.buf, cvt.len_cvt);
        len -= cvt.len_cvt;
    }
    result=available;
	data->dspBufferPos += result;
	data->dspBufferLen -= result;
    if (pauseEnabled())
		data->pauseAtLen -= result;
	if (data->dspBufferPos == DSP_BUFFER_SIZE) {
		data->dspBufferPos = 0;
        audioCallback(userdata, stream+result, len);
        return;
    }
	if (data->dspWaitingToWriteThread)
		wakeThread(NULL, data->dspWaitingToWriteThread);
    if (len>0) {
        memset(stream+result, data->got.silence, len);
        //if (!data->closePending)
		//    printf("audio underflow: %d\n", len);
    }
}

void dspCheck();

U32 bytesPerSample(struct DspData* data) {
    if (data->want.format == AUDIO_S16LSB || data->want.format == AUDIO_S16MSB || data->want.format == AUDIO_U16LSB || data->want.format == AUDIO_U16MSB)
        return 2;
#ifdef SDL2
    else if (data->want.format == AUDIO_F32LSB)
        return 4;
#endif
    else
        return 1;
}

void openAudio(struct DspData* data) {
    //want.samples = 4096;
    data->want.callback = audioCallback;  // you wrote this function elsewhere.
	data->want.userdata = data;

    if (pendingClose)
        dspCheck();
    if (pendingClose) {
        closeAudio(pendingClose);
        kfree(pendingClose, 0);
        pendingClose = NULL;
    }
    if (SDL_OpenAudio(&data->want, &data->got) < 0) {
        printf("Failed to open audio: %s\n", SDL_GetError());
    } 
    if (data->want.freq != data->got.freq || data->want.channels != data->got.channels || data->want.format != data->got.format) {
        data->sameFormat = 0;
    } else {
        data->sameFormat = 1;
    }
	data->isDspOpen = 1;
    data->needToBuildConversion = 1;
    SDL_PauseAudio(0);
	data->pauseAtLen = 0xFFFFFFFF;
	data->dspFragSize = data->got.size;

    data->bytesPerSecond = data->want.freq * data->want.channels * bytesPerSample(data);
	printf("openAudio: freq=%d(got %d) format=%d(%x/got %x) channels=%d(got %d)\n", data->want.freq, data->got.freq, data->dspFmt, data->want.format, data->got.format, data->want.channels, data->got.channels);
}

BOOL dsp_init(struct KProcess* process, struct FsOpenNode* node) {
	struct DspData* data = kalloc(sizeof(struct DspData), 0);
	data->dspFmt = AFMT_U8;
	data->want.format = AUDIO_U8;
	data->want.channels = 1;
	data->dspFragSize = 5512;
	data->want.freq = 11025;
	node->data = data;
    return TRUE;
}

S64 dsp_length(struct FsOpenNode* node) {
    return 0;
}

BOOL dsp_setLength(struct FsOpenNode* node, S64 len) {
    return FALSE;
}

S64 dsp_getFilePointer(struct FsOpenNode* node) {
    return 0;
}

S64 dsp_seek(struct FsOpenNode* node, S64 pos) {
    return 0;
}

U32 dsp_read(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len) {
    return 0;
}

U32 dsp_write(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 l) {
    S32 len = (S32)l;
    S32 available;
    S32 result;
    U32 startPos;
	struct DspData* data = node->data;

    if (len+data->dspBufferLen>DSP_BUFFER_SIZE) {
		if (data->dspWaitingToWriteThread) {
			kpanic("%d tried to wait on writing to dsp but %d is already waiting", thread->id, data->dspWaitingToWriteThread);
        }
		data->dspWaitingToWriteThread = thread;
		addClearOnWake(thread, &data->dspWaitingToWriteThread);
        return -K_WAIT;
    }
	if (!data->isDspOpen)
        openAudio(data);
    SDL_LockAudio();
	startPos = data->dspBufferPos + data->dspBufferLen;
    if (startPos>=DSP_BUFFER_SIZE)
        startPos-=DSP_BUFFER_SIZE;
    available = DSP_BUFFER_SIZE-startPos;
    if (available>len)
        available = len;
	if (available>DSP_BUFFER_SIZE - data->dspBufferLen)
		available = DSP_BUFFER_SIZE - data->dspBufferLen;
	memcopyToNative(thread, address, (char*)data->dspBuffer + startPos, available);
	data->dspBufferLen += available;
    result = available;	
	if (len != available && data->dspBufferLen< DSP_BUFFER_SIZE) {
        len-=available;
        address+=available;
        // we wrapped around the the buffer
		startPos = data->dspBufferPos + data->dspBufferLen;
        if (startPos>=DSP_BUFFER_SIZE)
            startPos-=DSP_BUFFER_SIZE;
        if (startPos!=0) {
            kpanic("dsp_write logic error");
        }
        available = DSP_BUFFER_SIZE-startPos;
        if (available>len)
            available = len;
		if (available>DSP_BUFFER_SIZE - data->dspBufferLen)
			available = DSP_BUFFER_SIZE - data->dspBufferLen;
		memcopyToNative(thread, address, (char*)data->dspBuffer + startPos, available);
		data->dspBufferLen += available;
        result+=available;
    }
    SDL_UnlockAudio();	
    return result;
}

void dsp_close(struct FsOpenNode* node) {
	struct DspData* data = node->data;

    if (data->isDspOpen) {
        if (data->dspBufferLen) {
            struct DspData* p = pendingClose;
            pendingClose = data;
            pendingClose->next = p;
            data->closePending = 1;
        } else {
            closeAudio(data);
            kfree(data, 0);
        }
    }	
	node->data = 0;
    node->func->free(node);
}

U32 dsp_ioctl(struct KThread* thread, struct FsOpenNode* node, U32 request) {
    U32 len = (request >> 16) & 0x3FFF;
    struct CPU* cpu = &thread->cpu;
    //BOOL read = request & 0x40000000;
    BOOL write = request & 0x80000000;
    int i;
	struct DspData* data = node->data;

    switch (request & 0xFFFF) {
    case 0x5000: // SNDCTL_DSP_RESET
        return 0;
    case 0x5002:  // SNDCTL_DSP_SPEED 
        if (len!=4) {
            kpanic("SNDCTL_DSP_SPEED was expecting a len of 4");
        }
		data->want.freq = readd(thread, IOCTL_ARG1);
		if (write)
            writed(thread, IOCTL_ARG1, data->want.freq);
        return 0;
    case 0x5003: { // SNDCTL_DSP_STEREO
        U32 fmt;

        if (len!=4) {
            kpanic("SNDCTL_DSP_STEREO was expecting a len of 4");
        }
        fmt = readd(thread, IOCTL_ARG1);
		if (fmt != data->want.channels - 1) {
            closeAudio(data);
        }
        if (fmt == 0) {
            data->want.channels = 1;
        } else if (fmt == 1) {
            data->want.channels = 2;
        } else {
            kpanic("SNDCTL_DSP_STEREO wasn't expecting a value of %d", fmt);
        }
        if (write)
            writed(thread, IOCTL_ARG1, data->want.channels - 1);
        return 0;
    }
    case 0x5005: { // SNDCTL_DSP_SETFMT 
        U32 fmt;

        if (len!=4) {
            kpanic("SNDCTL_DSP_SETFMT was expecting a len of 4");
        }
        fmt = readd(thread, IOCTL_ARG1);
		if (fmt != AFMT_QUERY && fmt != data->dspFmt) {
            closeAudio(data);
        }
        switch (fmt) {
        case AFMT_QUERY:
            break;
        case AFMT_MU_LAW:
        case AFMT_A_LAW:
        case AFMT_IMA_ADPCM:
			data->dspFmt = AFMT_U8;
			data->want.format = AUDIO_U8;
            break;
        case AFMT_U8:
			data->dspFmt = AFMT_U8;
			data->want.format = AUDIO_U8;
            break;
        case AFMT_S16_LE:
			data->dspFmt = AFMT_S16_LE;
			data->want.format = AUDIO_S16LSB;
            break;
        case AFMT_S16_BE:
			data->dspFmt = AFMT_S16_BE;
            data->want.format = AUDIO_S16MSB;
            break;
        case AFMT_S8:
			data->dspFmt = AFMT_S8;
            data->want.format = AUDIO_S8;
            break;
        case AFMT_U16_LE:
			data->dspFmt = AFMT_U16_LE;
            data->want.format = AUDIO_U16LSB;
            break;
        case AFMT_U16_BE:
			data->dspFmt = AFMT_U16_BE;
            data->want.format = AUDIO_U16MSB;
            break;
        case AFMT_MPEG:
			data->dspFmt = AFMT_U8;
            data->want.format = AUDIO_U8;
            break;
#ifdef SDL2
        case AFMT_FLOAT:
            data->dspFmt = AFMT_FLOAT;
            data->want.format = AUDIO_F32LSB;
            break;
#endif
        }
        if (write)
			writed(thread, IOCTL_ARG1, data->dspFmt);
		else if (data->dspFmt != fmt) {
            kpanic("SNDCTL_DSP_SETFMT dspFmt!=fmt and can't write result");
        }
        return 0;
        }
    case 0x5006: {// SOUND_PCM_WRITE_CHANNELS
        U32 channels = readd(thread, IOCTL_ARG1);
		if (channels != data->want.channels) {
            closeAudio(data);
        }
        if (channels==1) {
            data->want.channels = 1;
        } else if (channels == 2) {
            data->want.channels = 2;
        } else {
            data->want.channels = 2;
        }
        if (write)
            writed(thread, IOCTL_ARG1, data->want.channels);
        return 0;
        }
    case 0x500A: // SNDCTL_DSP_SETFRAGMENT
		data->dspFragSize = 1 << (readd(thread, IOCTL_ARG1) & 0xFFFF);
        return 0;
    case 0x500B: // SNDCTL_DSP_GETFMTS
        writed(thread, IOCTL_ARG1, AFMT_U8 | AFMT_S16_LE | AFMT_S16_BE | AFMT_S8 | AFMT_U16_BE | AFMT_FLOAT);
        return 0;

		//typedef struct audio_buf_info {
		//	int fragments;     /* # of available fragments (partially usend ones not counted) */
		//	int fragstotal;    /* Total # of fragments allocated */
		//	int fragsize;      /* Size of a fragment in bytes */
		//
		//	int bytes;         /* Available space in bytes (includes partially used fragments) */
		//	/* Note! 'bytes' could be more than fragments*fragsize */
		//} audio_buf_info;

    case 0x500C: // SNDCTL_DSP_GETOSPACE
    {
        int len = data->dspBufferLen-(int)data->dspFragSize;
        if (len<0)
            len = 0;
		writed(thread, IOCTL_ARG1, ((DSP_BUFFER_SIZE - data->dspBufferLen) / data->dspFragSize)); // fragments
		writed(thread, IOCTL_ARG1 + 4, DSP_BUFFER_SIZE / data->dspFragSize);
		writed(thread, IOCTL_ARG1 + 8, data->dspFragSize);
		writed(thread, IOCTL_ARG1 + 12, DSP_BUFFER_SIZE - len);
        return 0;
    }
    case 0x500F: // SNDCTL_DSP_GETCAPS
        writed(thread, IOCTL_ARG1, DSP_CAP_TRIGGER);
        return 0;
    case 0x5010: // SNDCTL_DSP_SETTRIGGER
        if (readd(thread, IOCTL_ARG1) & PCM_ENABLE_OUTPUT) {
            SDL_PauseAudio(0);
			data->pauseAtLen = 0xFFFFFFFF;
        } else {            
			data->pauseAtLen = data->dspBufferLen;
			if (data->pauseAtLen == 0) {
                SDL_PauseAudio(0);
            }
        }
        return 0;
    case 0x5012: // SNDCTL_DSP_GETOPTR
        writed(thread, IOCTL_ARG1, 0); // Total # of bytes processed
        writed(thread, IOCTL_ARG1 + 4, 0); // # of fragment transitions since last time
        if (pauseEnabled()) {
			writed(thread, IOCTL_ARG1 + 8, data->pauseAtLen); // Current DMA pointer value
			if (data->pauseAtLen == 0) {
                SDL_PauseAudio(0);
            }
        } else {
			writed(thread, IOCTL_ARG1 + 8, data->dspBufferLen); // Current DMA pointer value
        }
        return 0;
    case 0x5016: // SNDCTL_DSP_SETDUPLEX
        return -K_EINVAL;
    case 0x5017: // SNDCTL_DSP_GETODELAY 
        if (write) {
			writed(thread, IOCTL_ARG1, data->dspBufferLen);
            return 0;
        }
    case 0x580C: // SNDCTL_ENGINEINFO
        if (write) {
            U32 p = IOCTL_ARG1;
            p+=4; // int dev; /* Audio device number */
            writeNativeString(thread, p, "BoxedWine audio"); p+=64; // oss_devname_t name;
            writed(thread, p, 0); p+=4; // int busy; /* 0, OPEN_READ, OPEN_WRITE or OPEN_READWRITE */
            writed(thread, p, -1); p+=4; // int pid;
            writed(thread, p, PCM_CAP_OUTPUT); p+=4; // int caps;			/* PCM_CAP_INPUT, PCM_CAP_OUTPUT */
            writed(thread, p, 0); p+=4; // int iformats
            writed(thread, p, AFMT_U8 | AFMT_S16_LE | AFMT_S16_BE | AFMT_S8 | AFMT_U16_BE | AFMT_FLOAT); p+=4; // int oformats;
            writed(thread, p, 0); p+=4; // int magic;			/* Reserved for internal use */
            writeNativeString(thread, p, ""); p+=64; // oss_cmd_t cmd;		/* Command using the device (if known) */
            writed(thread, p, 0); p+=4; // int card_number;
            writed(thread, p, 0); p+=4; // int port_number;
            writed(thread, p, 0); p+=4; // int mixer_dev;
            writed(thread, p, 0); p+=4; // int legacy_device;		/* Obsolete field. Replaced by devnode */
            writed(thread, p, 1); p+=4; // int enabled;			/* 1=enabled, 0=device not ready at this moment */
            writed(thread, p, 0); p+=4; // int flags;			/* For internal use only - no practical meaning */
			writed(thread, p, 11025); p += 4; // int min_rate
            writed(thread, p, 44100); p+=4; // max_rate;	/* Sample rate limits */
            writed(thread, p, 1); p+=4; // int min_channels
            writed(thread, p, 2); p+=4; // max_channels;	/* Number of channels supported */
            writed(thread, p, 0); p+=4; // int binding;			/* DSP_BIND_FRONT, etc. 0 means undefined */
            writed(thread, p, 0); p+=4; // int rate_source;
            writeNativeString(thread, p, ""); p+=32; // oss_handle_t handle;
            writed(thread, p, 0); p+=4; // unsigned int nrates
            for (i=0;i<20;i++) {
                writed(thread, p, 0); p+=4; // rates[20];	/* Please read the manual before using these */
            }
            writeNativeString(thread, p, ""); p+=64; // oss_longname_t song_name;	/* Song name (if given) */
            writeNativeString(thread, p, ""); p+=16; // oss_label_t label;		/* Device label (if given) */
            writed(thread, p, -1); p+=4; // int latency;			/* In usecs, -1=unknown */
            writeNativeString(thread, p, "dsp"); p+=16; // oss_devnode_t devnode;	/* Device special file name (absolute path) */
            writed(thread, p, 0); p+=4; // int next_play_engine;		/* Read the documentation for more info */
            writed(thread, p, 0); p+=4; // int next_rec_engine;		/* Read the documentation for more info */
            return 0;
        }        
    }
    return -K_ENODEV;
}

void dsp_setAsync(struct FsOpenNode* node, struct KProcess* process, FD fd, BOOL isAsync) {
    if (isAsync)
        kwarn("dsp_setAsync not implemented");
}

BOOL dsp_isAsync(struct FsOpenNode* node, struct KProcess* process) {
    return 0;
}

void dsp_waitForEvents(struct FsOpenNode* node, struct KThread* thread, U32 events) {
	struct DspData* data = node->data;

    if (events & K_POLLOUT) {
		if (data->dspWaitingToWriteThread)
			kpanic("%d tried to wait on a input read, but %d is already waiting.", thread->id, data->dspWaitingToWriteThread->id);
		data->dspWaitingToWriteThread = thread;
		addClearOnWake(thread, &data->dspWaitingToWriteThread);
    }
}

BOOL dsp_isWriteReady(struct KThread* thread, struct FsOpenNode* node) {
    return (node->flags & K_O_ACCMODE)!=K_O_RDONLY;
}

BOOL dsp_isReadReady(struct KThread* thread, struct FsOpenNode* node) {
    return (node->flags & K_O_ACCMODE)!=K_O_WRONLY;
}

U32 dsp_map(struct KThread* thread, struct FsOpenNode* node,  U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

BOOL dsp_canMap(struct FsOpenNode* node) {
    return FALSE;
}

struct FsOpenNodeFunc dspAccess = {dsp_init, dsp_length, dsp_setLength, dsp_getFilePointer, dsp_seek, dsp_read, dsp_write, dsp_close, dsp_map, dsp_canMap, dsp_ioctl, dsp_setAsync, dsp_isAsync, dsp_waitForEvents, dsp_isWriteReady, dsp_isReadReady};

void dspCheck() {
    if (pendingClose && !pendingClose->dspBufferLen && pendingClose->silent) {
        struct DspData* n = pendingClose->next;
        closeAudio(pendingClose);
        kfree(pendingClose, 0);
        pendingClose = n;
    }
}