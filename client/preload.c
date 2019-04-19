/*
All the libc calls for file system,sockets,pipe etc are channelized to preload. This preload library communicates with the rwfm rule_engine for an below mentioned
function call to check whether the function call should be allowed or not in the rwfm context and gets back the result from the rule_engine. If it's allowed preload calls
the actual libc call for that function using the 'underlying_libc_functions.h'. Otherwise preload will deny the function call.
*/

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <stdarg.h>
#include "rule_engine.h"
#include "signal_utils.h"

void debuglog(char *log) {
    int logfd = underlying_open("/tmp/preload.log", 02000|02);
    underlying_write(logfd, log, strlen(log));
    underlying_close(logfd);
}

pid_t fork(void) {    
    if(is_rwfm_enabled()) {
		mode_t curmask = umask(0);
		sem_open(FORK_LOCK, O_CREAT, 0666, 0);
		umask(curmask);
		pid_t pid = underlying_fork();
        if(pid == 0) {
            char host_name[1024];
            sprintf(host_name, HOSTNAME);
            fork_check(host_name, getuid(), getpid(), getppid());
			sem_t * sem_id = sem_open(FORK_LOCK, 0);
			sem_post(sem_id);
        } else {
			sem_t * sem_id = sem_open(FORK_LOCK, 0);
			sem_wait(sem_id);
			sem_unlink(FORK_LOCK);
		}
		return pid;
    } else
		return underlying_fork();
}

int open(const char *path, int flags) {
    int fd = underlying_open(path, flags);
    if(is_rwfm_enabled()) {
        if(fd == -1) 
            return -1;

        struct stat file_info;
	    if(stat(path, &file_info) == -1)
            return -1;

        char host_name[1024];
        sprintf(host_name, HOSTNAME);

		if(S_ISREG(file_info.st_mode))
	        file_open_check(host_name, &file_info);
		else if(S_ISFIFO(file_info.st_mode))
			open_fifo_check(host_name, getuid(), getpid(), &file_info);
    }
    return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
    char host_name[1024];
    sprintf(host_name, HOSTNAME);

    struct stat file_info;
    fstat(fd, &file_info);

	ssize_t ret = underlying_read(fd, buf, count);

    if(is_rwfm_enabled()) {
        if(S_ISREG(file_info.st_mode) && file_read_check(host_name, getuid(), getpid(), fd) != 1) {
			memset(buf, '\0', count);
            return -1;
        } else if(S_ISFIFO(file_info.st_mode) && pipe_read_check(host_name, getuid(), getpid(), &file_info) != 1) {
			memset(buf, '\0', count);
			return -1;
		}
    }

    return ret;
}

ssize_t write(int fd, const void *buf, size_t count) {
    char host_name[1024];
    sprintf(host_name, HOSTNAME);

    struct stat file_info;
    fstat(fd, &file_info);

    if(is_rwfm_enabled()) {
        if(S_ISREG(file_info.st_mode) && file_write_check(host_name, getuid(), getpid(), fd) != 1) {
            return -1;
        } else if(S_ISFIFO(file_info.st_mode) && pipe_write_check(host_name, getuid(), getpid(), &file_info) != 1) {
			return -1;
		}
    }

    return underlying_write(fd, buf, count);
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    char host_name[1024];
    sprintf(host_name, HOSTNAME);

    int ret = underlying_connect(sockfd, addr, addrlen);
    if(!is_rwfm_enabled()) {
		return ret;
	}
	
	if(connect_check(host_name, getuid(), getpid(), sockfd, (struct sockaddr_in *)addr) == 1)
		return ret;

	return -1;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	int new_sockfd = underlying_accept(sockfd, addr, addrlen);
	if(!is_rwfm_enabled() || new_sockfd == -1) {
		return new_sockfd;
	}

	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	if(accept_check(host_name, getuid(), getpid(), new_sockfd) != 1) {
		return -1;
    }

	return new_sockfd;
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
	if(is_rwfm_enabled()) {
		char host_name[1024];
		sprintf(host_name, HOSTNAME);

		if(send_check(host_name, getuid(), getpid(), sockfd) != 1)
			return -1;
	}

	return underlying_send(sockfd, buf, len, flags);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	ssize_t ret = underlying_recv(sockfd, buf, len, flags);

	if(!is_rwfm_enabled()) {
		return ret;
	}

	if(recv_check(host_name, getuid(), getpid(), sockfd) != 1) {
		memset(buf, '\0', len);
		buf = NULL;
		return -1;
	}

	return ret;
}

