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

#ifndef __KSIGNAL_H__
#define __KSIGNAL_H__

#include "kthread.h"
#include "platform.h"

#define K_SIG_DFL 0
#define K_SIG_IGN 1

#define K_SIGHUP           1
#define K_SIGINT           2
#define K_SIGQUIT          3
#define K_SIGILL           4
#define K_SIGTRAP          5
#define K_SIGABRT          6
#define K_SIGIOT           6
#define K_SIGBUS           7
#define K_SIGFPE           8
#define K_SIGKILL          9
#define K_SIGUSR1         10
#define K_SIGSEGV         11
#define K_SIGUSR2         12
#define K_SIGPIPE         13
#define K_SIGALRM         14
#define K_SIGTERM         15
#define K_SIGSTKFLT       16
#define K_SIGCHLD         17
#define K_SIGCONT         18
#define K_SIGSTOP         19
#define K_SIGTSTP         20
#define K_SIGTTIN         21
#define K_SIGTTOU         22
#define K_SIGURG          23
#define K_SIGXCPU         24
#define K_SIGXFSZ         25
#define K_SIGVTALRM       26
#define K_SIGPROF         27
#define K_SIGWINCH        28
#define K_SIGIO           29
#define K_SIGPOLL         SIGIO
/*
#define SIGLOST         29
*/
#define K_SIGPWR          30
#define K_SIGSYS          31
#define K_SIGUNUSED       31

#define K_SS_ONSTACK 1
#define K_SS_DISABLE 4

#define K_SIG_BLOCK 0
#define K_SIG_UNBLOCK 1
#define K_SIG_SETMASK 2

#define K_SA_NOCLDSTOP  1          /* Don't send SIGCHLD when children stop.  */
#define K_SA_NOCLDWAIT  2          /* Don't create zombie on child death.  */
#define K_SA_SIGINFO    4          /* Invoke signal-catching function with
                                    three arguments instead of one.  */

#define K_SA_RESTORER     0x04000000u
#define K_SA_ONSTACK      0x08000000u
#define K_SA_RESTART      0x10000000u /* Restart syscall on signal return.  */
#define K_SA_NODEFER      0x40000000u
#define K_SA_RESETHAND    0x80000000u

#define K_SI_USER 0

#define K_POLL_IN         1   /* data input available */
#define K_POLL_OUT        2   /* output buffers available */
#define K_POLL_MSG        3   /* input message available */
#define K_POLL_ERR        4   /* i/o error */
#define K_POLL_PRI        5   /* high priority input available */
#define K_POLL_HUP        6   /* device disconnected */

#define CLD_EXITED 1

U32 syscall_rt_sigaction(struct KThread* thread, U32 sig, U32 act, U32 oact, U32 sigsetSize);
U32 syscall_rt_sigprocmask(struct KThread* thread, U32 how, U32 set, U32 oset, U32 sigsetSize);
U32 syscall_signalstack(struct KThread* thread, U32 ss, U32 oss);
U32 syscall_rt_sigsuspend(struct KThread* thread, U32 mask, U32 sigsetSize);
U32 syscall_signalfd4(struct KThread* thread, S32 fildes, U32 mask, U32 flags);

#endif