/*
This is the RWFM rule engine. The preload library calls the functions mentioned here to check for the rwfm rules. The rwfm rule engine in turn communnicates with 
the database_server using the database_queries.c to infer the labels of all the subjects(users,groups) and the objects. Then it checks whether the rwfm rule is satisfied 
for that particular operation using the functioned mentioned in 'user_set_manipulation_functions'. If the operation is allowed it sends true else false to preload.. 
*/

#ifndef _RULE_ENGINE_H_
#define _RULE_ENGINE_H_

#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include "infer_object_labels.h"

//For ensuring mutual exclusion of database server access
void lock() {
    mode_t prev_mask = umask(0);
    sem_t * sem_id = sem_open(RULE_ENGINE_SEMAPHORE, O_CREAT, 0666, 1);
    umask(prev_mask);
    sem_wait(sem_id);
}

void unlock() {
    mode_t prev_mask = umask(0);
    sem_t * sem_id = sem_open(RULE_ENGINE_SEMAPHORE, O_CREAT, 0666, 1);
    umask(prev_mask);
    sem_post(sem_id);
}

void fork_check(char* host_name, int uid, int child_pid, int parent_pid) {
	lock();
    int host_id_index = get_host_index(host_name);
    int parent_sub_id_index = get_subject_id_index(host_id_index, uid, parent_pid);
    int child_sub_id_index = add_subject_id(host_id_index, uid, child_pid);
    if(parent_sub_id_index == -1) {
        int owner = get_user_id_index(host_id_index, uid);
        USER_SET readers = get_all_users(get_number_of_users());
        USER_SET writers = 0;
        add_user_to_label(owner, &writers);
        add_subject(child_sub_id_index, owner, readers, writers);
    }
    else {
        SUBJECT parent_subject = get_subject(parent_sub_id_index);
        
        int child_subject = add_subject(child_sub_id_index, parent_subject.owner, parent_subject.readers, parent_subject.writers);
        copy_subject_info(parent_sub_id_index, child_sub_id_index);
    }
	unlock();
}

int file_open_check(char * host_name, struct stat * file_info){
	lock();
    int host_id_index = get_host_index(host_name);
    int object_id_index = get_object_id_index(host_id_index, file_info->st_dev, file_info->st_ino);
    
	if(object_id_index == -1) {
        object_id_index = add_object_id(host_id_index, file_info->st_dev, file_info->st_ino);
   
        OBJECT object;
        object.obj_id_index = object_id_index;
        object.owner = get_user_id_index(host_id_index, file_info->st_uid);
        infer_file_labels(&object, file_info, host_id_index);
        
        add_object(object_id_index, object.owner, object.readers, object.writers);
    }
	unlock();

    return 1;
}

int file_read_check(char * host_name, int uid, int pid, int fd) {
	lock();
	struct stat file_info;
	fstat(fd, &file_info);
    int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
    int obj_id_index = get_object_id_index(host_id_index, file_info.st_dev, file_info.st_ino);
	int ret_val = 0;

    //If fd is not a file fd
    if(obj_id_index == -1)
        ret_val = -1;
	else {
		SUBJECT subject = get_subject(sub_id_index);
		OBJECT object = get_object(obj_id_index);
	
		if(is_user_in_set(subject.owner, &object.readers) == 1) {
		    subject.readers = set_intersection(&subject.readers, &object.readers);
		    subject.writers = set_union(&subject.writers, &object.writers);
		    ret_val = update_subject_label(sub_id_index, subject.readers, subject.writers);
		}
	}
	unlock();

    return ret_val;
}

int file_write_check(char * host_name, int uid, int pid, int fd) {
	lock();
	struct stat file_info;
	fstat(fd, &file_info);
    int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
    int obj_id_index = get_object_id_index(host_id_index, file_info.st_dev, file_info.st_ino);
	int ret_val = 0;

    //If fd is not a file fd
    if(obj_id_index == -1)
        ret_val = -1;
	else {
		SUBJECT subject = get_subject(sub_id_index);
		OBJECT object = get_object(obj_id_index);

		if(is_user_in_set(subject.owner, &object.writers)
		    && is_superset_of(&subject.readers, &object.readers)
		    && is_subset_of(&subject.writers, &object.writers))
		    ret_val = 1;
	}
	unlock();

    return ret_val;
}

int connect_check(char * host_name, int uid, int pid, int sock_fd, struct sockaddr_in *peer_addr) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	int ret_val = 1;
	if(sub_id_index == -1)
		ret_val = -1;
	else {
		SUBJECT subject = get_subject(sub_id_index);

		struct sockaddr_in socket_addr;
		socklen_t socket_addr_sz;
		socket_addr_sz = sizeof socket_addr;
		underlying_getsockname(sock_fd, (struct sockaddr *) &socket_addr, &socket_addr_sz);

		add_connection(socket_addr.sin_addr.s_addr, socket_addr.sin_port, peer_addr->sin_addr.s_addr, peer_addr->sin_port, 1, sub_id_index, subject.readers, subject.writers);
	}
	unlock();

	return ret_val;
}

