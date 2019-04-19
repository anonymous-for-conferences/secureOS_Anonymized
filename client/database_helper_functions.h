/*
This file contains functions that maintain the entire database.
The actual database resides in the 'database_server.c' file.
All the extern variables declared here are instantiated in database_server.c.
The functions declared here adds, deletes, modifies or fetches various information from the database.
*/
#ifndef _DB_HELPER_FUNCTION_H_
#define _DB_HELPER_FUNCTION_H_

#include "database_model.h"
#include <string.h>

#define MAX_HOST_SIZE 1024

#define FROM_SUB 0
#define FROM_SHM 1

/*
Functions that are related to HOSTs in database.
num_hosts is total number of hosts.
all_hosts is an array containing all the host ids encountered till now.
*/
extern int num_hosts;
extern HOST * all_hosts;

/*
Description :   Helper function to compare two hosts.
Parameters  :   Two hosts that are being compared.
Return Value:   zero if the hosts do not match, non zero otherwise.
*/
int match_hosts(HOST h1, HOST h2) {
    return !strcmp(h1, h2);
}

/*
Description :   Add a new host to the database if it doesn't exist.
Parameters  :   new host to be added
Return Value:   index of 'new host' in the array.
*/
int add_host(HOST new_host) {
    for(int i=0;i<num_hosts;i++) {
        if(match_hosts(all_hosts[i], new_host))
            return i;
    }

    all_hosts = (HOST *)realloc(all_hosts, (num_hosts+1) * sizeof(HOST));
    all_hosts[num_hosts] = (HOST)malloc(MAX_HOST_SIZE * sizeof(char));
    strcpy(all_hosts[num_hosts], new_host);

    return num_hosts++;
}

int get_host_index(HOST new_host) {
    for(int i=0;i<num_hosts;i++) {
        if(match_hosts(all_hosts[i], new_host))
            return i;
    }

    return -1;
}

extern int num_user_ids;
extern USER_ID * all_user_ids;

int match_user_ids(USER_ID u1, USER_ID u2) {
    return ((u1.host_id_index == u2.host_id_index) && (u1.uid == u2.uid));
}

int add_user_id(USER_ID new_user_id) {
    for(int i=0;i<num_user_ids;i++) {
        if(match_user_ids(all_user_ids[i], new_user_id))
            return i;
    }

    all_user_ids = (USER_ID *)realloc(all_user_ids, (num_user_ids+1) * sizeof(USER_ID));
    all_user_ids[num_user_ids] = new_user_id;

    return num_user_ids++;
}

int get_user_id_index(USER_ID user_id) {
    for(int i=0;i<num_user_ids;i++) {
        if(match_user_ids(all_user_ids[i], user_id))
            return i;
    }

    return -1;
}

int get_number_of_users() {
    return num_user_ids;
}

extern int num_group_ids;
extern GROUP_ID * all_group_ids;

int match_group_ids(GROUP_ID u1, GROUP_ID u2) {
    return (u1.host_id_index == u2.host_id_index) && (u1.gid == u2.gid);
}

int add_group_id(GROUP_ID new_group_id) {
    for(int i=0;i<num_group_ids;i++) {
        if(match_group_ids(all_group_ids[i], new_group_id))
            return i;
    }

    all_group_ids = (GROUP_ID *)realloc(all_group_ids, (num_group_ids+1) * sizeof(GROUP_ID));
    all_group_ids[num_group_ids] = new_group_id;

    return num_group_ids++;
}

int get_group_id_index(GROUP_ID group_id) {
    for(int i=0;i<num_group_ids;i++) {
        if(match_group_ids(all_group_ids[i], group_id))
            return i;
    }

    return -1;
}

USER_SET get_members_from_group_id(int group_id_index) {
    return all_group_ids[group_id_index].members;
}

extern int num_object_ids;
extern OBJECT_ID * all_object_ids;

int match_object_ids(OBJECT_ID obj_id1, OBJECT_ID obj_id2) {
    return obj_id1.host_id_index == obj_id2.host_id_index
            && obj_id1.device_id == obj_id2.device_id
            && obj_id1.inode_number == obj_id2.inode_number;
}

int add_object_id(OBJECT_ID new_object_id) {
    for(int i=0;i<num_object_ids;i++) {
        if(match_object_ids(all_object_ids[i], new_object_id))
            return i;
    }

    all_object_ids = (OBJECT_ID *)realloc(all_object_ids, (num_object_ids+1) * sizeof(OBJECT_ID));
    all_object_ids[num_object_ids] = new_object_id;

    return num_object_ids++;
}

