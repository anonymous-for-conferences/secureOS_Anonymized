/*
This file contains all the functions to communicate with the database server. database_queries make a named fifo with the database_server. When database_queries receives 
database operation request either from 'add_users_and_groups' or 'rule_engine', it checks the request and sends the request using the proper function for that 
request to the database_server via the fifo. And after getting back the response from the database_server it sends back the response to the rule_engine.

*/

#ifndef _DB_QUERIES_H_
#define _DB_QUERIES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "underlying_libc_functions.h"
#include "database_macros.h"
#include "database_model.h"

#define	O_RDONLY 0x0000		/* open for reading only */
#define	O_WRONLY 0x0001		/* open for writing only */
#define	O_RDWR 0x0002		/* open for reading and writing */
#define O_CREAT	0x0040      /* create file if it doesnt already exist */

//Converting space seperated request string into arguments array
int get_args_from_request(char **args, char *req) {
    memset(args, '\0', sizeof(char*) * MAX_REQUEST_LENGTH);
    char *curToken = strtok(req, " ");
    int i;
    for (i = 0; curToken != NULL; i++) {
      args[i] = strdup(curToken);
      curToken = strtok(NULL, " ");
    }

    return i;
}


int read_response(MQ_BUFFER *res_buff) {
    key_t key = ftok(MQ_FILE_PATH, RESPONSE_MQ_PROJ_ID);
	int msgqid = underlying_msgget(key, 0666 | IPC_CREAT);

    return underlying_msgrcv(msgqid, res_buff, sizeof(res_buff->msg), getpid(), 0);
}


int write_request(MQ_BUFFER *req_buff) {
	req_buff->pid = getpid();
    key_t key = ftok(MQ_FILE_PATH, REQUEST_MQ_PROJ_ID);
	int msgqid = underlying_msgget(key, 0666 | IPC_CREAT);

    return underlying_msgsnd(msgqid, req_buff, sizeof(req_buff->msg), 0);
}


int is_rwfm_enabled() {
    MQ_BUFFER request, response;
    request.msg.msg_type = IS_RWFM_ENABLED_OP;
	sprintf(request.msg.msg_str, " ");
	write_request(&request);
    read_response(&response);
    
    return strtol(response.msg.msg_str, NULL, 10);
}