int accept_check(char * host_name, int uid, int pid, int sock_fd) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	int ret_val = 1;
	if(sub_id_index == -1)
		ret_val = -1;
	else {
		SUBJECT subject = get_subject(sub_id_index);

		struct sockaddr_in socket_addr, peer_addr;
		socklen_t socket_addr_sz, peer_addr_sz;
		socket_addr_sz = sizeof socket_addr;
		peer_addr_sz = sizeof peer_addr;

		underlying_getsockname(sock_fd, (struct sockaddr *) &socket_addr, &socket_addr_sz);
		underlying_getpeername(sock_fd, (struct sockaddr *) &peer_addr, &peer_addr_sz);

		add_connection(socket_addr.sin_addr.s_addr, socket_addr.sin_port, peer_addr.sin_addr.s_addr, peer_addr.sin_port, 1, sub_id_index, subject.readers, subject.writers);
	}
	unlock();

	return ret_val;
}

int send_check(char * host_name, int uid, int pid, int sock_fd) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	int ret_val = 1;
	if(sub_id_index == -1)
		ret_val = -1;
	else {
		struct sockaddr_in socket_addr, peer_addr;
		socklen_t socket_addr_sz, peer_addr_sz;
		socket_addr_sz = sizeof socket_addr;
		peer_addr_sz = sizeof peer_addr;

		underlying_getsockname(sock_fd, (struct sockaddr *) &socket_addr, &socket_addr_sz);
		underlying_getpeername(sock_fd, (struct sockaddr *) &peer_addr, &peer_addr_sz);

		int connection_index = get_connection_index(socket_addr.sin_addr.s_addr, socket_addr.sin_port, peer_addr.sin_addr.s_addr, peer_addr.sin_port);
		if(connection_index == -1)
			ret_val = -1;
		else {
			SOCKET_CONNECTION_OBJECT conn_obj = get_connection(connection_index);
			SUBJECT subject = get_subject(sub_id_index);

			if(conn_obj.readers != subject.readers || conn_obj.writers != subject.writers)
				update_connection_label(connection_index, set_intersection(&subject.readers, &conn_obj.readers), set_union(&subject.writers, &conn_obj.writers));
		}
	}
	unlock();
	return ret_val;
}

int recv_check(char * host_name, int uid, int pid, int sock_fd) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	int ret_val = 0;
	if(sub_id_index == -1)
		ret_val = -1;
	else {
		struct sockaddr_in socket_addr, peer_addr;
		socklen_t socket_addr_sz, peer_addr_sz;
		socket_addr_sz = sizeof socket_addr;
		peer_addr_sz = sizeof peer_addr;

		underlying_getsockname(sock_fd, (struct sockaddr *) &socket_addr, &socket_addr_sz);
		underlying_getpeername(sock_fd, (struct sockaddr *) &peer_addr, &peer_addr_sz);

		int connection_index = get_connection_index(socket_addr.sin_addr.s_addr, socket_addr.sin_port, peer_addr.sin_addr.s_addr, peer_addr.sin_port);
		if(connection_index == -1)
			ret_val = -1;
		else {
			SOCKET_CONNECTION_OBJECT conn_obj = get_connection(connection_index);
			SUBJECT subject = get_subject(sub_id_index);

			if(is_user_in_set(subject.owner, &conn_obj.readers)) {
				if(conn_obj.readers != subject.readers || conn_obj.writers != subject.writers)
					ret_val = update_subject_label(sub_id_index, set_intersection(&subject.readers, &conn_obj.readers), set_union(&subject.writers, &conn_obj.writers));
			}
		}
	}
	unlock();
	return ret_val;
}

int socket_close_check(char * host_name, int uid, int pid, int sock_fd) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);

    struct sockaddr_in socket_addr, peer_addr;
	socklen_t socket_addr_sz, peer_addr_sz;
	socket_addr_sz = sizeof socket_addr;
	peer_addr_sz = sizeof peer_addr;

	underlying_getsockname(sock_fd, (struct sockaddr *) &socket_addr, &socket_addr_sz);
	underlying_getpeername(sock_fd, (struct sockaddr *) &peer_addr, &peer_addr_sz);

    int ret_val = remove_peer_from_connection(socket_addr.sin_addr.s_addr, socket_addr.sin_port, peer_addr.sin_addr.s_addr, peer_addr.sin_port, sub_id_index);
	unlock();
	return ret_val;
}