int pipe(int pipefd[2]) {
	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	int ret = underlying_pipe(pipefd);

	if(!is_rwfm_enabled() || ret == -1) {
		return ret;
	}

	struct stat pipe_info;
	fstat(pipefd[0], &pipe_info);

	if(create_pipe_check(host_name, getuid(), getpid(), &pipe_info) != 1) {
		memset(pipefd, '\0', 2);
		pipefd = NULL;
		return -1;
	}

	return ret;
}

int msgget(key_t key, int msgflg) {
	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	int ret = underlying_msgget(key, msgflg);

	if(!is_rwfm_enabled() || ret == -1) {
		return ret;
	}

	if(create_msgq_check(host_name, ret) != 1) {
		return -1;
	}

	return ret;
}

int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg) {
	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	if(!is_rwfm_enabled())
		return underlying_msgsnd(msqid, msgp, msgsz, msgflg);

	if(msgsnd_check(host_name, getuid(), getpid(), msqid) != 1)
		return -1;

	return underlying_msgsnd(msqid, msgp, msgsz, msgflg);
}

ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg) {
	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	ssize_t ret = underlying_msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);

	if(!is_rwfm_enabled() || ret <= 0)
		return ret;

	if(msgrcv_check(host_name, getuid(), getpid(), msqid) != 1) {
		memset(msgp, '\0', msgsz);
		msgp = NULL;
		return -1;
	}

	return ret;
}

int msgctl(int msqid, int cmd, struct msqid_ds *buf) {
	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	ssize_t ret = underlying_msgctl(msqid, cmd, buf);

	if(!is_rwfm_enabled() || ret == -1)
		return ret;

	if(msgrcv_check(host_name, getuid(), getpid(), msqid) != 1) {
		memset(buf, '\0', sizeof(struct msqid_ds));
		buf = NULL;
		return -1;
	}

	if(cmd == IPC_RMID)
		remove_msgq_check(host_name, msqid);

	return ret;
}

int semget(key_t key, int nsems, int semflg) {
	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	int ret = underlying_semget(key, nsems, semflg);

	if(!is_rwfm_enabled() || ret == -1) {
		return ret;
	}

	if(create_sem_check(host_name, ret) != 1) {
		return -1;
	}

	return ret;
}

int semop(int semid, struct sembuf *sops, size_t nsops) {
	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	int rd_op=0, wr_op=0;
	for(int i=0;i<nsops;i++) {
		if(sops->sem_op > 0)
			wr_op = 1;
		else if(sops->sem_op == 0)
			rd_op = 1;
		else {
			rd_op = 1;
			wr_op = 1;
			break;
		}
	}

	int ret_val = -1;

	if(rd_op == 1 && sem_read_check(host_name, getuid(), getpid(), semid) == 1)
		ret_val = 1;
	if(wr_op == 1 && sem_write_check(host_name, getuid(), getpid(), semid) == 1)
		ret_val = 1;

	if(ret_val == 1)
		return underlying_semop(semid, sops, nsops);
	else
		return -1;
}

int semctl(int semid, int semnum, int cmd) {
	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	ssize_t ret = underlying_semctl(semid, semnum, cmd);

	if(!is_rwfm_enabled() || ret == -1)
		return ret;

	if(cmd == IPC_RMID)
		remove_sem_check(host_name, semid);

	return ret;
}

int close(int fd) {
	if(!is_rwfm_enabled())
		return underlying_close(fd);

    char host_name[1024];
    sprintf(host_name, HOSTNAME);
    struct stat file_info;
    if(fstat(fd, &file_info) == -1)
        return -1;

    if(S_ISSOCK(file_info.st_mode))
        socket_close_check(host_name, getuid(), getpid(), fd);
    else if(S_ISFIFO(file_info.st_mode))
		pipe_close_check(host_name, getuid(), getpid(), &file_info);

    return underlying_close(fd);
}

