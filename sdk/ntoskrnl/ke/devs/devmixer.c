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
#include "kprocess.h"
#include "oss.h"
#include "fsapi.h"

BOOL mixer_init(struct KProcess* process, struct FsOpenNode* node) {
    return TRUE;
}

S64 mixer_length(struct FsOpenNode* node) {
    return 0;
}

BOOL mixer_setLength(struct FsOpenNode* node, S64 len) {
    return FALSE;
}

S64 mixer_getFilePointer(struct FsOpenNode* node) {
    return 0;
}

S64 mixer_seek(struct FsOpenNode* node, S64 pos) {
    return 0;
}

U32 mixer_read(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 len) {
    return 0;
}

U32 mixer_write(struct KThread* thread, struct FsOpenNode* node, U32 address, U32 l) {
    return 0;
}

void mixer_close(struct FsOpenNode* node) {    
    node->func->free(node);
}

U32 mixer_ioctl(struct KThread* thread, struct FsOpenNode* node, U32 request) {
    U32 len = (request >> 16) & 0x3FFF;
    struct CPU* cpu = &thread->cpu;
    BOOL read = request & 0x40000000;
    BOOL write = request & 0x80000000;
    U32 i;

    switch (request & 0xFFFF) {
    case 0x5801: // SNDCTL_SYSINFO
        if (write) {
            U32 p = IOCTL_ARG1;
            for (i=0;i<len/4;i++)
                writed(thread, p+i*4, 2000+i);
            writeNativeString(thread, p, "OSS/Linux"); p+=32; // char product[32];		/* For example OSS/Free, OSS/Linux or OSS/Solaris */
            writeNativeString(thread, p, "4.0.0a"); p+=32; // char version[32];		/* For example 4.0a */
            writed(thread, p, 0x040000); p+=4; // int versionnum;		/* See OSS_GETVERSION */

            for (i=0;i<128;i++) {
                writeb(thread, p, i+100); p+=1; // char options[128];		/* Reserved */
            }

            writed(thread, p, 1); p+=4; // offset 196 int numaudios;		/* # of audio/dsp devices */
            for (i=0;i<8;i++) {
                writed(thread, p, 200+i); p+=4; // int openedaudio[8];		/* Bit mask telling which audio devices are busy */
            }

            writed(thread, p, 1); p+=4; // int numsynths;		/* # of availavle synth devices */
            writed(thread, p, 1); p+=4; // int nummidis;			/* # of available MIDI ports */
            writed(thread, p, 1); p+=4; // int numtimers;		/* # of available timer devices */
            writed(thread, p, 1); p+=4; // offset 244 int nummixers;		/* # of mixer devices */

            for (i=0;i<8;i++) {
                writed(thread, p, 0); p+=4; // int openedmidi[8];		/* Bit mask telling which midi devices are busy */
            }
            writed(thread, p, 1); p+=4; // offset 280 int numcards;			/* Number of sound cards in the system */
            writed(thread, p, 1); p+=4; // offset 284 int numaudioengines;		/* Number of audio engines in the system */
            writeNativeString(thread, p, "GPL"); p+=16; // char license[16];		/* For example "GPL" or "CDDL" */
            return 0;
        }
        break;
    case 0x5807: // SNDCTL_AUDIOINFO	
        if (write) {
            U32 p = IOCTL_ARG1;
            p+=4; // int dev; /* Audio device number */
            writeNativeString(thread, p, "BoxedWine mixer"); p+=64; // oss_devname_t name;
            writed(thread, p, 0); p+=4; // int busy; /* 0, OPEN_READ, OPEN_WRITE or OPEN_READWRITE */
            writed(thread, p, -1); p+=4; // int pid;
            writed(thread, p, PCM_CAP_OUTPUT); p+=4; // int caps;			/* PCM_CAP_INPUT, PCM_CAP_OUTPUT */
            writed(thread, p, 0); p+=4; // int iformats
            writed(thread, p, AFMT_U8 | AFMT_S16_LE | AFMT_S16_BE | AFMT_S8 | AFMT_U16_BE); p+=4; // int oformats;
            writed(thread, p, 0); p+=4; // int magic;			/* Reserved for internal use */
            writeNativeString(thread, p, ""); p+=64; // oss_cmd_t cmd;		/* Command using the device (if known) */
            writed(thread, p, 0); p+=4; // int card_number;
            writed(thread, p, 0); p+=4; // int port_number;
            writed(thread, p, 0); p+=4; // int mixer_dev;
            writed(thread, p, 0); p+=4; // int legacy_device;		/* Obsolete field. Replaced by devnode */
            writed(thread, p, 1); p+=4; // int enabled;			/* 1=enabled, 0=device not ready at this moment */
            writed(thread, p, 0); p+=4; // int flags;			/* For internal use only - no practical meaning */
            writed(thread, p, 0); p+=4; // int min_rate
            writed(thread, p, 0); p+=4; // max_rate;	/* Sample rate limits */
            writed(thread, p, 0); p+=4; // int min_channels
            writed(thread, p, 0); p+=4; // max_channels;	/* Number of channels supported */
            writed(thread, p, 0); p+=4; // int binding;			/* DSP_BIND_FRONT, etc. 0 means undefined */
            writed(thread, p, 0); p+=4; // int rate_source;
            writeNativeString(thread, p, ""); p+=64; // oss_handle_t handle;
            writed(thread, p, 0); p+=4; // unsigned int nrates
            for (i=0;i<20;i++) {
                writed(thread, p, 0); p+=4; // rates[20];	/* Please read the manual before using these */
            }
            writeNativeString(thread, p, ""); p+=32; // oss_longname_t song_name;	/* Song name (if given) */
            writeNativeString(thread, p, ""); p+=16; // oss_label_t label;		/* Device label (if given) */
            writed(thread, p, -1); p+=4; // int latency;			/* In usecs, -1=unknown */
            writeNativeString(thread, p, "/dev/dsp"); p+=16; // oss_devnode_t devnode;	/* Device special file name (absolute path) */
            writed(thread, p, 0); p+=4; // int next_play_engine;		/* Read the documentation for more info */
            writed(thread, p, 0); p+=4; // int next_rec_engine;		/* Read the documentation for more info */
            return 0;
        }        
    }
    return -K_ENODEV;
}

void mixer_setAsync(struct FsOpenNode* node, struct KProcess* process, FD fd, BOOL isAsync) {
    if (isAsync)
        kwarn("mixer_setAsync not implemented");
}

BOOL mixer_isAsync(struct FsOpenNode* node, struct KProcess* process) {
    return 0;
}

void mixer_waitForEvents(struct FsOpenNode* node, struct KThread* thread, U32 events) {
}

BOOL mixer_isWriteReady(struct KThread* thread, struct FsOpenNode* node) {
    return (node->flags & K_O_ACCMODE)!=K_O_RDONLY;
}

BOOL mixer_isReadReady(struct KThread* thread, struct FsOpenNode* node) {
    return (node->flags & K_O_ACCMODE)!=K_O_WRONLY;
}

U32 mixer_map(struct KThread* thread, struct FsOpenNode* node,  U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

BOOL mixer_canMap(struct FsOpenNode* node) {
    return FALSE;
}

struct FsOpenNodeFunc mixerAccess = {mixer_init, mixer_length, mixer_setLength, mixer_getFilePointer, mixer_seek, mixer_read, mixer_write, mixer_close, mixer_map, mixer_canMap, mixer_ioctl, mixer_setAsync, mixer_isAsync, mixer_waitForEvents, mixer_isWriteReady, mixer_isReadReady};