int open_fifo_check(char * host_name, int uid, int pid, struct stat *pipe_info) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	int pipe_index = get_pipe_index(host_id_index, pipe_info->st_dev, pipe_info->st_ino);
	if(pipe_index == -1) {
		pipe_index = add_new_pipe(host_id_index, pipe_info->st_dev, pipe_info->st_ino, 1, get_all_users(get_number_of_users()), 0);
		add_new_pipe_mapping(sub_id_index, pipe_index, 1);
	} else {
		increase_pipe_ref_count(pipe_index);
		if(increment_pipe_mapping_ref_count(sub_id_index, pipe_index) == -1)
			add_new_pipe_mapping(sub_id_index, pipe_index, 1);
	}
	unlock();

	return 1;
}

int create_pipe_check(char * host_name, int uid, int pid, struct stat *pipe_info) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	SUBJECT subject = get_subject(sub_id_index);

	int pipe_index = add_new_pipe(host_id_index, pipe_info->st_dev, pipe_info->st_ino, 2, subject.readers, subject.writers);
	add_new_pipe_mapping(sub_id_index, pipe_index, 2);
	unlock();

	return 1;
}

int pipe_read_check(char * host_name, int uid, int pid, struct stat *pipe_info) {
	lock();
	int ret_val = 0;

	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	SUBJECT subject = get_subject(sub_id_index);

	int pipe_index = get_pipe_index(host_id_index, pipe_info->st_dev, pipe_info->st_ino);
	PIPE_OBJECT pipe_obj = get_pipe(pipe_index);

	if(is_user_in_set(subject.owner, &pipe_obj.readers)) {
		if(pipe_obj.readers != subject.readers || pipe_obj.writers != subject.writers)
			ret_val = update_subject_label(sub_id_index, set_intersection(&subject.readers, &pipe_obj.readers), set_union(&subject.writers, &pipe_obj.writers));
	}
	unlock();

	return ret_val;
}

int pipe_write_check(char * host_name, int uid, int pid, struct stat *pipe_info) {
	lock();
	int ret_val = 1;

	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	SUBJECT subject = get_subject(sub_id_index);

	int pipe_index = get_pipe_index(host_id_index, pipe_info->st_dev, pipe_info->st_ino);
	PIPE_OBJECT pipe_obj = get_pipe(pipe_index);

	if(pipe_obj.readers != subject.readers || pipe_obj.writers != subject.writers)
		update_pipe_label(pipe_index, set_intersection(&subject.readers, &pipe_obj.readers), set_union(&subject.writers, &pipe_obj.writers));

	unlock();

	return ret_val;
}

int pipe_close_check(char * host_name, int uid, int pid, struct stat *pipe_info) {
	lock();

	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	int pipe_index = get_pipe_index(host_id_index, pipe_info->st_dev, pipe_info->st_ino);

	remove_pipe(host_id_index, pipe_info->st_dev, pipe_info->st_ino);
	decrement_pipe_mapping_ref_count(sub_id_index, pipe_index);

	unlock();

	return 1;
}

int create_msgq_check(char * host_name, int msgq_id) {
	lock();
    int host_id_index = get_host_index(host_name);
    int msgq_object_index = get_msgq_object_index(host_id_index, msgq_id);
    
	if(msgq_object_index == -1) {
		struct msqid_ds msgq_info;
		underlying_msgctl(msgq_id, IPC_STAT, &msgq_info);

        MSGQ_OBJECT msgq_object;
        msgq_object.host_index = host_id_index;
        msgq_object.msgq_id = msgq_id;
        msgq_object.owner = get_user_id_index(host_id_index, msgq_info.msg_perm.uid);
        infer_msgq_labels(&msgq_object, &msgq_info, host_id_index);
        
        add_msgq_object(host_id_index, msgq_id, msgq_object.owner, msgq_object.readers, msgq_object.writers);
    }
	unlock();

    return 1;
}

int msgrcv_check(char * host_name, int uid, int pid, int msgq_id) {
	lock();
    int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	SUBJECT subject = get_subject(sub_id_index);

    MSGQ_OBJECT msgq_object = get_msgq_object(host_id_index, msgq_id);
	int ret_val = 0;

	if(is_user_in_set(subject.owner, &msgq_object.readers) == 1)
	    ret_val = update_subject_label(sub_id_index, set_intersection(&subject.readers, &msgq_object.readers), set_union(&subject.writers, &msgq_object.writers));
	unlock();

    return ret_val;
}

int msgsnd_check(char * host_name, int uid, int pid, int msgq_id) {
	lock();
    int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	SUBJECT subject = get_subject(sub_id_index);

    MSGQ_OBJECT msgq_object = get_msgq_object(host_id_index, msgq_id);
	int ret_val = 0;

	if(is_user_in_set(subject.owner, &msgq_object.writers)
		    && is_superset_of(&subject.readers, &msgq_object.readers)
		    && is_subset_of(&subject.writers, &msgq_object.writers))
	    ret_val = 1;
	unlock();

    return ret_val;
}