int kill(pid_t pid, int sig) {
	if(!is_rwfm_enabled())
		return underlying_kill(pid, sig);

	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	char **cur_process_grp = NULL;
	int n;

    //If signalling a specific process
    if(pid > 0) {
		int peer_uid = get_uid_from_pid(pid);
        if(peer_uid < 0)
            return -1;
        if(kill_check(host_name, getuid(), getpid(), peer_uid, pid) != 1)
            return -1;
        return underlying_kill(pid, sig);
    } else if(pid == 0) { //If signalling current process group
        n = get_group_pids_from_pid(getpid(), &cur_process_grp);
	} else if(pid == -1) { //If signalling all processes for which signalling is permitted
		n = get_all_permitted_pids_from_pid(getpid(), &cur_process_grp);
	} else { //If signalling a specific process group
		n = get_group_pids_from_pid(-pid, &cur_process_grp);
	}

	//If no other process in current process group then let it pass
    if(n<=0)
        return underlying_kill(pid, sig);

	//If only one other process, then same sematics as signalling single process
    if(n==1) {
		if(kill_check(host_name, getuid(), getpid(), get_uid_from_pid(strtol(cur_process_grp[0], NULL, 10)), strtol(cur_process_grp[0], NULL, 10)) != 1) {
			free(cur_process_grp[0]);
            free(cur_process_grp);
	        return -1;
		}
		free(cur_process_grp[0]);
        free(cur_process_grp);

	    return underlying_kill(pid, sig);
	}

	//If signalling multiple processes, then check semantics for each process to be signalled
	char peers[MAX_REQUEST_LENGTH];
    peers[0] = '\0';
    for(int i=0;i<n;i++) {
        sprintf(peers + strlen(peers), "%d %s ", get_uid_from_pid(strtol(cur_process_grp[i], NULL, 10)), cur_process_grp[i]);
        free(cur_process_grp[i]);
    }
    free(cur_process_grp);

    if(kill_many_check(host_name, getuid(), getpid(), peers) != 1)
        return -1;

    return underlying_kill(pid, sig);
}

int sigqueue(pid_t pid, int sig, const union sigval value) {
	if(!is_rwfm_enabled())
		return underlying_sigqueue(pid, sig, value);

	char host_name[1024];
    sprintf(host_name, HOSTNAME);

	int peer_uid = get_uid_from_pid(pid);
    if(peer_uid < 0)
        return -1;
    if(kill_check(host_name, getuid(), getpid(), peer_uid, pid) != 1)
        return -1;

    return underlying_sigqueue(pid, sig, value);
}

int killpg(int pgrp, int sig) {
	if(!is_rwfm_enabled())
		return underlying_killpg(pgrp, sig);

	char host_name[1024];
    sprintf(host_name, HOSTNAME);

    char **cur_process_grp = NULL;
    int n = get_group_pids_from_pid(-pgrp, &cur_process_grp);
	//If no other process permitted then let it pass
    if(n<=0)
        return underlying_killpg(pgrp, sig);

	//If only one other process, then same sematics as signalling single process
    if(n==1) {
		if(kill_check(host_name, getuid(), getpid(), get_uid_from_pid(strtol(cur_process_grp[0], NULL, 10)), strtol(cur_process_grp[0], NULL, 10)) != 1) {
			free(cur_process_grp[0]);
            free(cur_process_grp);
	        return -1;
		}
		free(cur_process_grp[0]);
        free(cur_process_grp);

	    return underlying_killpg(pgrp, sig);
    }

	//If signalling multiple processes, then check rwfm semantics for each process to be signalled
	char peers[MAX_REQUEST_LENGTH];
    peers[0] = '\0';
    for(int i=0;i<n;i++) {
        sprintf(peers + strlen(peers), "%d %s ", get_uid_from_pid(strtol(cur_process_grp[i], NULL, 10)), cur_process_grp[i]);
        free(cur_process_grp[i]);
    }
    free(cur_process_grp);

    if(kill_many_check(host_name, getuid(), getpid(), peers) != 1)
        return -1;

    return underlying_killpg(pgrp, sig);
}
