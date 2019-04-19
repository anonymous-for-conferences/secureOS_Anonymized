#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include "database_model.h"
#include "database_macros.h"

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

int set_rwfm_enabled(int rwfm_enabled) {
    MQ_BUFFER request, response;
	request.msg.msg_type = SET_RWFM_ENABLED_OP;
    sprintf(request.msg.msg_str, "%d", rwfm_enabled);
    write_request(&request);
    read_response(&response);

    return strtol(response.msg.msg_str, NULL, 10);
}

int main(int argc, char *argv[]) {
    if(argc != 2)
        return -1;
    int res = set_rwfm_enabled(strtol(argv[1], NULL, 10));
    printf("RWFM set to %d\n", res);
    return 0;
}
