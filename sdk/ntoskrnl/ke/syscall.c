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
#include "cpu.h"
#include "kthread.h"
#include "kprocess.h"
#include "memory.h"
#include "kmmap.h"
#include "decoder.h"
#include "kio.h"
#include "log.h"
#include "kscheduler.h"
#include "kerror.h"
#include "ksystem.h"
#include "ksignal.h"
#include "ksocket.h"
#include "kepoll.h"
#include "ksystem.h"
#include <stdarg.h>

U64 sysCallTime;
extern struct Block emptyBlock;
//#undef LOG_SYSCALLS
#undef LOG_OPS
#ifdef LOG_OPS
void logsyscall(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (logFile) {
        vfprintf(logFile, fmt, args);
        fprintf(logFile, "\n");
    }
    va_end(args);
}

#define LOGSYS logsyscall
#elif defined LOG_SYSCALLS
#define LOGSYS klog
#else
#define LOGSYS if (0) klog
#endif

#define __NR_exit 1
#define __NR_read 3
#define __NR_write 4
#define __NR_open 5
#define __NR_close 6
#define __NR_waitpid 7
#define __NR_link 9
#define __NR_unlink 10
#define __NR_execve 11
#define __NR_chdir 12
#define __NR_time 13
#define __NR_chmod 15
#define __NR_lseek 19
#define __NR_getpid 20
#define __NR_getuid 24
#define __NR_ptrace	26
#define __NR_alarm 27
#define __NR_utime 30
#define __NR_access 33
#define __NR_sync	36
#define __NR_kill 37
#define __NR_rename 38
#define __NR_mkdir 39
#define __NR_rmdir 40
#define __NR_dup 41
#define __NR_pipe 42
#define __NR_times 43
#define __NR_brk 45
#define __NR_getgid 47
#define __NR_geteuid 49
#define __NR_getegid 50
#define __NR_ioctl 54
#define __NR_setpgid 57
#define __NR_umask 60
#define __NR_dup2 63
#define __NR_getppid 64
#define __NR_getpgrp 65
#define __NR_setsid 66
#define __NR_setrlimit 75
#define __NR_getrusage 77
#define __NR_gettimeofday 78
#define __NR_symlink 83
#define __NR_readlink 85
#define __NR_mmap 90
#define __NR_munmap 91
#define __NR_ftruncate 93
#define __NR_fchmod 94
#define __NR_setpriority 97
#define __NR_statfs 99
#define __NR_ioperm	101
#define __NR_socketcall 102
#define __NR_setitimer 104
#define __NR_iopl	110
#define __NR_wait4 114
#define __NR_sysinfo 116
#define __NR_ipc 117
#define __NR_fsync 118
#define __NR_sigreturn 119
#define __NR_clone 120
#define __NR_uname 122
#define __NR_modify_ldt 123
#define __NR_mprotect 125
#define __NR_getpgid 132
#define __NR_fchdir 133
#define __NR__llseek 140
#define __NR_getdents 141
#define __NR_newselect 142
#define __NR_flock 143
#define __NR_msync 144
#define __NR_writev 146
#define __NR_fdatasync	148
#define __NR_mlock 150
#define __NR_sched_getparam 155
#define __NR_sched_getscheduler 157
#define __NR_sched_yield 158
#define __NR_sched_get_priority_max	159
#define __NR_sched_get_priority_min	160
#define __NR_nanosleep 162
#define __NR_mremap 163
#define __NR_vm86 166
#define __NR_poll 168
#define __NR_prctl 172
#define __NR_rt_sigaction 174
#define __NR_rt_sigprocmask 175
#define __NR_rt_sigsuspend 179
#define __NR_pread64 180
#define __NR_pwrite64 181
#define __NR_getcwd 183
#define __NR_sigaltstack 186
#define __NR_vfork 190
#define __NR_ugetrlimit 191
#define __NR_mmap2 192
#define __NR_ftruncate64 194
#define __NR_stat64 195
#define __NR_lstat64 196
#define __NR_fstat64 197
#define __NR_lchown32 198
#define __NR_getuid32 199
#define __NR_getgid32 200
#define __NR_geteuid32 201
#define __NR_getegid32 202
#define __NR_getgroups32 205
#define __NR_setgroups32 206
#define __NR_fchown32 207
#define __NR_setresuid32 208
#define __NR_getresuid32 209
#define __NR_setresgid32 210
#define __NR_getresgid32 211
#define __NR_chown32 212
#define __NR_setuid32 213
#define __NR_setgid32 214
#define __NR_mincore 218
#define __NR_madvise 219
#define __NR_getdents64 220
#define __NR_fcntl64 221
#define __NR_gettid 224
#define __NR_fsetxattr 228
#define __NR_fgetxattr 231
#define __NR_flistxattr 234
#define __NR_tkill 238
#define __NR_futex 240
#define __NR_sched_setaffinity	241
#define __NR_sched_getaffinity 242
#define __NR_set_thread_area 243
#define __NR_exit_group 252
#define __NR_epoll_create 254
#define __NR_epoll_ctl 255
#define __NR_epoll_wait 256
#define __NR_set_tid_address 258
#define __NR_clock_gettime 265
#define __NR_clock_getres 266
#define __NR_statfs64 268
#define __NR_fstatfs64 269
#define __NR_tgkill 270
#define __NR_utimes	271
#define __NR_fadvise64_64 272
#define __NR_inotify_init 291
#define __NR_inotify_add_watch 292
#define __NR_inotify_rm_watch 293
#define __NR_openat 295
#define __NR_mkdirat 296
#define __NR_fchownat 298
#define __NR_fstatat64 300
#define __NR_unlinkat 301
#define __NR_symlinkat 304
#define __NR_readlinkat 305
#define __NR_fchmodat 306
#define __NR_faccessat 307
#define __NR_set_robust_list 311
#define __NR_sync_file_range 314
#define __NR_getcpu 318
#define __NR_utimensat 320
#define __NR_signalfd4 327
#define __NR_epoll_create1 329
#define __NR_pipe2 331
#define __NR_prlimit64 340
//#define __NR_name_to_handle_at 341
//#define __NR_open_by_handle_at 342
#define __NR_sendmmsg 345

/*
#define __NR_setns 346
#define __NR_process_vm_readv 347
#define __NR_process_vm_writev 348
#define __NR_kcmp 349
#define __NR_finit_module 350
#define __NR_sched_setattr 351
#define __NR_sched_getattr 352
#define __NR_renameat2 353
#define __NR_seccomp 354
#define __NR_getrandom 355
#define __NR_memfd_create 356
#define __NR_bpf 357
#define __NR_execveat 358
#define __NR_socket 359
#define __NR_socketpair 360
#define __NR_bind 361
#define __NR_connect 362
#define __NR_listen 363
#define __NR_accept4 364
#define __NR_getsockopt 365
#define __NR_setsockopt 366
#define __NR_getsockname 367
#define __NR_getpeername 368
#define __NR_sendto 369
#define __NR_sendmsg 370
#define __NR_recvfrom 371
#define __NR_recvmsg 372
#define __NR_shutdown 373
#define __NR_userfaultfd 374
#define __NR_membarrier 375
#define __NR_mlock2 376
#define __NR_copy_file_range 377
#define __NR_preadv2 378
#define __NR_pwritev2 379
*/