int add_host(char *host) {
    MQ_BUFFER request, response;
    request.msg.msg_type = ADD_HOST_OP;
    sprintf(request.msg.msg_str, "%s", host);
	write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int get_host_index(char *host) {
    MQ_BUFFER request, response;
    request.msg.msg_type = GET_HOST_INDEX_OP;
    sprintf(request.msg.msg_str, "%s", host);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}


int add_user_id(int host_id_index, int uid) {
    MQ_BUFFER request, response;
	request.msg.msg_type = ADD_USER_ID_OP;
    sprintf(request.msg.msg_str, "%d %d", host_id_index, uid);
	write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int get_user_id_index(int host_id_index, int uid) {
    MQ_BUFFER request, response;
	request.msg.msg_type = GET_USER_ID_INDEX_OP;
    sprintf(request.msg.msg_str, "%d %d", host_id_index, uid);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int get_number_of_users() {
    MQ_BUFFER request, response;
    request.msg.msg_type = GET_NUMBER_OF_USERS_OP;
	sprintf(request.msg.msg_str, " ");
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}


int add_group_id(int host_id_index, int gid, unsigned long long int member_set) {
    MQ_BUFFER request, response;
	request.msg.msg_type = ADD_GROUP_ID_OP;
    sprintf(request.msg.msg_str, "%d %d %llx", host_id_index, gid, member_set);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int get_group_id_index(int host_id_index, int gid) {
    MQ_BUFFER request, response;
	request.msg.msg_type = GET_GROUP_ID_INDEX_OP;
    sprintf(request.msg.msg_str, "%d %d", host_id_index, gid);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

unsigned long long int get_members_from_group_id(int group_id_index) {
    MQ_BUFFER request, response;
	request.msg.msg_type = GET_MEMBERS_FROM_GROUP_ID_OP;
    sprintf(request.msg.msg_str, "%d", group_id_index);
    write_request(&request);
    read_response(&response);
	
    return strtoull(response.msg.msg_str, NULL, 16);
}


int add_object_id(int host_id_index, unsigned long device_id, unsigned long inode_num) {
    MQ_BUFFER request, response;
	request.msg.msg_type = ADD_OBJECT_ID_OP;
    sprintf(request.msg.msg_str, "%d %lu %lu", host_id_index, device_id, inode_num);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int get_object_id_index(int host_id_index, unsigned long device_id, unsigned long inode_num) {
    MQ_BUFFER request, response;
	request.msg.msg_type = GET_OBJECT_ID_INDEX_OP;
    sprintf(request.msg.msg_str, "%d %lu %lu", host_id_index, device_id, inode_num);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int add_object(int obj_id_index, int owner, unsigned long long readers, unsigned long long writers) {
    MQ_BUFFER request, response;
	request.msg.msg_type = ADD_OBJECT_OP;
    sprintf(request.msg.msg_str, "%d %d %llx %llx", obj_id_index, owner, readers, writers);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

OBJECT get_object(int obj_id_index) {
    MQ_BUFFER request, response;
	request.msg.msg_type = GET_OBJECT_OP;
    sprintf(request.msg.msg_str, "%d", obj_id_index);
    write_request(&request);
    read_response(&response);
	char **arguments = (char**)malloc(MAX_REQUEST_LENGTH * sizeof(char*));
    get_args_from_request(arguments, response.msg.msg_str);
    OBJECT object;
    object.owner = strtoul(arguments[0], NULL, 10);
    object.readers = strtoull(arguments[1], NULL, 16);
    object.writers = strtoull(arguments[2], NULL, 16);

    return object;
}

int update_object_label(int obj_id_index, unsigned long long readers, unsigned long long writers) {
    MQ_BUFFER request, response;
	request.msg.msg_type = UPDATE_OBJECT_LABEL_OP;
    sprintf(request.msg.msg_str, "%d %llx %llx", obj_id_index, readers, writers);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}


int add_subject_id(int host_id_index, int uid, int pid) {
    MQ_BUFFER request, response;
	request.msg.msg_type = ADD_SUBJECT_ID_OP;
    sprintf(request.msg.msg_str, "%d %d %d", host_id_index, uid, pid);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int get_subject_id_index(int host_id_index, int uid, int pid) {
    MQ_BUFFER request, response;
	request.msg.msg_type = GET_SUBJECT_ID_INDEX_OP;
    sprintf(request.msg.msg_str, "%d %d %d", host_id_index, uid, pid);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int add_subject(int sub_id_index, int owner, unsigned long long readers, unsigned long long writers) {
    MQ_BUFFER request, response;
	request.msg.msg_type = ADD_SUBJECT_OP;
    sprintf(request.msg.msg_str, "%d %d %llx %llx", sub_id_index, owner, readers, writers);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

SUBJECT get_subject(int sub_id_index) {
    MQ_BUFFER request, response;
	request.msg.msg_type = GET_SUBJECT_OP;
    sprintf(request.msg.msg_str, "%d", sub_id_index);
    write_request(&request);
    read_response(&response);
	char **arguments = (char**)malloc(MAX_REQUEST_LENGTH * sizeof(char*));
    get_args_from_request(arguments, response.msg.msg_str);
    SUBJECT subject;
    subject.owner = strtoul(arguments[0], NULL, 10);
    subject.readers = strtoull(arguments[1], NULL, 16);
    subject.writers = strtoull(arguments[2], NULL, 16);

    return subject;
}

int update_subject_label(int sub_id_index, unsigned long long readers, unsigned long long writers) {
    MQ_BUFFER request, response;
	request.msg.msg_type = UPDATE_SUBJECT_LABEL_OP;
    sprintf(request.msg.msg_str, "%d %llx %llx", sub_id_index, readers, writers);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}


int add_connection(ulong src_ip, uint src_port, ulong dstn_ip, uint dstn_port, uint num_peers, int peer, USER_SET readers, USER_SET writers) {
    MQ_BUFFER request, response;
	request.msg.msg_type = ADD_CONNECTION_OP;
    sprintf(request.msg.msg_str, "%lu %u %lu %u %u %d %llx %llx", src_ip, src_port, dstn_ip, dstn_port, num_peers, peer, readers, writers);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int get_connection_index(ulong src_ip, uint src_port, ulong dstn_ip, uint dstn_port) {
	MQ_BUFFER request, response;
	request.msg.msg_type = GET_CONNECTION_INDEX_OP;
    sprintf(request.msg.msg_str, "%lu %u %lu %u", src_ip, src_port, dstn_ip, dstn_port);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

SOCKET_CONNECTION_OBJECT get_connection(int connection_index) {
    MQ_BUFFER request, response;
	request.msg.msg_type = GET_CONNECTION_OP;
    sprintf(request.msg.msg_str, "%d", connection_index);
    write_request(&request);
    read_response(&response);
	char **arguments = (char**)malloc(MAX_REQUEST_LENGTH * sizeof(char*));
    get_args_from_request(arguments, response.msg.msg_str);
    SOCKET_CONNECTION_OBJECT connection;
    connection.readers = strtoull(arguments[0], NULL, 16);
    connection.writers = strtoull(arguments[1], NULL, 16);

    return connection;
}

int update_connection_label(int connection_index, USER_SET readers, USER_SET writers) {
    MQ_BUFFER request, response;
	request.msg.msg_type = UPDATE_CONNECTION_LABEL_OP;
    sprintf(request.msg.msg_str, "%d %llx %llx", connection_index, readers, writers);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int remove_peer_from_connection(ulong src_ip, uint src_port, ulong dstn_ip, uint dstn_port, int peer_id) {
    MQ_BUFFER request, response;
	request.msg.msg_type = REMOVE_PEER_FROM_CONNECTION_OP;
    sprintf(request.msg.msg_str, "%lu %u %lu %u %d", src_ip, src_port, dstn_ip, dstn_port, peer_id);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}


int add_new_pipe(int host_id_index, ulong device_id, ulong inode_num, int pipe_ref_count, USER_SET readers, USER_SET writers) {
	MQ_BUFFER request, response;
	request.msg.msg_type = ADD_PIPE_OP;
    sprintf(request.msg.msg_str, "%d %lu %lu %d %llx %llx", host_id_index, device_id, inode_num, pipe_ref_count, readers, writers);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int increase_pipe_ref_count(int pipe_id) {
	MQ_BUFFER request, response;
	request.msg.msg_type = INCREASE_PIPE_REF_COUNT_OP;
    sprintf(request.msg.msg_str, "%d", pipe_id);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int update_pipe_label(int pipe_index, USER_SET readers, USER_SET writers) {
	MQ_BUFFER request, response;
	request.msg.msg_type = UPDATE_PIPE_LABEL_OP;
    sprintf(request.msg.msg_str, "%d %llx %llx", pipe_index, readers, writers);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int get_pipe_index(int host_id_index, ulong device_id, ulong inode_number) {
	MQ_BUFFER request, response;
	request.msg.msg_type = GET_PIPE_INDEX_OP;
    sprintf(request.msg.msg_str, "%d %lu %lu", host_id_index, device_id, inode_number);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

PIPE_OBJECT get_pipe(int pipe_id) {
	MQ_BUFFER request, response;
	request.msg.msg_type = GET_PIPE_OP;
    sprintf(request.msg.msg_str, "%d", pipe_id);
    write_request(&request);
    read_response(&response);
	char **arguments = (char**)malloc(MAX_REQUEST_LENGTH * sizeof(char*));
    get_args_from_request(arguments, response.msg.msg_str);
    PIPE_OBJECT pipe_obj;
	pipe_obj.pipe_ref_count = strtol(arguments[0], NULL, 10);
    pipe_obj.readers = strtoull(arguments[1], NULL, 16);
    pipe_obj.writers = strtoull(arguments[2], NULL, 16);

    return pipe_obj;
}

int remove_pipe(int host_id_index, ulong device_id, ulong inode_number) {
	MQ_BUFFER request, response;
	request.msg.msg_type = REMOVE_PIPE_OP;
    sprintf(request.msg.msg_str, "%d %lu %lu", host_id_index, device_id, inode_number);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}


int add_new_pipe_mapping(int sub_id_index, int pipe_index, int ref_count) {
	MQ_BUFFER request, response;
	request.msg.msg_type = ADD_NEW_PIPE_REF_MAPPING_OP;
    sprintf(request.msg.msg_str, "%d %d %d", sub_id_index, pipe_index, ref_count);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int increment_pipe_mapping_ref_count(int sub_id_index, int pipe_index) {
	MQ_BUFFER request, response;
	request.msg.msg_type = INCREMENT_PIPE_MAPPING_REF_COUNT_OP;
    sprintf(request.msg.msg_str, "%d %d", sub_id_index, pipe_index);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int decrement_pipe_mapping_ref_count(int sub_id_index, int pipe_index) {
	MQ_BUFFER request, response;
	request.msg.msg_type = DECREMENT_PIPE_MAPPING_REF_COUNT_OP;
    sprintf(request.msg.msg_str, "%d %d", sub_id_index, pipe_index);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}


int add_msgq_object(int host_id_index, int msgq_id, int owner, USER_SET readers, USER_SET writers) {
	MQ_BUFFER request, response;
	request.msg.msg_type = ADD_MSGQ_OBJECT_OP;
    sprintf(request.msg.msg_str, "%d %d %d %llx %llx", host_id_index, msgq_id, owner, readers, writers);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int get_msgq_object_index(int host_id_index, int msgq_id) {
	MQ_BUFFER request, response;
	request.msg.msg_type = GET_MSGQ_OBJECT_INDEX_OP;
    sprintf(request.msg.msg_str, "%d %d", host_id_index, msgq_id);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

MSGQ_OBJECT get_msgq_object(int host_id_index, int msgq_id) {
	MQ_BUFFER request, response;
	request.msg.msg_type = GET_MSGQ_OBJECT_OP;
    sprintf(request.msg.msg_str, "%d %d", host_id_index, msgq_id);
    write_request(&request);
    read_response(&response);
	char **arguments = (char**)malloc(MAX_REQUEST_LENGTH * sizeof(char*));
    get_args_from_request(arguments, response.msg.msg_str);
    MSGQ_OBJECT msgq_obj;
	msgq_obj.owner = strtol(arguments[0], NULL, 10);
    msgq_obj.readers = strtoull(arguments[1], NULL, 16);
    msgq_obj.writers = strtoull(arguments[2], NULL, 16);

    return msgq_obj;
}

int remove_msgq_object(int host_id_index, int msgq_id) {
	MQ_BUFFER request, response;
	request.msg.msg_type = REMOVE_MSGQ_OBJECT_OP;
    sprintf(request.msg.msg_str, "%d %d", host_id_index, msgq_id);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}


int add_sem_object(int host_id_index, int sem_id, int owner, USER_SET readers, USER_SET writers) {
	MQ_BUFFER request, response;
	request.msg.msg_type = ADD_SEM_OBJECT_OP;
    sprintf(request.msg.msg_str, "%d %d %d %llx %llx", host_id_index, sem_id, owner, readers, writers);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

int get_sem_object_index(int host_id_index, int sem_id) {
	MQ_BUFFER request, response;
	request.msg.msg_type = GET_SEM_OBJECT_INDEX_OP;
    sprintf(request.msg.msg_str, "%d %d", host_id_index, sem_id);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

SEM_OBJECT get_sem_object(int host_id_index, int sem_id) {
	MQ_BUFFER request, response;
	request.msg.msg_type = GET_SEM_OBJECT_OP;
    sprintf(request.msg.msg_str, "%d %d", host_id_index, sem_id);
    write_request(&request);
    read_response(&response);
	char **arguments = (char**)malloc(MAX_REQUEST_LENGTH * sizeof(char*));
    get_args_from_request(arguments, response.msg.msg_str);
    SEM_OBJECT sem_obj;
	sem_obj.owner = strtol(arguments[0], NULL, 10);
    sem_obj.readers = strtoull(arguments[1], NULL, 16);
    sem_obj.writers = strtoull(arguments[2], NULL, 16);

    return sem_obj;
}

int remove_sem_object(int host_id_index, int sem_id) {
	MQ_BUFFER request, response;
	request.msg.msg_type = REMOVE_SEM_OBJECT_OP;
    sprintf(request.msg.msg_str, "%d %d", host_id_index, sem_id);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}


int copy_subject_info(int src_sub_id_index, int dstn_sub_id_index) {
    MQ_BUFFER request, response;
	request.msg.msg_type = COPY_SUBJECT_FDS_OP;
    sprintf(request.msg.msg_str, "%d %d", src_sub_id_index, dstn_sub_id_index);
    write_request(&request);
    read_response(&response);
	
    return strtol(response.msg.msg_str, NULL, 10);
}

#endif