int get_object_id_index(OBJECT_ID object_id) {
    for(int i=0;i<num_object_ids;i++) {
        if(match_object_ids(all_object_ids[i], object_id))
            return i;
    }

    return -1;
}

extern int num_objects;
extern OBJECT * all_objects;

int match_objects(OBJECT obj1, OBJECT obj2) {
    return obj1.obj_id_index == obj2.obj_id_index;
}

int add_object(OBJECT new_object) {
    for(int i=0;i<num_objects;i++) {
        if(match_objects(all_objects[i], new_object))
            return i;
    }

    all_objects = (OBJECT *)realloc(all_objects, (num_objects+1) * sizeof(OBJECT));
    all_objects[num_objects] = new_object;

    return num_objects++;
}

int get_object_from_obj_id_index(int obj_id_index) {
    for(int i=0;i<num_objects;i++) {
        if(all_objects[i].obj_id_index == obj_id_index)
            return i;
    }

    return -1;
}

int update_object_label(int obj_id_index, USER_SET readers, USER_SET writers) {
    int obj_index = get_object_from_obj_id_index(obj_id_index);
    all_objects[obj_index].readers = readers;
    all_objects[obj_index].writers = writers;

    return 0;
}

extern int num_subject_ids;
extern SUBJECT_ID * all_subject_ids;

int match_subject_ids(SUBJECT_ID sub_id1, SUBJECT_ID sub_id2) {
    return sub_id1.host_id_index == sub_id2.host_id_index
            && sub_id1.uid == sub_id2.uid
            && sub_id1.pid == sub_id2.pid;
}

int add_subject_id(SUBJECT_ID new_subject_id) {
    for(int i=0;i<num_subject_ids;i++) {
        if(match_subject_ids(all_subject_ids[i], new_subject_id))
            return i;
    }

    all_subject_ids = (SUBJECT_ID *)realloc(all_subject_ids, (num_subject_ids+1) * sizeof(SUBJECT_ID));
    all_subject_ids[num_subject_ids] = new_subject_id;

    return num_subject_ids++;
}

int get_subject_id_index(SUBJECT_ID subject_id) {
    for(int i=0;i<num_subject_ids;i++) {
        if(match_subject_ids(all_subject_ids[i], subject_id))
            return i;
    }

    return -1;
}

extern int num_subjects;
extern SUBJECT * all_subjects;

int match_subjects(SUBJECT sub1, SUBJECT sub2) {
    return sub1.sub_id_index == sub2.sub_id_index;
}

int add_subject(SUBJECT new_subject) {
    for(int i=0;i<num_subjects;i++) {
        if(match_subjects(all_subjects[i], new_subject))
            return i;
    }

    all_subjects = (SUBJECT *)realloc(all_subjects, (num_subjects+1) * sizeof(SUBJECT));
    all_subjects[num_subjects] = new_subject;

    return num_subjects++;
}

int get_subject_from_sub_id_index(int sub_id_index) {
    for(int i=0;i<num_subjects;i++) {
        if(all_subjects[i].sub_id_index == sub_id_index)
            return i;
    }

    return -1;
}

int update_subject_label(int sub_id_index, USER_SET readers, USER_SET writers) {
    int sub_index = get_subject_from_sub_id_index(sub_id_index);
    all_subjects[sub_index].readers = readers;
    all_subjects[sub_index].writers = writers;

    return 1;
}

extern int num_socket_connections;
extern SOCKET_CONNECTION_OBJECT * all_socket_connections;

int match_address(ADDRESS addr1, ADDRESS addr2) {
	return ((addr1.ip == addr2.ip) && (addr1.port == addr2.port));
}

int get_connection_index(ADDRESS src, ADDRESS dstn) {
	for(int i=0;i<num_socket_connections;i++)
		if((match_address(all_socket_connections[i].src, src) && match_address(all_socket_connections[i].dstn, dstn))
            || (match_address(all_socket_connections[i].src, dstn) && match_address(all_socket_connections[i].dstn, src)))
			return i;

	return -1;
}

int add_connection(SOCKET_CONNECTION_OBJECT new_socket_connection) {
	int exists = get_connection_index(new_socket_connection.src, new_socket_connection.dstn);
	if(exists == -1) {
		all_socket_connections = (SOCKET_CONNECTION_OBJECT *)realloc(all_socket_connections, (num_socket_connections+1) * sizeof(SOCKET_CONNECTION_OBJECT));
		all_socket_connections[num_socket_connections] = new_socket_connection;

		return num_socket_connections++;
	} else {
		all_socket_connections[exists].peer_ids[1] = all_socket_connections[exists].peer_ids[0];
		all_socket_connections[exists].num_peers++;
		all_socket_connections[exists].readers = all_socket_connections[exists].readers & new_socket_connection.readers;
		all_socket_connections[exists].writers = all_socket_connections[exists].writers | new_socket_connection.writers;

		return exists;
	}
}

