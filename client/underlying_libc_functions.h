#ifndef _UND_LIBC_FUN_H_
#define _UND_LIBC_FUN_H_

#include <dlfcn.h>
#include <sys/socket.h>
#include <signal.h>
#include "database_model.h"
#include "preload.h"

void *get_libc() {
    static void *libc_handle = 0;
    if (!libc_handle) {
        libc_handle = dlopen(LIBC, RTLD_LAZY);
    }
    return libc_handle;
}

pid_t underlying_fork(void) {
    static pid_t (*underlying)(void) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "fork");
    }
    return (*underlying)();
}

int underlying_open(const char *pathname, int flags) {
    static int (*underlying)(const char *pathname, int flags) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "open");
    }
    return (*underlying)(pathname, flags);
}

FILE * underlying_fopen(const char *path, const char *mode) {
	static FILE * (*underlying)(const char *pathname, const char *mode) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "fopen");
    }
    return (*underlying)(path, mode);
}

int underlying_read(int fd, void *buf, size_t count) {
    static int (*underlying)(int fd, void *buf, size_t count) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "read");
    }
    return (*underlying)(fd, buf, count);
}

int underlying_write(int fd, const void *buf, size_t count) {
    static int (*underlying)(int fd, const void *buf, size_t count) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "write");
    }
    return (*underlying)(fd, buf, count);
}

int underlying_close(int fd) {
    static int (*underlying)(int fd) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "close");
    }
    return (*underlying)(fd);
}

int underlying_socket(int domain, int type, int protocol) {
    static int (*underlying)(int , int , int ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "socket");
    }
    return (*underlying)(domain, type, protocol);
}

int underlying_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	static int (*underlying)(int, const struct sockaddr *, socklen_t) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "bind");
    }
    return (*underlying)(sockfd, addr, addrlen);
}

int underlying_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    static int (*underlying)(int ,const struct sockaddr* , socklen_t ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "connect");
    }
    return (*underlying)(sockfd, addr, addrlen);
}

int underlying_accept(int sockfd, struct sockaddr* addr, socklen_t *addrlen) {
    static int (*underlying)(int, struct sockaddr*, socklen_t* ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "accept");
    }
    return (*underlying)(sockfd, addr, addrlen);
}

ssize_t underlying_send(int sockfd, const void *buf, size_t len, int flags) {
    static int (*underlying)(int ,const void * , size_t, int ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "send");
    }
    return (*underlying)(sockfd, buf, len, flags);
}

ssize_t underlying_recv(int sockfd, void *buf, size_t len, int flags) {
    static int (*underlying)(int ,void * , size_t, int ) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "recv");
    }
    return (*underlying)(sockfd, buf, len, flags);
}

int underlying_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	static int (*underlying)(int, struct sockaddr *, socklen_t *) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "getsockname");
    }
    return (*underlying)(sockfd, addr, addrlen);
}

int underlying_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	static int (*underlying)(int, struct sockaddr *, socklen_t *) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "getpeername");
    }
    return (*underlying)(sockfd, addr, addrlen);
}

int underlying_pipe(int pipefd[2]) {
	static int (*underlying)(int *) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "pipe");
    }
    return (*underlying)(pipefd);
}

int underlying_msgget(key_t key, int msgflg) {
	static int (*underlying)(key_t, int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "msgget");
    }
    return (*underlying)(key, msgflg);
}

int underlying_msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg) {
	static int (*underlying)(int, const void *, size_t, int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "msgsnd");
    }
    return (*underlying)(msqid, msgp, msgsz, msgflg);
}

int underlying_msgrcv(int msqid, const void *msgp, size_t msgsz, long msgtyp, int msgflg) {
	static int (*underlying)(int, const void *, size_t, long, int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "msgrcv");
    }
    return (*underlying)(msqid, msgp, msgsz, msgtyp, msgflg);
}

int underlying_msgctl(int msqid, int cmd, struct msqid_ds *buf) {
	static int (*underlying)(int, int, struct msqid_ds *) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "msgctl");
    }
    return (*underlying)(msqid, cmd, buf);
}

int underlying_semget(key_t key, int nsems, int semflg) {
	static int (*underlying)(key_t, int, int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "semget");
    }
    return (*underlying)(key, nsems, semflg);
}

int underlying_semop(int semid, struct sembuf *sops, size_t nsops) {
	static int (*underlying)(int, struct sembuf *, size_t) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "semop");
    }
    return (*underlying)(semid, sops, nsops);
}

int underlying_semctl(int semid, int semnum, int cmd) {
	static int (*underlying)(int, int, int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "semctl");
    }
    return (*underlying)(semid, semnum, cmd);
}

int underlying_semctl_multiarg(int semid, int semnum, int cmd, union semun arg) {
	static int (*underlying)(int, int, int, union semun) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "semctl");
    }
    return (*underlying)(semid, semnum, cmd, arg);
}

int underlying_kill(pid_t pid, int sig) {
	static int (*underlying)(pid_t, int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "kill");
    }
    return (*underlying)(pid, sig);
}

int underlying_sigqueue(pid_t pid, int sig, const union sigval value) {
	static int (*underlying)(pid_t, int, const union sigval) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "sigqueue");
    }
    return (*underlying)(pid, sig, value);
}

int underlying_killpg(int pgrp, int sig) {
	static int (*underlying)(int, int) = 0;
    if (!underlying) {
        underlying = dlsym(get_libc(), "killpg");
    }
    return (*underlying)(pgrp, sig);
}

#endif