int remove_msgq_check(char * host_name, int msgq_id) {
	lock();
    int host_id_index = get_host_index(host_name);
    int msgq_object_index = get_msgq_object_index(host_id_index, msgq_id);
    
	if(msgq_object_index != -1)
        remove_msgq_object(host_id_index, msgq_id);
	unlock();

    return 1;
}

int create_sem_check(char * host_name, int sem_id) {
	lock();
    int host_id_index = get_host_index(host_name);
    int sem_object_index = get_sem_object_index(host_id_index, sem_id);
    
	if(sem_object_index == -1) {
		union semun sem_arg;
		underlying_semctl_multiarg(sem_id, 0, IPC_STAT, sem_arg);

        SEM_OBJECT sem_object;
        sem_object.host_index = host_id_index;
        sem_object.sem_id = sem_id;
        sem_object.owner = get_user_id_index(host_id_index, sem_arg.buf->sem_perm.uid);
        infer_sem_labels(&sem_object, sem_arg.buf, host_id_index);

        add_sem_object(host_id_index, sem_id, sem_object.owner, sem_object.readers, sem_object.writers);
    }
	unlock();

    return 1;
}

int sem_read_check(char * host_name, int uid, int pid, int sem_id) {
	lock();
    int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	SUBJECT subject = get_subject(sub_id_index);

    SEM_OBJECT sem_object = get_sem_object(host_id_index, sem_id);
	int ret_val = 0;

	if(is_user_in_set(subject.owner, &sem_object.readers) == 1)
	    ret_val = update_subject_label(sub_id_index, set_intersection(&subject.readers, &sem_object.readers), set_union(&subject.writers, &sem_object.writers));
	unlock();

    return ret_val;
}

int sem_write_check(char * host_name, int uid, int pid, int sem_id) {
	lock();
    int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	SUBJECT subject = get_subject(sub_id_index);

    SEM_OBJECT sem_object = get_sem_object(host_id_index, sem_id);
	int ret_val = 0;

	if(is_user_in_set(subject.owner, &sem_object.writers)
		    && is_superset_of(&subject.readers, &sem_object.readers)
		    && is_subset_of(&subject.writers, &sem_object.writers))
	    ret_val = 1;
	unlock();

    return ret_val;
}

int remove_sem_check(char * host_name, int sem_id) {
	lock();
    int host_id_index = get_host_index(host_name);
    int sem_object_index = get_sem_object_index(host_id_index, sem_id);
    
	if(sem_object_index != -1)
        remove_sem_object(host_id_index, sem_id);
	unlock();

    return 1;
}

int kill_check(char * host_name, int uid, int pid, int peer_uid, int peer_pid) {
	lock();
	int ret_val = 0;
	int host_id_index = get_host_index(host_name);

    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	SUBJECT subject = get_subject(sub_id_index);

	int peer_sub_id_index = get_subject_id_index(host_id_index, peer_uid, peer_pid);
	SUBJECT peer_subject = get_subject(peer_sub_id_index);

	if(is_user_in_set(peer_subject.owner, &subject.readers) == 1)
	    ret_val = update_subject_label(peer_sub_id_index, set_intersection(&subject.readers, &peer_subject.readers), set_union(&subject.writers, &peer_subject.writers));
	
	unlock();

	return ret_val;
}

int kill_many_check(char * host_name, int uid, int pid, char * peers) {
	lock();
	int ret_val = 1;
	int host_id_index = get_host_index(host_name);

    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	SUBJECT subject = get_subject(sub_id_index);

	char **arguments = (char**)malloc(MAX_REQUEST_LENGTH * sizeof(char*));
    int num_args = get_args_from_request(arguments, peers);

	int * peer_sub_id_index = (int *)malloc((num_args/2) * sizeof(int));
	SUBJECT * peer_subject = (SUBJECT *)malloc((num_args/2) * sizeof(SUBJECT));

	for(int i=0;i<num_args;i+=2) {
		peer_sub_id_index[i/2] = get_subject_id_index(host_id_index, strtol(arguments[i], NULL, 10), strtol(arguments[i+1], NULL, 10));
		peer_subject[i/2] = get_subject(peer_sub_id_index[i/2]);

		if(is_user_in_set(peer_subject[i/2].owner, &subject.readers) != 1) {
			ret_val = 0;
			break;
		}
	}

	if(ret_val == 1) {
		for(int i=0;i<num_args;i+=2) {
			USER_SET new_readers = set_intersection(&subject.readers, &peer_subject[i/2].readers);
			USER_SET new_writers = set_union(&subject.writers, &peer_subject[i/2].writers);
			update_subject_label(peer_sub_id_index[i/2], new_readers, new_writers);
		}
	}
	
	unlock();

	return ret_val;
}

#endif