int update_connection_label(int connection_index, USER_SET readers, USER_SET writers) {
	all_socket_connections[connection_index].readers = readers;
	all_socket_connections[connection_index].writers = writers;

	return 0;
}

int is_peer(int connection_index, int peer_id) {
	return (all_socket_connections[connection_index].peer_ids[0] == peer_id || all_socket_connections[connection_index].peer_ids[1] == peer_id);
}

int remove_peer_from_connection(ADDRESS src, ADDRESS dstn, int peer_id) {
	int connection_index = get_connection_index(src, dstn);
	if(connection_index == -1 || !is_peer(connection_index, peer_id))
		return -1;

	if(all_socket_connections[connection_index].num_peers > 1) {
		all_socket_connections[connection_index].num_peers--;
		return num_socket_connections;
	}

    for(int i=connection_index;i<num_socket_connections-1;i++)
    	all_socket_connections[i] = all_socket_connections[i+1];
    all_socket_connections = (SOCKET_CONNECTION_OBJECT *)realloc(all_socket_connections, (--num_socket_connections) * sizeof(SOCKET_CONNECTION_OBJECT));

	return num_socket_connections;
}


extern int num_pipe_objects;
extern PIPE_OBJECT * all_pipe_objects;

int match_pipe(PIPE_OBJECT pipe1, PIPE_OBJECT pipe2) {
	return ((pipe1.host_id_index == pipe2.host_id_index) && (pipe1.device_id == pipe2.device_id) && (pipe1.inode_number == pipe2.inode_number));
}

int get_pipe_index(int host_id_index, ulong device_id, ulong inode_number) {
	PIPE_OBJECT cur_pipe;
	cur_pipe.host_id_index = host_id_index;
	cur_pipe.device_id = device_id;
	cur_pipe.inode_number = inode_number;
	for(int i=0;i<num_pipe_objects;i++) {
		if(match_pipe(all_pipe_objects[i], cur_pipe))
			return i;
	}

	return -1;
}

int add_new_pipe(PIPE_OBJECT new_pipe) {
	all_pipe_objects = (PIPE_OBJECT *)realloc(all_pipe_objects, (num_pipe_objects+1) * sizeof(PIPE_OBJECT));
    all_pipe_objects[num_pipe_objects] = new_pipe;

    return num_pipe_objects++;
}

int update_pipe_label(int pipe_index, USER_SET readers, USER_SET writers) {
	all_pipe_objects[pipe_index].readers = readers;
	all_pipe_objects[pipe_index].writers = writers;

	return 0;
}

int increase_pipe_ref_count(int pipe_index) {
	all_pipe_objects[pipe_index].pipe_ref_count++;

	return 0;
}

int remove_pipe(int host_id_index, ulong device_id, ulong inode_number) {
	int pipe_index = get_pipe_index(host_id_index, device_id, inode_number);
	if(pipe_index == -1)
		return -1;
	if(all_pipe_objects[pipe_index].pipe_ref_count > 1) {
		all_pipe_objects[pipe_index].pipe_ref_count--;
		return num_pipe_objects;
	}
	for(int i=pipe_index;i<num_pipe_objects-1;i++)
    	all_pipe_objects[i] = all_pipe_objects[i+1];
    all_pipe_objects = (PIPE_OBJECT *)realloc(all_pipe_objects, (--num_pipe_objects) * sizeof(PIPE_OBJECT));

	return num_pipe_objects;
}


extern int num_pipe_ref_maps;
extern PIPE_REF_MAP * pipe_ref_map;

int get_pipe_ref_map_index(int sub_id_index, int pipe_index) {
	for(int i=0;i<num_pipe_ref_maps;i++) {
		if(pipe_ref_map[i].sub_id_index == sub_id_index && pipe_ref_map[i].pipe_index == pipe_index)
			return i;
	}

	return -1;
}

int add_new_pipe_mapping(PIPE_REF_MAP new_map) {
	pipe_ref_map = (PIPE_REF_MAP *)realloc(pipe_ref_map, (num_pipe_ref_maps+1) * sizeof(PIPE_REF_MAP));
    pipe_ref_map[num_pipe_ref_maps] = new_map;

    return num_pipe_ref_maps++;
}

