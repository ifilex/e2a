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

#ifndef __KSOCKET_H__
#define __KSOCKET_H__

#include "platform.h"
#include "kthread.h"

#define	K_SOCK_STREAM 1
#define K_SOCK_DGRAM 2
#define K_SOCK_RAW 3
#define K_SOCK_RDM 4
#define K_SOCK_SEQPACKET 5

#define	K_AF_UNIX      1
#define	K_AF_INET      2
#define K_AF_INET6     10
#define K_AF_NETLINK   16

#define	K_PF_UNIX      1
#define	K_PF_INET      2

#define	K_MSG_PEEK     0x2

#define K_SHUT_RD      0
#define K_SHUT_WR      1
#define K_SHUT_RDWR    2

#define K_SO_TYPE      3
#define	K_SO_ERROR     4
#define K_SO_SNDBUF    7
#define K_SO_RCVBUF    8
#define K_SO_PASSCRED  16
#define K_SO_PEERCRED  17
#define K_SO_ATTACH_FILTER  26

#define K_SOL_SOCKET  1
#define K_SCM_RIGHTS  1

U32 ksocket(struct KThread* thread, U32 domain, U32 type, U32 protocol);
U32 kbind(struct KThread* thread, U32 socket, U32 address, U32 len);
U32 kconnect(struct KThread* thread, U32 socket, U32 address, U32 len);
U32 klisten(struct KThread* thread, U32 socket, U32 backog);
U32 kaccept(struct KThread* thread, U32 socket, U32 address, U32 len);
U32 kgetsockname(struct KThread* thread, U32 socket, U32 address, U32 len);
U32 kgetpeername(struct KThread* thread, U32 socket, U32 address, U32 len);
U32 ksocketpair(struct KThread* thread, U32 af, U32 type, U32 protocol, U32 socks);
U32 ksend(struct KThread* thread, U32 socket, U32 buffer, U32 len, U32 flags);
U32 krecv(struct KThread* thread, U32 socket, U32 buffer, U32 len, U32 flags);
U32 kshutdown(struct KThread* thread, U32 socket, U32 how);
U32 ksetsockopt(struct KThread* thread, U32 socket, U32 level, U32 name, U32 value, U32 len);
U32 kgetsockopt(struct KThread* thread, U32 socket, U32 level, U32 name, U32 value, U32 len);
U32 ksendmsg(struct KThread* thread, U32 socket, U32 msg, U32 flags);
U32 ksendmmsg(struct KThread* thread, U32 socket, U32 address, U32 vlen, U32 flags);
U32 krecvmsg(struct KThread* thread, U32 socket, U32 msg, U32 flags);
U32 ksendto(struct KThread* thread, U32 socket, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len);
U32 krecvfrom(struct KThread* thread, U32 socket, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len);

U32 syscall_pipe(struct KThread* thread, U32 address);
U32 syscall_pipe2(struct KThread* thread, U32 address, U32 flags);

const char* socketAddressName(struct KThread* thread, U32 address, U32 len, char* result, U32 cbResult);
U32 isNativeSocket(struct KThread* thread, int desc);
#endif