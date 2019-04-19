/*
This file communicates with the database server using fifo path and adds new users and groups which it gets as arguments,
to the database.
add_users_and_groups communicates with the database_server using named pipe.It writes the request for adding user or groups to the database
at the write end the of the pipe using the 'write_request' function. And get the response back by using the 'read_response' function.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "database_macros.h"
#include "user_set_manipulation_functions.h"

#define USER 0
#define GROUP 1

int read_response(MQ_BUFFER *res_buff) {
    key_t key = ftok(MQ_FILE_PATH, RESPONSE_MQ_PROJ_ID);
	int msgqid = msgget(key, 0666 | IPC_CREAT);

    return msgrcv(msgqid, res_buff, sizeof(res_buff->msg), getpid(), 0);
}

int write_request(MQ_BUFFER *req_buff) {
	req_buff->pid = getpid();
    key_t key = ftok(MQ_FILE_PATH, REQUEST_MQ_PROJ_ID);
	int msgqid = msgget(key, 0666 | IPC_CREAT);

    return msgsnd(msgqid, req_buff, sizeof(req_buff->msg), 0);
}

int add_host(char *host) {
    MQ_BUFFER request, response;
    request.msg.msg_type = ADD_HOST_OP;
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

int add_group_id(int host_id_index, int gid, unsigned long long int member_set) {
    MQ_BUFFER request, response;
    request.msg.msg_type = ADD_GROUP_ID_OP;
	sprintf(request.msg.msg_str, "%d %d %llx", host_id_index, gid, member_set);
    write_request(&request);
    read_response(&response);

    return strtol(response.msg.msg_str, NULL, 10);
}

int get_group_id_index(int host_id_index, int gid, unsigned long long int members) {
    MQ_BUFFER request, response;
    request.msg.msg_type = GET_GROUP_ID_INDEX_OP;
	sprintf(request.msg.msg_str, "%d %d %llx", host_id_index, gid, members);
    write_request(&request);
    read_response(&response);

    return strtol(response.msg.msg_str, NULL, 10);
}

int main(int argc, char* argv[]) {
    if(argc<4)
        return -1;
    int host_index = add_host(argv[2]);
    if(host_index == -1)
        return -1;
    switch(strtol(argv[1], NULL, 10)) {
        case USER:
        {
            if(add_user_id(host_index, strtol(argv[3], NULL, 10))==-1)
                return -1;
            break;
        }
        case GROUP:
        {
            unsigned long long int members = 0;
            for(int i=4;i<argc;i++) {
                int user_to_add = get_user_id_index(host_index, strtol(argv[i], NULL, 10));
                if(add_user_to_label(user_to_add, &members) == -1)
                    return -1;
            }
            if(add_group_id(host_index, strtol(argv[3], NULL, 10), members)==-1)
                return -1;
            break;
        }
        default:
            return -1;
    }
    return 0;
}