int increment_pipe_mapping_ref_count(int sub_id_index, int pipe_index) {
	int cur_map = get_pipe_ref_map_index(sub_id_index, pipe_index);
	if(cur_map == -1)
		return -1;
	return ++pipe_ref_map[cur_map].ref_count;
}

int decrement_pipe_mapping_ref_count(int sub_id_index, int pipe_index) {
	int cur_map = get_pipe_ref_map_index(sub_id_index, pipe_index);
	if(cur_map == -1)
		return -1;

	if(pipe_ref_map[cur_map].ref_count > 1)
		return --pipe_ref_map[cur_map].ref_count;

	for(int i=cur_map;i<num_pipe_ref_maps-1;i++)
        pipe_ref_map[i] = pipe_ref_map[i+1];
    pipe_ref_map = (PIPE_REF_MAP *)realloc(pipe_ref_map, (--num_pipe_ref_maps) * sizeof(PIPE_REF_MAP));

    return 0;
}


extern int num_msgq_objects;
extern MSGQ_OBJECT * all_msgq_objects;

int add_msgq_object(MSGQ_OBJECT msgq_object) {
	all_msgq_objects = (MSGQ_OBJECT *)realloc(all_msgq_objects, (num_msgq_objects+1) * sizeof(MSGQ_OBJECT));
    all_msgq_objects[num_msgq_objects] = msgq_object;

    return num_msgq_objects++;
}

int get_msgq_object_index(int host_index, int msgq_id) {
	for(int i=0;i<num_msgq_objects;i++) {
		if(all_msgq_objects[i].host_index == host_index && all_msgq_objects[i].msgq_id == msgq_id)
			return i;
	}

	return -1;
}

int remove_msgq_object(int host_index, int msgq_id) {
	int msgq_index = get_msgq_object_index(host_index, msgq_id);
	if(msgq_index == -1)
		return -1;
	for(int i=msgq_index;i<num_msgq_objects-1;i++)
        all_msgq_objects[i] = all_msgq_objects[i+1];
    all_msgq_objects = (MSGQ_OBJECT *)realloc(all_msgq_objects, (--num_msgq_objects) * sizeof(MSGQ_OBJECT));

	return num_msgq_objects;
}


extern int num_sem_objects;
extern SEM_OBJECT * all_sem_objects;

int add_sem_object(SEM_OBJECT sem_object) {
	all_sem_objects = (SEM_OBJECT *)realloc(all_sem_objects, (num_sem_objects+1) * sizeof(SEM_OBJECT));
    all_sem_objects[num_sem_objects] = sem_object;

    return num_sem_objects++;
}

int get_sem_object_index(int host_index, int sem_id) {
	for(int i=0;i<num_sem_objects;i++) {
		if(all_sem_objects[i].host_index == host_index && all_sem_objects[i].sem_id == sem_id)
			return i;
	}

	return -1;
}

int remove_sem_object(int host_index, int sem_id) {
	int sem_index = get_sem_object_index(host_index, sem_id);
	if(sem_index == -1)
		return -1;
	for(int i=sem_index;i<num_sem_objects-1;i++)
        all_sem_objects[i] = all_sem_objects[i+1];
    all_sem_objects = (SEM_OBJECT *)realloc(all_sem_objects, (--num_sem_objects) * sizeof(SEM_OBJECT));

	return num_sem_objects;
}


/*
Description :   Copy all the information contained in source subject id to destination subject id.
                The information include all the fds (file, pipe, socket, etc) opened by the source.
Parameters  :   subject id index of source and destination subjects
Return Value:   Always succeeds and returns 0.
*/

void copy_pipe_map(int new_sub_id, int old_pipe_map_index) {
    int new_map_index = add_new_pipe_mapping(pipe_ref_map[old_pipe_map_index]);
    pipe_ref_map[new_map_index].sub_id_index = new_sub_id;
	//Now the pipe object is refered to by twice as many subjects
	all_pipe_objects[pipe_ref_map[new_map_index].pipe_index].pipe_ref_count+=pipe_ref_map[new_map_index].ref_count;
}

int copy_subject_info(uint src_sub_id_index, uint dstn_sub_id_index) {
	for(int i=0;i<num_pipe_ref_maps;i++) {
        if(pipe_ref_map[i].sub_id_index == src_sub_id_index)
            copy_pipe_map(dstn_sub_id_index, i);
    }

    return 0;
}

#endif