#define ARG1 EBX
#define ARG2 ECX
#define ARG3 EDX
#define ARG4 ESI
#define ARG5 EDI
#define ARG6 EBP

#define SARG2 readd(thread, ARG2)
#define SARG3 readd(thread, ARG2+4)
#define SARG4 readd(thread, ARG2+8)
#define SARG5 readd(thread, ARG2+12)
#define SARG6 readd(thread, ARG2+16)
#define SARG7 readd(thread, ARG2+20)

void ksyscall(struct CPU* cpu, U32 eipCount) {
    struct KThread* thread = cpu->thread;
    struct KProcess* process = thread->process;
    S32 result=0;
    U64 startTime = getMicroCounter();

#ifdef LOG_SYSCALLS
    char buffer[1024];
    sprintf(buffer, "%s %d/%d ",process->name, thread->id, process->id);
    syscallToString(cpu, buffer+strlen(buffer));
#endif
    thread->inSysCall = 1;
#ifdef BOXEDWINE_VM
    if (thread->interrupted) {
        thread->interrupted = 0;
        EAX = -K_EINTR;
        return;
    }
#endif
    switch (EAX) {
    case __NR_exit:
        exitThread(cpu->thread, ARG1);
        break;
    case __NR_read:		
        result=syscall_read(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_write:	
        result=syscall_write(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_open:
        result=syscall_open(thread, process->currentDirectory, ARG1, ARG2);
        /*
        {
            char tmp[MAX_FILEPATH_LEN];
            printf("open: name=%s flags=%x result=%d\n", getNativeString(cpu->thread, ARG1, tmp, MAX_FILEPATH_LEN), ARG2, result);
        }
        */
        break;		
    case __NR_close:
        result=syscall_close(thread, ARG1);
        break;
    case __NR_waitpid:		
        result=syscall_waitpid(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_link:
        result = syscall_link(thread, ARG1, ARG2);
        break;
    case __NR_unlink:
        result =syscall_unlink(thread, ARG1);
        break;
    case __NR_execve:
        result = syscall_execve(thread, ARG1, ARG2, ARG3);		
#ifdef LOG_SYSCALLS
        sprintf(buffer, " commandline=%s", thread->process->commandLine);
#endif
        break;
    case __NR_chdir:
        result = syscall_chdir(thread, ARG1);
        break;
    case __NR_time:
        result = (U32)(getSystemTimeAsMicroSeconds() / 1000000l);
        if (ARG1)
            writed(thread, ARG1, result);
        break;
    case __NR_chmod:
        result = 0;
        break;
    case __NR_lseek:
        result = syscall_seek(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_getpid:
        result = thread->process->id;
        break;
    case __NR_getuid:
        result = process->userId;
        break;
    case __NR_ptrace:
        result = -K_EPERM;
        break;
    case __NR_alarm:
        result = syscall_alarm(thread, ARG1);
        break;
    case __NR_utime:
        result = 0;
        break;
    case __NR_access:
        result = syscall_access(thread, ARG1, ARG2);
        break;
    case __NR_sync:
        result = 0;
        break;
    case __NR_kill:
        result = syscall_kill(thread, ARG1, ARG2);
        break;
    case __NR_rename:
        result = syscall_rename(thread, ARG1, ARG2);
        break;
    case __NR_mkdir:
        result = syscall_mkdir(thread, ARG1, ARG2);
        break;		
    case __NR_rmdir:
        result = syscall_rmdir(thread, ARG1);
        break;		
    case __NR_dup:
        result = syscall_dup(thread, ARG1);
        break;
    case __NR_pipe:
        result = syscall_pipe(thread, ARG1);
#ifdef LOG_SYSCALLS
        sprintf(buffer+strlen(buffer), " (%d,%d)", readd(thread, ARG1), readd(thread, ARG1+4));
#endif
        break;
    case __NR_times:
        result = syscall_times(thread, ARG1);
        break;
    case __NR_brk:
        if (ARG1 > process->brkEnd) {
            U32 len = ARG1-process->brkEnd;
            U32 alreadyAllocated = ROUND_UP_TO_PAGE(process->brkEnd) - process->brkEnd;

            if (len<=alreadyAllocated) {
                process->brkEnd+=len;
            } else {
                U32 aligned = (process->brkEnd+4095) & 0xFFFFF000;

                if (syscall_mmap64(thread, aligned, len - alreadyAllocated, K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC, K_MAP_PRIVATE|K_MAP_ANONYMOUS|K_MAP_FIXED, -1, 0)==aligned) {
                    process->brkEnd+=len;
                }				
            }
        }
        result = process->brkEnd;
        break;
    case __NR_getgid:
        result = process->groupId;
        break;
    case __NR_geteuid:
        result = process->effectiveUserId;
        break;
    case __NR_getegid:
        result = process->effectiveGroupId;
        break;
    case __NR_ioctl:
        result = syscall_ioctl(thread, ARG1, ARG2);
        break;
    case __NR_setpgid:
        result = syscall_setpgid(thread, ARG1, ARG2);
        break;
    case __NR_umask:
        result = ARG1;
#ifdef _DEBUG
        kwarn("syscall umask not implemented");
#endif
        break;
    case __NR_dup2:
        result = syscall_dup2(thread, ARG1, ARG2);
        break;
    case __NR_getppid:
        result = thread->process->parentId;
        break;
    case __NR_getpgrp:
        result = thread->process->groupId;
        break;
    case __NR_setsid:
        result = 1; // :TODO:
#ifdef _DEBUG
        kwarn("__NR_setsid not implemented");
#endif
        break;
    case __NR_setrlimit:
        result = 0;
        break;
    case __NR_getrusage:
        result = syscall_getrusuage(thread, ARG1, ARG2);
        break;
    case __NR_gettimeofday:
        result = syscall_gettimeofday(thread, ARG1, ARG2);
        break;
    case __NR_symlink:
        result = syscall_symlink(thread, ARG1, ARG2);
        break;
    case __NR_readlink:
        result = syscall_readlink(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_mmap:
        result = syscall_mmap64(thread, readd(thread, ARG1), readd(thread, ARG1+4), readd(thread, ARG1+8), readd(thread, ARG1+12), readd(thread, ARG1+16), readd(thread, ARG1+20));
        break;
    case __NR_munmap:
        result = syscall_unmap(thread, ARG1, ARG2);
        break;
    case __NR_ftruncate:
        result = syscall_ftruncate64(thread, ARG1, ARG2);
        break;
    case __NR_fchmod:
        result = syscall_fchmod(thread, ARG1, ARG2);
        break;
    case __NR_setpriority:
        result = 0;
        break;
    case __NR_statfs:
        result = syscall_statfs(thread, ARG1, ARG2);
        break;
    case __NR_ioperm:
        result = syscall_ioperm(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_socketcall:
        switch (ARG1) {
            case 1: // SYS_SOCKET
                result = ksocket(thread, SARG2, SARG3 & 0xFF, SARG4);
                break;
            case 2: // SYS_BIND
                result = kbind(thread, SARG2, SARG3, SARG4);
                break;
            case 3: // SYS_CONNECT
                result = kconnect(thread, SARG2, SARG3, SARG4);
                break;
            case 4: // SYS_LISTEN				
                result = klisten(thread, SARG2, SARG3);
                break;
            case 5: // SYS_ACCEPT
                result = kaccept(thread, SARG2, SARG3, SARG4);
                break;			
            case 6: // SYS_GETSOCKNAME
                result = kgetsockname(thread, SARG2, SARG3, SARG4);
                break;			
            case 7: // SYS_GETPEERNAME
                result = kgetpeername(thread, SARG2, SARG3, SARG4);
                break;		
            case 8: // SYS_SOCKETPAIR
                result = ksocketpair(thread, SARG2, SARG3, SARG4, SARG5);
                break;
            case 9: // SYS_SEND
                result = ksend(thread, SARG2, SARG3, SARG4, SARG5);
                break;
            case 10: // SYS_RECV
                result = krecv(thread, SARG2, SARG3, SARG4, SARG5);
                break;
            case 11: // SYS_SENDTO
                result = ksendto(thread, SARG2, SARG3, SARG4, SARG5, SARG6, SARG7);
                break;
            case 12: // SYS_RECVFROM
                result = krecvfrom(thread, SARG2, SARG3, SARG4, SARG5, SARG6, SARG7);
                break;
            case 13: // SYS_SHUTDOWN
                result = kshutdown(thread, SARG2, SARG3);
                break;
            case 14: // SYS_SETSOCKOPT
                result = ksetsockopt(thread, SARG2, SARG3, SARG4, SARG5, SARG6);
                break;
            case 15: // SYS_GETSOCKOPT
                result = kgetsockopt(thread, SARG2, SARG3, SARG4, SARG5, SARG6);
                break;		
            case 16: // SYS_SENDMSG
                result = ksendmsg(thread, SARG2, SARG3, SARG4);
                break;
            case 17: // SYS_RECVMSG
                result = krecvmsg(thread, SARG2, SARG3, SARG4);
                break;
            //case 18: // SYS_ACCEPT4
            default:
                kpanic("Unknown socket syscall: %d",ARG1);
        }
        break;
    case __NR_setitimer:
        result = syscall_setitimer(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_iopl:
        result = syscall_iopl(thread, ARG1);
        break;
    case __NR_wait4:
        result=syscall_waitpid(thread, ARG1, ARG2, ARG3);
#ifdef _DEBUG
        if (ARG4) {
            kwarn("__NR_wait4 rusuage not implemented");
        }
#endif
        break;
    case __NR_sysinfo:
        result=syscall_sysinfo(thread, ARG1);
        break;
    case __NR_ipc:
        // ARG5 holds the pointer to be copied
        if (ARG1 == 21) { // IPCOP_shmat
            result = syscall_shmat(thread, ARG2, ARG5, ARG3, ARG4);
        }  else if (ARG1 == 22) { // IPCOP_shmdt
            result = syscall_shmdt(thread, ARG5);
        } else if (ARG1 == 23) { // IPCOP_shmget
            //result = -1; // :TODO: this crashes hsetroot
            result = syscall_shmget(thread, ARG2, ARG3, ARG4);
        } else if (ARG1 == 24) { // IPCOP_shmctl 
            result = syscall_shmctl(thread, ARG2, ARG3, ARG5);
        } else {
            kpanic("__NR_ipc op %d not implemented", ARG1);
        }
        break;		
    case __NR_fsync:
        result = 0; // :TODO:
        break;		
    case __NR_sigreturn:
        result = syscall_sigreturn(thread);
        break;
    case __NR_clone:
        result = syscall_clone(thread, ARG1, ARG2, ARG3, ARG4, ARG5);
        break;
    case __NR_uname:
        result = syscall_uname(thread, ARG1);
        break;
    case __NR_modify_ldt:
        result = syscall_modify_ldt(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_mprotect:
        result = syscall_mprotect(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_getpgid:
        result = syscall_getpgid(thread, ARG1);
        break;
    case __NR_fchdir:
        result = syscall_fchdir(thread, ARG1);
        break;
    case __NR__llseek: {
        S64 r64 = syscall_llseek(thread, ARG1, ((U64)ARG2)<<32|ARG3, ARG5);
        result = (S32)r64;
        if (ARG4) {
            writeq(thread, ARG4, r64);
        }
        break;
        }
    case __NR_getdents:
        result = syscall_getdents(thread, ARG1, ARG2, ARG3, FALSE);
        break;
    case __NR_newselect:		
        result = syscall_select(thread, ARG1, ARG2, ARG3, ARG4, ARG5);
        break;
    case __NR_flock:
        result = 0; // :TODO:
        break;
    case __NR_msync:
        result = syscall_msync(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_writev:		
        result=syscall_writev(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_fdatasync:
        result=0;
        break;
    case __NR_mlock:
        result=syscall_mlock(thread, ARG1, ARG2);
        break;
    case __NR_sched_getparam:
        result = 0;
        break;
    case __NR_sched_getscheduler:
        result = 0; // SCHED_OTHER
        break;
    case __NR_sched_yield:
        result = 0;
        threadDone(cpu);
        break;
    case __NR_sched_get_priority_max:
        result = 32;
        break;
    case __NR_sched_get_priority_min:
        result = 0;
        break;
    case __NR_nanosleep:
        result = threadSleep(thread, readd(thread, ARG1)*1000+readd(thread, ARG1+4)/1000000);
        break;
    case __NR_mremap:
        result = syscall_mremap(thread, ARG1, ARG2, ARG3, ARG4);
        break;
    case __NR_vm86:
        kpanic("Application tried to enter DOS mode (vm86).  BoxedWine does not support this.");
        break;
    case __NR_poll:
        result = syscall_poll(thread, ARG1, ARG2, ARG3);		
        break;
    case __NR_prctl:
        result = syscall_prctl(thread, ARG1);
        break;
    case __NR_rt_sigaction:
        result = syscall_rt_sigaction(thread, ARG1, ARG2, ARG3, ARG4);
        break;		
    case __NR_rt_sigprocmask:
        result = syscall_rt_sigprocmask(thread, ARG1, ARG2, ARG3, ARG4);
        break;
    case __NR_rt_sigsuspend:
        result = syscall_rt_sigsuspend(thread, ARG1, ARG2);
        break;
    case __NR_pread64:
        result = syscall_pread64(thread, ARG1, ARG2, ARG3, ARG4 | ((U64)ARG5) << 32);
        break;
    case __NR_pwrite64:
        result = syscall_pwrite64(thread, ARG1, ARG2, ARG3, ARG4 | ((U64)ARG5) << 32);
        break;
    case __NR_getcwd:
        result = syscall_getcwd(thread, ARG1, ARG2);
        break;
    case __NR_sigaltstack:
        result = syscall_signalstack(thread, ARG1, ARG2);
        break;
    case __NR_vfork:
        result = syscall_clone(thread, 0x01000000 |0x00200000 | 0x00004000, 0, 0, 0, 0);
        break;
    case __NR_ugetrlimit:
        result = syscall_ugetrlimit(thread, ARG1, ARG2);
        break;
    case __NR_mmap2:
        result = syscall_mmap64(thread, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6*4096l);
        break;
    case __NR_ftruncate64: {
        U64 len = ARG2 | ((U64)ARG3 << 32);
        result = syscall_ftruncate64(thread, ARG1, len);
        break;
    }
    case __NR_stat64:
        result = syscall_stat64(thread, ARG1, ARG2);
        break;
    case __NR_lstat64:
        result = syscall_lstat64(thread, ARG1, ARG2);
        break;
    case __NR_fstat64:
        result = syscall_fstat64(thread, ARG1, ARG2);
        break;
    case __NR_lchown32:
        result = 0;
        break;
    case __NR_getuid32:
        result = process->userId;
        break;
    case __NR_getgid32:
        result = process->groupId;
        break;
    case __NR_geteuid32:
        result = process->effectiveUserId;
        break;
    case __NR_getegid32:
        result = process->effectiveGroupId;
        break;
    case __NR_getgroups32:
        result = 1;
        if (ARG2!=0) {
            writed(thread, ARG2, process->groupId);
        }
        break;
    case __NR_setgroups32:
        if (ARG1) {
            process->groupId = readd(thread, ARG2);
        }
        result = 0;
        break;
    case __NR_fchown32:
        result = 0;
        break;
    case __NR_setresuid32:
        if (ARG1!=0xFFFFFFFF)
            process->userId = ARG1;
        if (ARG2!=0xFFFFFFFF)
            process->effectiveUserId = ARG2;
        result = 0;
        break;
    case __NR_getresuid32:
        if (ARG1)
            writed(thread, ARG1, process->userId);
        if (ARG2)
            writed(thread, ARG2, process->effectiveUserId);
        if (ARG3)
            writed(thread, ARG3, process->userId);
        result=0;
        break;
    case __NR_setresgid32:
        if (ARG1!=0xFFFFFFFF)
            process->groupId = ARG1;
        if (ARG2!=0xFFFFFFFF)
            process->effectiveGroupId = ARG2;
        result = 0;
        break;
    case __NR_getresgid32:
        if (ARG1)
            writed(thread, ARG1, process->groupId);
        if (ARG2)
            writed(thread, ARG2, process->effectiveGroupId);
        if (ARG3)
            writed(thread, ARG3, process->groupId);
        result=0;
        break;
    case __NR_chown32:
        result = 0;
        break;
    case __NR_setuid32:
        process->effectiveUserId = ARG1;
        result = 0;
        break;
    case __NR_setgid32:
        process->groupId = ARG1;
        result = 0;
        break;
    case __NR_mincore:
        result = syscall_mincore(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_madvise:
        result = 0;
        break;
    case __NR_getdents64:
        result = syscall_getdents(thread, ARG1, ARG2, ARG3, TRUE);
        break;
    case __NR_fcntl64:
        result = syscall_fcntrl(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_gettid:
        result = thread->id;
        break;
    case __NR_fsetxattr:
        result = -K_ENOTSUP;
        break;
    case __NR_fgetxattr:
        result = -K_ENOTSUP;
        break;
    case __NR_flistxattr:
        result = -K_ENOTSUP;
        break;
        /*
    case __NR_tkill:
        break;
        */
    case __NR_futex:
        result = syscall_futex(thread, ARG1, ARG2, ARG3, ARG4);
        break;
    case __NR_sched_setaffinity:
        result = 0;
        break;
    case __NR_sched_getaffinity:
#ifdef _DEBUG
        kwarn("__NR_sched_getaffinity not implemented");
#endif
        result = -1;
        break;
    case __NR_set_thread_area: {
        struct user_desc desc;

        readMemory(thread, (U8*)&desc, ARG1, sizeof(struct user_desc));
        if (desc.entry_number==-1) {
            U32 i;

            for (i=0;i<TLS_ENTRIES;i++) {
                if (thread->process->usedTLS[i]==0) {
                    desc.entry_number=i+TLS_ENTRY_START_INDEX;
                    break;
                }
            }
            if (desc.entry_number==-1) {
                kwarn("__NR_set_thread_area ran out of TLS slots");
                result = -K_ESRCH;
                break;
            }
            writeMemory(thread, ARG1, (U8*)&desc, sizeof(struct user_desc));
        }
        if (desc.base_addr!=0) {
            if (desc.entry_number<TLS_ENTRY_START_INDEX || desc.entry_number>=TLS_ENTRIES+TLS_ENTRY_START_INDEX) {
                result = -K_ESRCH;
                break;
            }
            thread->process->usedTLS[desc.entry_number-TLS_ENTRY_START_INDEX]=1;
            thread->tls[desc.entry_number-TLS_ENTRY_START_INDEX] = desc;            
        }
        result=0;		
        break;
    }
    case __NR_exit_group:
        result = syscall_exitgroup(thread, ARG1);		
        break;
    case __NR_epoll_create:
        result = syscall_epollcreate(thread, ARG1, 0);
        break;
    case __NR_epoll_ctl:
        result = syscall_epollctl(thread, ARG1, ARG2, ARG3, ARG4);
        break;
    case __NR_epoll_wait:
        result = syscall_epollwait(thread, ARG1, ARG2, ARG3, ARG4);
        break;
    case __NR_set_tid_address:
        thread->clear_child_tid = ARG1;
        result = thread->id;
        break;
    case __NR_clock_gettime:
        result = syscall_clock_gettime(thread, ARG1, ARG2);
        break;
    case __NR_clock_getres:
        writed(thread, ARG2, 0);
        writed(thread, ARG2+4, 1000000);
        result = 0;
        break;
    case __NR_statfs64:
        result = syscall_statfs64(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_fstatfs64:
        result = syscall_fstatfs64(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_tgkill:
        result = syscall_tgkill(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_utimes:
        result = syscall_utimes(thread, ARG1, ARG2);
        break;
    case __NR_fadvise64_64:
        result = 0;
        break;
    case __NR_inotify_init:
        result = -K_ENOSYS;
        break;
        /*
    case __NR_inotify_add_watch:
        break;
    case __NR_inotify_rm_watch:
        break;
        */
    case __NR_openat:
        result=syscall_openat(thread, ARG1, ARG2, ARG3);
        break;	
    case __NR_mkdirat:
        result = syscall_mkdirat(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_fchownat:
        result=0;
        break;	
    case __NR_fstatat64:
        result = syscall_fstatat64(thread, ARG1, ARG2, ARG3, ARG4);
        break;
    case __NR_unlinkat:
        result = syscall_unlinkat(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_symlinkat:
        result = syscall_symlinkat(thread, ARG1, ARG2, ARG3);
        break;
    case __NR_readlinkat:
        result = syscall_readlinkat(thread, ARG1, ARG2, ARG3, ARG4);
        break;
    case __NR_fchmodat:
        result = 0;
        break;
    case __NR_faccessat:
        result = syscall_faccessat(thread, ARG1, ARG2, ARG3, ARG4);
        break;
    case __NR_set_robust_list:
#ifdef _DEBUG
        kwarn("syscall __NR_set_robust_list not implemented");
#endif
        result = -1;
        break;
    case __NR_sync_file_range:
        result = 0;
        break;
        /*
    case __NR_getcpu:
        break;
        */
    case __NR_utimensat:
        result = syscall_utimesat(thread, ARG1, ARG2, ARG3, ARG4);
        break;	
        /*
    case __NR_signalfd4:
        result = syscall_signalfd4(thread, (S32)ARG1, ARG2, ARG3);
        break;
        */
    case __NR_epoll_create1:
        result = syscall_epollcreate(thread, 0, ARG1);
        break;
    case __NR_pipe2:
        result = syscall_pipe2(thread, ARG1, ARG2);
        break;
    case __NR_prlimit64:
        result = syscall_prlimit64(thread, ARG1, ARG2, ARG3, ARG4);
        break;
    case __NR_sendmmsg:
        result = ksendmmsg(thread, ARG1, ARG2, ARG3, ARG4);
        break;
    default:
        klog("Unknown syscall %d", EAX);
        result = -K_ENOSYS;
        break;
    }	
#ifdef LOG_SYSCALLS
    sprintf(buffer+strlen(buffer), " %d(%X)", result, result);
    LOGSYS(buffer);
#endif
#ifdef BOXEDWINE_VM
    if (result!=-K_CONTINUE) {
        U32 oldEAX = EAX;
        EAX = result;
        cpu->eip.u32+=eipCount;
        if (oldEAX == __NR_rt_sigprocmask) {
            runSignals(thread, thread);
        }
    }
#else
    if (result==-K_CONTINUE) {
        if (cpu->nextBlock!=&emptyBlock)
            cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
    } else if (result==-K_WAIT) {
        thread->waitSyscall = EAX;		
        waitThread(thread);		
    } else {
        U32 oldEAX = EAX;
        EAX = result;
        cpu->eip.u32+=eipCount;
        if (oldEAX == __NR_rt_sigprocmask) {
            runSignals(thread, thread);
        }
        if (cpu->nextBlock!=&emptyBlock)
            cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
    }	
#endif
    thread->inSysCall = 0;
    sysCallTime+=(getMicroCounter()-startTime);        
}

void OPCALL syscall_op(struct CPU* cpu, struct Op* op) {
    DONE();
    ksyscall(cpu, op->eipCount);    
}

void syscallToString(struct CPU* cpu, char* buffer) {
    struct KThread* thread = cpu->thread;
    char tmp[MAX_FILEPATH_LEN];
    char tmp2[MAX_FILEPATH_LEN];

    switch (EAX) {
    case __NR_exit: sprintf(buffer, "exit: %d", ARG1); break;
    case __NR_read: sprintf(buffer, "read: fd=%d buf=0x%X len=%d", ARG1, ARG2, ARG3); break;
    case __NR_write: sprintf(buffer, "write: fd=%d buf=0x%X len=%d", ARG1, ARG2, ARG3); break;
    case __NR_open: sprintf(buffer, "open: name=%s flags=%x", getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp)), ARG2); break;		
    case __NR_close: sprintf(buffer, "close: fd=%d", ARG1); break;
    case __NR_waitpid: sprintf(buffer, "waitpid: pid=%d status=%d options=%x", ARG1, ARG2, ARG3); break;
    case __NR_link: sprintf(buffer, "link: path1=%X(%s) path2=%X(%s)", ARG1, getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp)), ARG2, getNativeString(cpu->thread, ARG2, tmp2, sizeof(tmp2))); break;
    case __NR_unlink: sprintf(buffer, "unlink: path=%X(%s)", ARG1, getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp))); break;
    case __NR_execve: sprintf(buffer, "execve: path=%X(%s) argv=%X envp=%X", ARG1, getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp)), ARG2, ARG3); break;
    case __NR_chdir: sprintf(buffer, "chdir: path=%X(%s)", ARG1, getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp))); break;
    case __NR_time: sprintf(buffer, "time: tloc=%X", ARG1); break;
    case __NR_chmod: sprintf(buffer, "chmod: path=%X (%s) mode=%o", ARG1, getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp)), ARG2); break;
    case __NR_lseek: sprintf(buffer, "lseek: fildes=%d offset=%d whence=%d", ARG1, ARG2, ARG3); break;
    case __NR_getpid: sprintf(buffer, "getpid:"); break;
    case __NR_getuid: sprintf(buffer, "getuid:"); break;
    case __NR_alarm: sprintf(buffer, "alarm: seconds=%d", ARG1); break;
    case __NR_utime: sprintf(buffer, "utime: filename=%s times=%X", getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp)), ARG2); break;
    case __NR_access: sprintf(buffer, "access: filename=%s flags=0x%X", getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp)), ARG2); break;
    case __NR_sync: sprintf(buffer, "sync");
    case __NR_kill: sprintf(buffer, "kill: pid=%d signal=%d", ARG1, ARG2); break;
    case __NR_rename: sprintf(buffer, "rename: oldName=%X(%s) newName=%X(%s)", ARG1, getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp)), ARG2, getNativeString(cpu->thread, ARG2, tmp2, sizeof(tmp2))); break;
    case __NR_mkdir: sprintf(buffer, "mkdir: path=%X (%s) mode=%X", ARG1, getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp)), ARG2); break;
    case __NR_rmdir: sprintf(buffer, "rmdir: path=%X(%s)", ARG1, getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp))); break;
    case __NR_dup: sprintf(buffer, "dup: fildes=%d", ARG1); break;
    case __NR_pipe: sprintf(buffer, "pipe: fildes=%X", ARG1); break;
    case __NR_times: sprintf(buffer, "times: buf=%X", ARG1); break;
    case __NR_brk: sprintf(buffer, "brk: address=%.8X", ARG1); break;
    case __NR_getgid: sprintf(buffer, "getgid:"); break;
    case __NR_geteuid: sprintf(buffer, "geteuid:"); break;
    case __NR_getegid: sprintf(buffer, "getegid:"); break;
    case __NR_ioctl: sprintf(buffer, "ioctl: fd=%d request=%d", ARG1, ARG2); break;
    case __NR_setpgid: sprintf(buffer, "setpgid: pid=%d pgid=%d", ARG1, ARG2); break;
    case __NR_umask: sprintf(buffer, "umask: cmask=%X", ARG1); break;
    case __NR_dup2: sprintf(buffer, "dup2: fildes1=%d fildes2=%d", ARG1, ARG2); break;
    case __NR_getppid: sprintf(buffer, "getppid:"); break;
    case __NR_getpgrp: sprintf(buffer, "getpgrp:"); break;
    case __NR_setsid: sprintf(buffer, "setsid:"); break;
    case __NR_setrlimit: sprintf(buffer, "setrlimit:"); break;
    case __NR_getrusage: sprintf(buffer, "getrusage: who=%d usuage=%X", ARG1, ARG2); break;
    case __NR_gettimeofday: sprintf(buffer, "gettimeofday: tv=%X tz=%X", ARG1, ARG2); break;
    case __NR_symlink: sprintf(buffer, "symlink: path1=%X(%s) path2=%X(%s)", ARG1, getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp)), ARG2, getNativeString(cpu->thread, ARG2, tmp2, sizeof(tmp2))); break;
    case __NR_readlink: sprintf(buffer, "readlink: path=%X (%s) buffer=%X bufSize=%d", ARG1, getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp)), ARG1, ARG3); break;
    case __NR_mmap: sprintf(buffer, "mmap: address=%.8X len=%d prot=%X flags=%X fd=%d offset=%d", readd(cpu->thread, ARG1), readd(cpu->thread, ARG1+4), readd(cpu->thread, ARG1+8), readd(cpu->thread, ARG1+12), readd(cpu->thread, ARG1+16), readd(cpu->thread, ARG1+20));
    case __NR_munmap: sprintf(buffer, "munmap: address=%X len=%d", ARG1, ARG2); break;
    case __NR_ftruncate: sprintf(buffer, "ftruncate: address=%X len=%d", ARG1, ARG2); break;
    case __NR_fchmod: sprintf(buffer, "fchmod: fd=%d mod=%X", ARG1, ARG2); break;
    case __NR_setpriority: sprintf(buffer, "setpriority: which=%d, who=%d, prio=%d", ARG1, ARG2, ARG3); break;
    case __NR_statfs: sprintf(buffer, "fstatfs: path=%X(%s) buf=%X", ARG1, getNativeString(cpu->thread, ARG1, tmp, sizeof(tmp)), ARG2); break;
    case __NR_ioperm: sprintf(buffer, "ioperm: from=%X num=%d turn_on=%X", ARG1, ARG2, ARG3); break;
    case __NR_socketcall: {
        char tmp[4096];
        switch (ARG1) {
            case 1: sprintf(buffer, "SYS_SOCKET: domain=%d(%s) type=%d(%s) protocol=%d(%s)", SARG2, SARG2==K_AF_UNIX?"AF_UNIX":(SARG2==K_AF_INET)?"AF_INET":"", (SARG3 & 0xFF), (SARG3 & 0xFF)==K_SOCK_STREAM?"SOCK_STREAM":((SARG3 & 0xFF)==K_SOCK_DGRAM)?"AF_SOCK_DGRAM":"", SARG4, (SARG4 == 0)?"IPPROTO_IP":(SARG4==6)?"IPPROTO_TCP":(SARG4==17)?"IPPROTO_UDP":""); break;
            case 2: sprintf(buffer, "SYS_BIND: socket=%d address=%X(%s) len=%d", SARG2, SARG3, socketAddressName(thread, SARG3, SARG4, tmp, sizeof(tmp)), SARG4); break;
            case 3: sprintf(buffer, "SYS_CONNECT: socket=%d address=%X(%s) len=%d", SARG2, SARG3, socketAddressName(thread, SARG3, SARG4, tmp, sizeof(tmp)), SARG4); break;
            case 4: sprintf(buffer, "SYS_LISTEN: socket=%d backlog=%d", SARG2, SARG3); break;
            case 5: sprintf(buffer, "SYS_ACCEPT: socket=%d address=%X(%s) len=%d", SARG2, SARG3, socketAddressName(thread, SARG3, SARG4, tmp, sizeof(tmp)), SARG4); break;			
            case 6: sprintf(buffer, "SYS_GETSOCKNAME: socket=%d address=%X len=%d", SARG2, SARG3, SARG4); break;			
            case 7: sprintf(buffer, "SYS_GETPEERNAME: socket=%d address=%X len=%d", SARG2, SARG3, SARG4); break;		
            case 8: sprintf(buffer, "SYS_SOCKETPAIR: af=%d(%s) type=%d(%s) socks=%X", SARG2, SARG2==K_AF_UNIX?"AF_UNIX":(SARG2==K_AF_INET)?"AF_INET":"", SARG3, SARG3==K_SOCK_STREAM?"SOCK_STREAM":(SARG3==K_SOCK_DGRAM)?"AF_SOCK_DGRAM":"", SARG5); break;
            case 9: sprintf(buffer, "SYS_SEND: socket=%d buffer=%X len=%d flags=%X", SARG2, SARG3, SARG4, SARG5); break;
            case 10: sprintf(buffer, "SYS_RECV: socket=%d buffer=%X len=%d flags=%X", SARG2, SARG3, SARG4, SARG5); break;
            case 11: sprintf(buffer, "SYS_SENDTO: socket=%d buffer=%X len=%d flags=%X dest=%s", SARG2, SARG3, SARG4, SARG5, socketAddressName(thread, SARG6, SARG7, tmp, sizeof(tmp))); break;
            case 12: sprintf(buffer, "SYS_RECVFROM: socket=%d buffer=%X len=%d flags=%X address=%s", SARG2, SARG3, SARG4, SARG5, socketAddressName(thread, SARG6, SARG7, tmp, sizeof(tmp))); break;
            case 13: sprintf(buffer, "SYS_SHUTDOWN: socket=%d how=%d", SARG2, SARG3); break;
            case 14: sprintf(buffer, "SYS_SETSOCKOPT: socket=%d level=%d name=%d value=%d, len=%d", SARG2, SARG3, SARG4, SARG5, SARG6); break;
            case 15: sprintf(buffer, "SYS_GETSOCKOPT: socket=%d level=%d name=%d value=%d, len=%d", SARG2, SARG3, SARG4, SARG5, SARG6); break;		
            case 16: sprintf(buffer, "SYS_SENDMSG: socket=%d message=%X flags=%X", SARG2, SARG3, SARG4); break;
            case 17: sprintf(buffer, "SYS_RECVMSG: socket=%d message=%X flags=%X", SARG2, SARG3, SARG4); break;
            //case 18: // SYS_ACCEPT4
            default:
                kpanic("Unknown socket syscall: %d",ARG1);
        }
        break;
    }
    case __NR_setitimer: sprintf(buffer, "setitimer :which=%d newValue=%d(%d.%.06d) oldValue=%d", ARG1, ARG2, (ARG2?readd(thread, ARG2+8):0), (ARG2?readd(thread, ARG2+12):0), ARG3); break;
    case __NR_iopl: sprintf(buffer, "iopl: level=%1", ARG1); break;
    case __NR_wait4: sprintf(buffer, "wait4: pid=%d status=%d options=%x rusage=%X", ARG1, ARG2, ARG3, ARG4); break;
    case __NR_ipc:
        if (ARG1 == 21) { // IPCOP_shmat
            sprintf(buffer, "ipc: IPCOP_shmat shmid=%d shmaddr=%d shmflg=%X", ARG2, ARG5, ARG3);
        }  else if (ARG1 == 22) { // IPCOP_shmdt
            sprintf(buffer, "ipc IPCOP_shmdt shmaddr=%d", ARG5);
        } else if (ARG1 == 23) { // IPCOP_shmget
            sprintf(buffer, "ipc: IPCOP_shmget key=%d size=%d flags=%X", ARG2, ARG3, ARG4);
        } else if (ARG1 == 24) { // IPCOP_shmctl 
            sprintf(buffer, "ipc: IPCOP_shmctl shmid=%d cmd=%d buf=%X", ARG2, ARG3, ARG5);
        } else {
            kpanic("__NR_ipc op %d not implemented", ARG1);
        }
        break;		
    case __NR_fsync: sprintf(buffer, "fsync: fd=%d", ARG1); break;
    case __NR_sigreturn: sprintf(buffer, "sigreturn:"); break;
    case __NR_clone: sprintf(buffer, "clone: flags=%X child_stack=%X ptid=%X tls=%X ctid=%X", ARG1, ARG2, ARG3, ARG4, ARG5); break;
    case __NR_uname: sprintf(buffer, "uname: name=%.8X", ARG1); break;
    case __NR_modify_ldt: sprintf(buffer, "modify_ldt: func=%d ptr=%X(index=%d address=%X limit=%X flags=%X) count=%d", ARG1, ARG2, readd(thread, ARG2),  readd(thread, ARG2+4), readd(thread, ARG2+8), readd(thread, ARG2+12), ARG3); break;
    case __NR_mprotect: sprintf(buffer, "mprotect: address=%X len=%d prot=%X", ARG1, ARG2, ARG3); break;
    case __NR_getpgid: sprintf(buffer, "getpgid: pid=%d", ARG1); break;
    case __NR_fchdir: sprintf(buffer, "fchdir: fd=%d", ARG1); break;
    case __NR__llseek: sprintf(buffer, "llseek: fildes=%d offset=%.8X%.8X pResult=%X whence=%d", ARG1, ARG2, ARG3, ARG4, ARG5); break;
    case __NR_getdents: sprintf(buffer, "getdents: fd=%d dir=%X count=%d", ARG1, ARG2, ARG3); break;
    case __NR_newselect: sprintf(buffer, "newselect: nfd=%d readfds=%X writefds=%X errorfds=%X timeout=%d", ARG1, ARG2, ARG3, ARG4, ARG5); break;
    case __NR_flock: sprintf(buffer, "flock: fd=%d operation=%d", ARG1, ARG2); break;
    case __NR_msync: sprintf(buffer, "msync addr=%X length=%d flags=%X", ARG1, ARG2, ARG3); break;
    case __NR_writev: sprintf(buffer, "writev: filds=%d iov=0x%X iovcn=%d", ARG1, ARG2, ARG3); break;
    case __NR_fdatasync: sprintf(buffer, "fdatasync: fd=%d", ARG1); break;
    case __NR_mlock: sprintf(buffer, "mlock: address=0x%X len=%d", ARG1, ARG2); break;
    case __NR_sched_getparam: sprintf(buffer, "sched_getparam: pid=%d params=%X", ARG1, ARG2); break;
    case __NR_sched_getscheduler: sprintf(buffer, "sched_getscheduler: pid=%d params=%X", ARG1, ARG2); break;
    case __NR_sched_yield: sprintf(buffer, "yield:"); break;
    case __NR_sched_get_priority_max: sprintf(buffer, "sched_get_priority_max: policy=%d", ARG1);
    case __NR_sched_get_priority_min: sprintf(buffer, "sched_get_priority_min: policy=%d", ARG1);
    case __NR_nanosleep: sprintf(buffer, "nanosleep: req=%X(%d.%.09d sec)", ARG1, readd(thread, ARG1), readd(thread, ARG1+4)); break;
    case __NR_mremap: sprintf(buffer, "mremap: oldaddress=%x oldsize=%d newsize=%d flags=%X", ARG1, ARG2, ARG3, ARG4); break;
    case __NR_poll: sprintf(buffer, "poll: pfds=%X nfds=%d timeout=%X", ARG1, ARG2, ARG3); break;
    case __NR_prctl: sprintf(buffer, "prctl: options=%d", ARG1); break;
    case __NR_rt_sigaction: sprintf(buffer, "rt_sigaction: sig=%d act=%X oact=%X", ARG1, ARG2, ARG3); break;		
    case __NR_rt_sigprocmask: sprintf(buffer, "rt_sigprocmask: how=%d set=%X oset=%X", ARG1, ARG2, ARG3); break;
    case __NR_rt_sigsuspend: sprintf(buffer, "rt_sigsuspend: mask=%X", ARG1); break;
    case __NR_pread64: sprintf(buffer, "pread64: fd=%d buf=%X len=%d offset=%d", ARG1, ARG2, ARG3, ARG4); break;
    case __NR_pwrite64: sprintf(buffer, "pwrite64: fd=%d buf=%X len=%d offset=%d", ARG1, ARG2, ARG3, ARG4); break;
    case __NR_getcwd: sprintf(buffer, "getcwd: buf=%X size=%d (%s)", ARG1, ARG2, thread->process->currentDirectory); break;
    case __NR_sigaltstack: sprintf(buffer, "sigaltstack ss=%X oss=%X", ARG1, ARG2); break;
    case __NR_vfork: sprintf(buffer, "vfork:"); break;
    case __NR_ugetrlimit: sprintf(buffer, "ugetrlimit: resource=%d rlim=%X", ARG1, ARG2); break;
    case __NR_mmap2: sprintf(buffer, "mmap2: address=%.8X len=%d prot=%X flags=%X fd=%d offset=%d", ARG1, ARG2, ARG3, ARG4, ARG5, ARG6); break;
    case __NR_ftruncate64: sprintf(buffer, "ftruncate64: fildes=%d length=%llu", ARG1, ARG2 | ((U64)ARG3 << 32)); break;    
    case __NR_stat64: sprintf(buffer, "stat64: path=%s buf=%X", getNativeString(thread, ARG1, tmp, sizeof(tmp)), ARG2); break;
    case __NR_lstat64: sprintf(buffer, "lstat64: path=%s buf=%X", getNativeString(thread, ARG1, tmp, sizeof(tmp)), ARG2); break;
    case __NR_fstat64: sprintf(buffer, "fstat64: fildes=%d buf=%X", ARG1, ARG2); break;
    case __NR_lchown32: sprintf(buffer, "lchown32: path=%s owner=%d group=%d", getNativeString(thread, ARG1, tmp, sizeof(tmp)), ARG2, ARG3); break;
    case __NR_getuid32: sprintf(buffer, "getuid32:"); break;
    case __NR_getgid32: sprintf(buffer, "getgid32:"); break;
    case __NR_geteuid32: sprintf(buffer, "geteuid32:"); break;
    case __NR_getegid32: sprintf(buffer, "getegid32:"); break;
    case __NR_getgroups32: sprintf(buffer, "getgroups32: size=%d list=%X", ARG1, ARG2);
    case __NR_setgroups32: sprintf(buffer, "setgroups32: size=%d list=%X", ARG1, ARG2);
    case __NR_fchown32: sprintf(buffer, "fchown32: fd=%d owner=%d group=%d", ARG1, ARG2, ARG3); break;
    case __NR_setresuid32:  sprintf(buffer, "setresuid3: ruid=%d euid=%d suid=%d", ARG1, ARG2, ARG3); break;
    case __NR_getresuid32: sprintf(buffer, "getresuid32: ruid=%X(%d) euid=%X(%d) suid=%X(%d)", ARG1, thread->process->userId, ARG2, thread->process->effectiveUserId, ARG3, thread->process->userId); break;
    case __NR_setresgid32:  sprintf(buffer, "setresgid32: rgid=%d egid=%d sgid=%d", ARG1, ARG2, ARG3); break;
    case __NR_getresgid32: sprintf(buffer, "getresgid32: rgid=%X(%d) egid=%X(%d) sgid=%X(%d)", ARG1, thread->process->groupId, ARG2, thread->process->groupId, ARG3, thread->process->groupId); break;
    case __NR_chown32: sprintf(buffer, "chown32: path=%s owner=%d group=%d", getNativeString(thread, ARG1, tmp, sizeof(tmp)), ARG2, ARG3); break;
    case __NR_setuid32: sprintf(buffer, "setuid32: uid=%d", ARG1); break;
    case __NR_setgid32: sprintf(buffer, "setgid32: gid=%d", ARG1); break;
    case __NR_mincore: sprintf(buffer, "mincore: address=%X length=%d vec=%X", ARG1, ARG2, ARG3); break;
    case __NR_madvise: sprintf(buffer, "madvise: address=%X len=%d advise=%d", ARG1, ARG2, ARG3); break;
    case __NR_getdents64: sprintf(buffer, "getdents64: fd=%d dir=%X count=%d", ARG1, ARG2, ARG3); break;
    case __NR_fcntl64: sprintf(buffer, "fcntl64: fildes=%d cmd=%d arg=%d", ARG1, ARG2, ARG3); break;
    case __NR_gettid: sprintf(buffer, "gettid:"); break;
    case __NR_fsetxattr: sprintf(buffer, "fsetxattr:"); break;
    case __NR_fgetxattr: sprintf(buffer, "fgetxattr:"); break;
    case __NR_flistxattr: sprintf(buffer, "flistxattr:"); break;
    case __NR_futex: sprintf(buffer, "futex: address=%X op=%d", ARG1, ARG2); break;
    case __NR_sched_setaffinity: sprintf(buffer, "sched_setaffinity: pid=%d cpusetsize=d cpu_set_t=%X", ARG1, ARG2, ARG3); break;
    case __NR_sched_getaffinity: sprintf(buffer, "sched_getaffinity: pid=%d cpusetsize=%d mask=%X", ARG1, ARG2, ARG3); break;
    case __NR_set_thread_area: sprintf(buffer, "set_thread_area: u_info=%X", ARG1); break;        
    case __NR_exit_group: sprintf(buffer, "exit_group: code=%d", ARG1); break;
    case __NR_epoll_create: sprintf(buffer, "epoll_create: size=%d", ARG1); break;
    case __NR_epoll_ctl: sprintf(buffer, "epoll_ctl: epfd=%d op=%d fd=%d events=%X", ARG1, ARG2, ARG3, ARG4); break;
    case __NR_epoll_wait: sprintf(buffer, "epoll_wait: epfd=%d events=%X maxevents=%d timeout=%d", ARG1, ARG2, ARG3, ARG4); break;
    case __NR_set_tid_address: sprintf(buffer, "set_tid_address: address=%X", ARG1); break;
    case __NR_clock_gettime: sprintf(buffer, "clock_gettime: clock_id=%d tp=%X", ARG1, ARG2); break;
    case __NR_clock_getres: sprintf(buffer, "clock_getres: clock_id=%d res=%X", ARG1, ARG2); break;
    case __NR_statfs64: sprintf(buffer, "fstatfs64: path=%X(%s) len=%d buf=%X", ARG1, getNativeString(thread, ARG1, tmp, sizeof(tmp)), ARG2, ARG3); break;
    case __NR_fstatfs64: sprintf(buffer, "fstatfs64: fd=%d len=%d buf=%X", ARG1, ARG2, ARG3); break;
    case __NR_tgkill: sprintf(buffer, "tgkill: threadGroupId=%d threadId=%d signal=%d", ARG1, ARG2, ARG3); break;
    case __NR_utimes: sprintf(buffer, "utimes: fileName=%s times=%X", getNativeString(thread, ARG1, tmp, sizeof(tmp)), ARG2);
    case __NR_fadvise64_64: sprintf(buffer, "fadvise64_64: fd=%d", ARG1); break;
    case __NR_inotify_init: sprintf(buffer, "inotify_init: "); break;
    case __NR_openat: sprintf(buffer, "openat: dirfd=%d name=%s flags=%x", ARG1, getNativeString(thread, ARG2, tmp, sizeof(tmp)), ARG3); break;	
    case __NR_mkdirat: sprintf(buffer, "mkdirat: dirfd=%d path=%s mode=%x", ARG1, getNativeString(thread, ARG2, tmp, sizeof(tmp)), ARG3); break;	
    case __NR_fchownat: sprintf(buffer, "fchown32: pathname=%X(%s) owner=%d group=%d flags=%d", ARG2, getNativeString(thread, ARG2, tmp, sizeof(tmp)), ARG3, ARG4, ARG5); break;	
    case __NR_fstatat64: sprintf(buffer, "statat64: dirfd=%d path=%s buf=%X flags=%x", ARG1, getNativeString(thread, ARG2, tmp, sizeof(tmp)), ARG3, ARG4); break;
    case __NR_unlinkat: sprintf(buffer, "unlinkat: dirfd=%d path=%s flags=%x", ARG1, getNativeString(thread, ARG2, tmp, sizeof(tmp)), ARG3); break;
    case __NR_symlinkat: sprintf(buffer, "symlinkat: oldpath=%x(%s) dirfd=%d newpath=%X(%s)", ARG1, getNativeString(thread, ARG1, tmp, sizeof(tmp)), ARG2, ARG3, getNativeString(thread, ARG3, tmp2, sizeof(tmp2))); break;
    case __NR_readlinkat: sprintf(buffer, "symlinkat: dirfd=%d pathname=%X(%s) buf=%X(%s) bufsiz=%d", ARG1, ARG2, getNativeString(thread, ARG2, tmp, sizeof(tmp)), ARG3, getNativeString(thread, ARG3, tmp2, sizeof(tmp2)), ARG4); break;
    case __NR_fchmodat: sprintf(buffer, "fchmodat pathname=%X(%s) mode=%X flags=%X", ARG2, getNativeString(thread, ARG2, tmp, sizeof(tmp)), ARG3, ARG4); break;
    case __NR_faccessat: sprintf(buffer, "faccessat dirfd=%X pathname=%X(%s) mode=%X flags=%X", ARG1, ARG2, getNativeString(thread, ARG2, tmp, sizeof(tmp)), ARG3, ARG4); break;
    case __NR_set_robust_list: sprintf(buffer, "set_robust_list:"); break;
    case __NR_sync_file_range: sprintf(buffer, "sync_file_range:"); break;
    case __NR_utimensat: sprintf(buffer, "utimensat dirfd=%d path=%X(%s) times=%X flags=%X", ARG1, ARG2, getNativeString(thread, ARG2, tmp, sizeof(tmp)), ARG3, ARG4); break;
    case __NR_signalfd4: sprintf(buffer, "signalfd4 fd=%d mask=%X flags=%X", ARG1, ARG2, ARG3); break;
    case __NR_epoll_create1: sprintf(buffer, "epoll_create1: falgs=%X", ARG1); break;
    case __NR_pipe2: sprintf(buffer, "pipe2 fildes=%X", ARG1); break;
    case __NR_prlimit64: sprintf(buffer, "prlimit64 pid=%d resource=%d newlimit=%X oldlimit=%X", ARG1, ARG2, ARG3, ARG4); break;
    case __NR_sendmmsg: sprintf(buffer, "sendmmsg fd=%d address=%X vlen=%d flags=%X", ARG1, ARG2, ARG3, ARG4); break;
    default: sprintf(buffer, "unknown syscall: %d", EAX); break;
    }
}
