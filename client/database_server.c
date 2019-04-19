/*
This is the main database server file. The server runs infinitely and listens for requests made at the read end of the fifo. Then it extracts the parameters from the 
requet,checks which database operation needs to be done and executes it using the 'database_helper_functions'. And finally writes back the response to the fifo.

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "database_helper_functions.h"
#include "database_macros.h"

int rwfm_enabled = 0;

int num_hosts = 0;
HOST * all_hosts = NULL;

int num_user_ids = 0;
USER_ID * all_user_ids = NULL;

int num_group_ids = 0;
GROUP_ID * all_group_ids = NULL;

int num_object_ids = 0;
OBJECT_ID * all_object_ids = NULL;

int num_subject_ids = 0;
SUBJECT_ID * all_subject_ids = NULL;

int num_objects = 0;
OBJECT * all_objects = NULL;

int num_subjects = 0;
SUBJECT * all_subjects = NULL;

int num_socket_connections = 0;
SOCKET_CONNECTION_OBJECT * all_socket_connections = NULL;

int num_pipe_objects = 0;
PIPE_OBJECT * all_pipe_objects = NULL;

int num_pipe_ref_maps = 0;
PIPE_REF_MAP * pipe_ref_map = NULL;

int num_msgq_objects = 0;
MSGQ_OBJECT * all_msgq_objects = NULL;

int num_sem_objects = 0;
SEM_OBJECT * all_sem_objects = NULL;

int read_request(MQ_BUFFER *req_buff) {
	key_t key = ftok(MQ_FILE_PATH, REQUEST_MQ_PROJ_ID);
	int msgqid = msgget(key, 0666 | IPC_CREAT);

    return msgrcv(msgqid, req_buff, sizeof(req_buff->msg), 0, 0);
}

int write_response(MQ_BUFFER *res_buff) {
    key_t key = ftok(MQ_FILE_PATH, RESPONSE_MQ_PROJ_ID);
	int msgqid = msgget(key, 0666 | IPC_CREAT);

    return msgsnd(msgqid, res_buff, sizeof(res_buff->msg), 0);
}

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

char * get_request_msg(int req_type) {
	switch(req_type) {
		case IS_RWFM_ENABLED_OP:
        	return "IS_RWFM_ENABLED_OP";
        case SET_RWFM_ENABLED_OP:
        	return "SET_RWFM_ENABLED_OP";
        case ADD_HOST_OP:
        	return "ADD_HOST_OP";
        case GET_HOST_INDEX_OP:
        	return "GET_HOST_INDEX_OP";
        case ADD_USER_ID_OP:
		    return "ADD_USER_ID_OP";
        case GET_USER_ID_INDEX_OP:
		    return "GET_USER_ID_INDEX_OP";
        case GET_NUMBER_OF_USERS_OP:
		    return "GET_NUMBER_OF_USERS_OP";
        case ADD_GROUP_ID_OP:
        	return "ADD_GROUP_ID_OP";
        case GET_GROUP_ID_INDEX_OP:
		    return "GET_GROUP_ID_INDEX_OP";
        case GET_MEMBERS_FROM_GROUP_ID_OP:
			return "GET_MEMBERS_FROM_GROUP_ID_OP";        
        case ADD_OBJECT_ID_OP:
		    return "ADD_OBJECT_ID_OP";
        case GET_OBJECT_ID_INDEX_OP:
	        return "GET_OBJECT_ID_INDEX_OP";
		case ADD_OBJECT_OP:
            return "ADD_OBJECT_OP";
        case GET_OBJECT_OP:
            return "GET_OBJECT_OP";
        case UPDATE_OBJECT_LABEL_OP:
            return "UPDATE_OBJECT_LABEL_OP";
        case ADD_SUBJECT_ID_OP:
            return "ADD_SUBJECT_ID_OP";
        case GET_SUBJECT_ID_INDEX_OP:
            return "GET_SUBJECT_ID_INDEX_OP";
        case ADD_SUBJECT_OP:
            return "ADD_SUBJECT_OP";
        case GET_SUBJECT_OP:
            return "GET_SUBJECT_OP";
        case UPDATE_SUBJECT_LABEL_OP:
            return "UPDATE_SUBJECT_LABEL_OP";
		case ADD_CONNECTION_OP:
            return "ADD_CONNECTION_OP";
		case GET_CONNECTION_INDEX_OP:
            return "GET_CONNECTION_INDEX_OP";
		case GET_CONNECTION_OP:
            return "GET_CONNECTION_OP";
		case UPDATE_CONNECTION_LABEL_OP:
            return "UPDATE_CONNECTION_LABEL_OP";
        case REMOVE_PEER_FROM_CONNECTION_OP:
            return "REMOVE_PEER_FROM_CONNECTION_OP";
		case ADD_PIPE_OP:
            return "ADD_PIPE_OP";
		case INCREASE_PIPE_REF_COUNT_OP:
            return "INCREASE_PIPE_REF_COUNT_OP";
		case GET_PIPE_INDEX_OP:
            return "GET_PIPE_INDEX_OP";
		case GET_PIPE_OP:
            return "GET_PIPE_OP";
		case UPDATE_PIPE_LABEL_OP:
            return "UPDATE_PIPE_LABEL_OP";
		case REMOVE_PIPE_OP:
            return "REMOVE_PIPE_OP";
		case ADD_NEW_PIPE_REF_MAPPING_OP:
            return "ADD_NEW_PIPE_REF_MAPPING_OP";
		case INCREMENT_PIPE_MAPPING_REF_COUNT_OP:
            return "INCREMENT_PIPE_MAPPING_REF_COUNT_OP";
		case DECREMENT_PIPE_MAPPING_REF_COUNT_OP:
            return "DECREMENT_PIPE_MAPPING_REF_COUNT_OP";
		case ADD_MSGQ_OBJECT_OP:
            return "ADD_MSGQ_OBJECT_OP";
		case GET_MSGQ_OBJECT_INDEX_OP:
            return "GET_MSGQ_OBJECT_INDEX_OP";
		case GET_MSGQ_OBJECT_OP:
            return "GET_MSGQ_OBJECT_OP";
		case REMOVE_MSGQ_OBJECT_OP:
            return "REMOVE_MSGQ_OBJECT_OP";
		case ADD_SEM_OBJECT_OP:
            return "ADD_MSGQ_OBJECT_OP";
		case GET_SEM_OBJECT_INDEX_OP:
            return "GET_MSGQ_OBJECT_INDEX_OP";
		case GET_SEM_OBJECT_OP:
            return "GET_MSGQ_OBJECT_OP";
		case REMOVE_SEM_OBJECT_OP:
            return "REMOVE_MSGQ_OBJECT_OP";
        case COPY_SUBJECT_FDS_OP:
            return "COPY_SUBJECT_FDS_OP";
        default:
            return "ERROR";
	}
}

int do_operation(int operation, char **req_args, int num_args, long pid) {
    MQ_BUFFER response;
	response.pid = pid;

    switch(operation) {

        case IS_RWFM_ENABLED_OP:
        {
            if(num_args!=0)
                return -1;
            response.msg.msg_type = IS_RWFM_ENABLED_OP;
			sprintf(response.msg.msg_str, "%d", rwfm_enabled);
            write_response(&response);
            break;
        }
        case SET_RWFM_ENABLED_OP:
        {
            if(num_args!=1)
                return -1;
            rwfm_enabled = strtol(req_args[0], NULL, 10);
            response.msg.msg_type = SET_RWFM_ENABLED_OP;
			sprintf(response.msg.msg_str, "%d", rwfm_enabled);
            write_response(&response);
            break;
        }


        case ADD_HOST_OP:
        {
            if(num_args!=1)
                return -1;
            response.msg.msg_type = ADD_HOST_OP;
			sprintf(response.msg.msg_str, "%d", add_host(req_args[0]));
            write_response(&response);
            break;
        }
        case GET_HOST_INDEX_OP:
        {
            if(num_args != 1)
                return -1;
            response.msg.msg_type = GET_HOST_INDEX_OP;
			sprintf(response.msg.msg_str, "%d", get_host_index(req_args[0]));
            write_response(&response);
            break;
        }


        case ADD_USER_ID_OP:
        {
            if(num_args != 2)
                return -1;
            USER_ID user_id_add;
            user_id_add.host_id_index = strtol(req_args[0], NULL, 10);
            user_id_add.uid = strtol(req_args[1], NULL, 10);
            response.msg.msg_type = ADD_USER_ID_OP;
			sprintf(response.msg.msg_str, "%d", add_user_id(user_id_add));
            write_response(&response);
            break;
        }
        case GET_USER_ID_INDEX_OP:
        {
            if(num_args != 2)
                return -1;
            USER_ID user_id_get;
            user_id_get.host_id_index = strtol(req_args[0], NULL, 10);
            user_id_get.uid = strtol(req_args[1], NULL, 10);
            response.msg.msg_type = GET_USER_ID_INDEX_OP;
			sprintf(response.msg.msg_str, "%d", get_user_id_index(user_id_get));
            write_response(&response);
            break;
        }
        case GET_NUMBER_OF_USERS_OP:
        {
            if(num_args != 0)
                return -1;
            response.msg.msg_type = GET_NUMBER_OF_USERS_OP;
			sprintf(response.msg.msg_str, "%d", get_number_of_users());
            write_response(&response);
            break;
        }


        case ADD_GROUP_ID_OP:
        {
            if(num_args != 3)
                return -1;
            GROUP_ID group_id_add;
            group_id_add.host_id_index = strtol(req_args[0], NULL, 10);
            group_id_add.gid = strtol(req_args[1], NULL, 10);
            group_id_add.members = strtoull(req_args[2], NULL, 16);
            response.msg.msg_type = ADD_GROUP_ID_OP;
			sprintf(response.msg.msg_str, "%d", add_group_id(group_id_add));
            write_response(&response);
            break;
        }
        case GET_GROUP_ID_INDEX_OP:
        {
            if(num_args != 2)
                return -1;
            GROUP_ID group_id_get;
            group_id_get.host_id_index = strtol(req_args[0], NULL, 10);
            group_id_get.gid = strtol(req_args[1], NULL, 10);
            response.msg.msg_type = GET_GROUP_ID_INDEX_OP;
			sprintf(response.msg.msg_str, "%d", get_group_id_index(group_id_get));
            write_response(&response);
            break;
        }
        case GET_MEMBERS_FROM_GROUP_ID_OP:
        {
            if(num_args != 1)
                return -1;
            int group_id_index = strtol(req_args[0], NULL, 10);
            response.msg.msg_type = GET_MEMBERS_FROM_GROUP_ID_OP;
			sprintf(response.msg.msg_str, "%llx", get_members_from_group_id(group_id_index));
            write_response(&response);
            break;
        }


        case ADD_OBJECT_ID_OP:
        {
            if(num_args != 3)
                return -1;
            OBJECT_ID object_id_add;
            object_id_add.host_id_index = strtol(req_args[0], NULL, 10);
            object_id_add.device_id = strtoul(req_args[1], NULL, 10);
            object_id_add.inode_number = strtoul(req_args[2], NULL, 10);
            response.msg.msg_type = ADD_OBJECT_ID_OP;
			sprintf(response.msg.msg_str, "%d", add_object_id(object_id_add));
            write_response(&response);
            break;
        }
        case GET_OBJECT_ID_INDEX_OP:
        {
            if(num_args != 3)
                return -1;
            OBJECT_ID object_id_get;
            object_id_get.host_id_index = strtol(req_args[0], NULL, 10);
            object_id_get.device_id = strtoul(req_args[1], NULL, 10);
            object_id_get.inode_number = strtoul(req_args[2], NULL, 10);
            response.msg.msg_type = GET_OBJECT_ID_INDEX_OP;
			sprintf(response.msg.msg_str, "%d", get_object_id_index(object_id_get));
            write_response(&response);
            break;
        }
		case ADD_OBJECT_OP:
        {
            if(num_args != 4)
                return -1;
            OBJECT object_add;
            object_add.obj_id_index = strtol(req_args[0], NULL, 10);
            object_add.owner = strtol(req_args[1], NULL, 10);
            object_add.readers = strtoull(req_args[2], NULL, 16);
            object_add.writers = strtoull(req_args[3], NULL, 16);
            response.msg.msg_type = ADD_OBJECT_OP;
			sprintf(response.msg.msg_str, "%d", add_object(object_add));
            write_response(&response);
            break;
        }
        case GET_OBJECT_OP:
        {
            if(num_args != 1)
                return -1;
            int obj_id_index = strtol(req_args[0], NULL, 10);
            int obj_index = get_object_from_obj_id_index(obj_id_index);
            if(obj_index == -1)
                return -2;
            OBJECT obj = all_objects[obj_index];
            response.msg.msg_type = GET_OBJECT_OP;
			sprintf(response.msg.msg_str, "%u %llx %llx",obj.owner,obj.readers,obj.writers);
            write_response(&response);
            break;
        }
        case UPDATE_OBJECT_LABEL_OP:
        {
            if(num_args != 3)
                return -1;
            int update_obj_id = strtol(req_args[0], NULL, 10);
            USER_SET new_obj_readers = strtoull(req_args[1], NULL, 16);
            USER_SET new_obj_writers = strtoull(req_args[2], NULL, 16);
            response.msg.msg_type = UPDATE_OBJECT_LABEL_OP;
			sprintf(response.msg.msg_str, "%d", update_object_label(update_obj_id, new_obj_readers, new_obj_writers));
            write_response(&response);
            break;
        }


        case ADD_SUBJECT_ID_OP:
        {
            if(num_args != 3)
                return -1;
            SUBJECT_ID subject_id_add;
            subject_id_add.host_id_index = strtol(req_args[0], NULL, 10);
            subject_id_add.uid = strtol(req_args[1], NULL, 10);
            subject_id_add.pid = strtol(req_args[2], NULL, 10);
            response.msg.msg_type = ADD_SUBJECT_ID_OP;
			sprintf(response.msg.msg_str, "%d", add_subject_id(subject_id_add));
            write_response(&response);
            break;
        }
        case GET_SUBJECT_ID_INDEX_OP:
        {
            if(num_args != 3)
                return -1;
            SUBJECT_ID subject_id_get;
            subject_id_get.host_id_index = strtol(req_args[0], NULL, 10);
            subject_id_get.uid = strtol(req_args[1], NULL, 10);
            subject_id_get.pid = strtol(req_args[2], NULL, 10);
            response.msg.msg_type = GET_SUBJECT_ID_INDEX_OP;
			sprintf(response.msg.msg_str, "%d", get_subject_id_index(subject_id_get));
            write_response(&response);
            break;
        }
        case ADD_SUBJECT_OP:
        {
            if(num_args != 4)
                return -1;
            SUBJECT subject_add;
            subject_add.sub_id_index = strtol(req_args[0], NULL, 10);
            subject_add.owner = strtol(req_args[1], NULL, 10);
            subject_add.readers = strtoull(req_args[2], NULL, 16);
            subject_add.writers = strtoull(req_args[3], NULL, 16);
            response.msg.msg_type = ADD_SUBJECT_OP;
			sprintf(response.msg.msg_str, "%d", add_subject(subject_add));
            write_response(&response);
            break;
        }
        case GET_SUBJECT_OP:
        {
            if(num_args != 1)
                return -1;
            int sub_id_index = strtol(req_args[0], NULL, 10);
            int sub_index = get_subject_from_sub_id_index(sub_id_index);
            if(sub_index == -1)
                return -2;
            SUBJECT sub = all_subjects[sub_index];
            response.msg.msg_type = GET_SUBJECT_OP;
			sprintf(response.msg.msg_str, "%u %llx %llx",sub.owner,sub.readers,sub.writers);
            write_response(&response);
            break;
        }
        case UPDATE_SUBJECT_LABEL_OP:
        {
            if(num_args != 3)
                return -1;
            int update_sub_id = strtol(req_args[0], NULL, 10);
            USER_SET new_sub_readers = strtoull(req_args[1], NULL, 16);
            USER_SET new_sub_writers = strtoull(req_args[2], NULL, 16);
            response.msg.msg_type = UPDATE_SUBJECT_LABEL_OP;
			sprintf(response.msg.msg_str, "%d", update_subject_label(update_sub_id, new_sub_readers, new_sub_writers));
            write_response(&response);
            break;
        }


		case ADD_CONNECTION_OP:
		{
			if(num_args != 8)
                return -1;
            SOCKET_CONNECTION_OBJECT connection;
			connection.src.ip = strtoul(req_args[0], NULL, 10);
			connection.src.port = strtol(req_args[1], NULL, 10);
			connection.dstn.ip = strtoul(req_args[2], NULL, 10);
			connection.dstn.port = strtol(req_args[3], NULL, 10);
			connection.num_peers = strtol(req_args[4], NULL, 10);
            connection.peer_ids[0] = strtol(req_args[5], NULL, 10);
            connection.peer_ids[1] = -1;
            connection.readers = strtoull(req_args[6], NULL, 16);
            connection.writers = strtoull(req_args[7], NULL, 16);
            response.msg.msg_type = ADD_CONNECTION_OP;
			sprintf(response.msg.msg_str, "%d", add_connection(connection));
            write_response(&response);
            break;
		}
		case GET_CONNECTION_INDEX_OP:
		{
			if(num_args != 4)
                return -1;
			ADDRESS src, dstn;
			src.ip = strtoul(req_args[0], NULL, 10);
            src.port = strtoul(req_args[1], NULL, 10);
			dstn.ip = strtoul(req_args[2], NULL, 10);
            dstn.port = strtoul(req_args[3], NULL, 10);
            response.msg.msg_type = GET_CONNECTION_INDEX_OP;
			sprintf(response.msg.msg_str, "%d", get_connection_index(src, dstn));
            write_response(&response);
            break;
		}
		case GET_CONNECTION_OP:
        {
            if(num_args != 1)
                return -1;
            int connection_index = strtol(req_args[0], NULL, 10);
            SOCKET_CONNECTION_OBJECT connection = all_socket_connections[connection_index];
            response.msg.msg_type = GET_CONNECTION_OP;
			sprintf(response.msg.msg_str, "%llx %llx", connection.readers, connection.writers);
            write_response(&response);
            break;
        }
		case UPDATE_CONNECTION_LABEL_OP:
        {
            if(num_args != 3)
                return -1;
            int connection_index = strtol(req_args[0], NULL, 10);
            USER_SET readers = strtoull(req_args[1], NULL, 16);
            USER_SET writers = strtoull(req_args[2], NULL, 16);
            response.msg.msg_type = UPDATE_CONNECTION_LABEL_OP;
			sprintf(response.msg.msg_str, "%d", update_connection_label(connection_index, readers, writers));
            write_response(&response);
            break;
        }
        case REMOVE_PEER_FROM_CONNECTION_OP:
        {
            if(num_args != 5)
                return -1;
			ADDRESS src, dstn;
            src.ip = strtoul(req_args[0], NULL, 10);
            src.port = strtoul(req_args[1], NULL, 10);
			dstn.ip = strtoul(req_args[2], NULL, 10);
            dstn.port = strtoul(req_args[3], NULL, 10);
			int peer_id = strtol(req_args[4], NULL, 10);
            response.msg.msg_type = REMOVE_PEER_FROM_CONNECTION_OP;
			sprintf(response.msg.msg_str, "%d", remove_peer_from_connection(src, dstn, peer_id));
            write_response(&response);
            break;
        }


		case ADD_PIPE_OP:
		{
			if(num_args != 6)
                return -1;
			PIPE_OBJECT pipe_obj;
            pipe_obj.host_id_index = strtol(req_args[0], NULL, 10);
            pipe_obj.device_id = strtoul(req_args[1], NULL, 10);
			pipe_obj.inode_number = strtoul(req_args[2], NULL, 10);
            pipe_obj.pipe_ref_count = strtol(req_args[3], NULL, 10);
			pipe_obj.readers = strtoull(req_args[4], NULL, 16);
            pipe_obj.writers = strtoull(req_args[5], NULL, 16);
            response.msg.msg_type = ADD_PIPE_OP;
			sprintf(response.msg.msg_str, "%d", add_new_pipe(pipe_obj));
            write_response(&response);
            break;
		}
		case INCREASE_PIPE_REF_COUNT_OP:
		{
			if(num_args != 1)
                return -1;
            int pipe_id = strtol(req_args[0], NULL, 10);
            response.msg.msg_type = INCREASE_PIPE_REF_COUNT_OP;
			sprintf(response.msg.msg_str, "%d", increase_pipe_ref_count(pipe_id));
            write_response(&response);
            break;
		}
		case GET_PIPE_INDEX_OP:
		{
			if(num_args != 3)
                return -1;
            int host_id_index = strtol(req_args[0], NULL, 10);
            int device_id = strtoul(req_args[1], NULL, 10);
			int inode_number = strtoul(req_args[2], NULL, 10);
            response.msg.msg_type = GET_PIPE_INDEX_OP;
			sprintf(response.msg.msg_str, "%d", get_pipe_index(host_id_index, device_id, inode_number));
            write_response(&response);
            break;
		}
		case GET_PIPE_OP:
		{
            if(num_args != 1)
                return -1;
            int pipe_id = strtol(req_args[0], NULL, 10);
            PIPE_OBJECT pipe_obj = all_pipe_objects[pipe_id];
            response.msg.msg_type = GET_PIPE_OP;
			sprintf(response.msg.msg_str, "%d %llx %llx", pipe_obj.pipe_ref_count, pipe_obj.readers, pipe_obj.writers);
            write_response(&response);
            break;
        }
		case UPDATE_PIPE_LABEL_OP:
		{
			if(num_args != 3)
                return -1;
            int pipe_id = strtol(req_args[0], NULL, 10);
            USER_SET readers = strtoull(req_args[1], NULL, 16);
            USER_SET writers = strtoull(req_args[2], NULL, 16);
            response.msg.msg_type = UPDATE_PIPE_LABEL_OP;
			sprintf(response.msg.msg_str, "%d", update_pipe_label(pipe_id, readers, writers));
            write_response(&response);
            break;
		}
		case REMOVE_PIPE_OP:
		{
            if(num_args != 3)
                return -1;
            int host_id_index = strtol(req_args[0], NULL, 10);
            int device_id = strtoul(req_args[1], NULL, 10);
			int inode_number = strtoul(req_args[2], NULL, 10);
            response.msg.msg_type = REMOVE_PIPE_OP;
			sprintf(response.msg.msg_str, "%d", remove_pipe(host_id_index, device_id, inode_number));
            write_response(&response);
            break;
        }


        case ADD_NEW_PIPE_REF_MAPPING_OP:
        {
            if(num_args != 3)
                return -1;
            PIPE_REF_MAP pipe_map;
            pipe_map.sub_id_index = strtol(req_args[0], NULL, 10);
            pipe_map.pipe_index = strtol(req_args[1], NULL, 10);
            pipe_map.ref_count = strtol(req_args[2], NULL, 10);
            response.msg.msg_type = ADD_NEW_PIPE_REF_MAPPING_OP;
			sprintf(response.msg.msg_str, "%d", add_new_pipe_mapping(pipe_map));
            write_response(&response);
            break;
        }
		case INCREMENT_PIPE_MAPPING_REF_COUNT_OP:
        {
            if(num_args != 2)
                return -1;
            int sub_id_index = strtol(req_args[0], NULL, 10);
            int pipe_index = strtol(req_args[1], NULL, 10);
            response.msg.msg_type = INCREMENT_PIPE_MAPPING_REF_COUNT_OP;
			sprintf(response.msg.msg_str, "%d", increment_pipe_mapping_ref_count(sub_id_index, pipe_index));
            write_response(&response);
            break;
        }
		case DECREMENT_PIPE_MAPPING_REF_COUNT_OP:
        {
            if(num_args != 2)
                return -1;
            int sub_id_index = strtol(req_args[0], NULL, 10);
            int pipe_index = strtol(req_args[1], NULL, 10);
            response.msg.msg_type = DECREMENT_PIPE_MAPPING_REF_COUNT_OP;
			sprintf(response.msg.msg_str, "%d", decrement_pipe_mapping_ref_count(sub_id_index, pipe_index));
            write_response(&response);
            break;
        }


		case ADD_MSGQ_OBJECT_OP:
        {
            if(num_args != 5)
                return -1;
            MSGQ_OBJECT object_add;
            object_add.host_index = strtol(req_args[0], NULL, 10);
            object_add.msgq_id = strtol(req_args[1], NULL, 10);
            object_add.owner = strtol(req_args[2], NULL, 10);
            object_add.readers = strtoull(req_args[3], NULL, 16);
            object_add.writers = strtoull(req_args[4], NULL, 16);
            response.msg.msg_type = ADD_MSGQ_OBJECT_OP;
			sprintf(response.msg.msg_str, "%d", add_msgq_object(object_add));
            write_response(&response);
            break;
        }
		case GET_MSGQ_OBJECT_INDEX_OP:
		{
			if(num_args != 2)
                return -1;
			int host_index = strtol(req_args[0], NULL, 10);
			int msgq_id = strtol(req_args[1], NULL, 10);
			response.msg.msg_type = GET_MSGQ_OBJECT_INDEX_OP;
			sprintf(response.msg.msg_str, "%d", get_msgq_object_index(host_index, msgq_id));
            write_response(&response);
			break;
		}
		case GET_MSGQ_OBJECT_OP:
		{
			if(num_args != 2)
                return -1;
			int host_index = strtol(req_args[0], NULL, 10);
			int msgq_id = strtol(req_args[1], NULL, 10);
			MSGQ_OBJECT msgq_object = all_msgq_objects[get_msgq_object_index(host_index, msgq_id)];
			response.msg.msg_type = GET_MSGQ_OBJECT_OP;
			sprintf(response.msg.msg_str, "%d %llx %llx", msgq_object.owner, msgq_object.readers, msgq_object.writers);
            write_response(&response);
			break;
		}
		case REMOVE_MSGQ_OBJECT_OP:
		{
			if(num_args != 2)
                return -1;
			int host_index = strtol(req_args[0], NULL, 10);
			int msgq_id = strtol(req_args[1], NULL, 10);
			response.msg.msg_type = REMOVE_MSGQ_OBJECT_OP;
			sprintf(response.msg.msg_str, "%d", remove_msgq_object(host_index, msgq_id));
            write_response(&response);
			break;
		}


		case ADD_SEM_OBJECT_OP:
        {
            if(num_args != 5)
                return -1;
            SEM_OBJECT object_add;
            object_add.host_index = strtol(req_args[0], NULL, 10);
            object_add.sem_id = strtol(req_args[1], NULL, 10);
            object_add.owner = strtol(req_args[2], NULL, 10);
            object_add.readers = strtoull(req_args[3], NULL, 16);
            object_add.writers = strtoull(req_args[4], NULL, 16);
            response.msg.msg_type = ADD_SEM_OBJECT_OP;
			sprintf(response.msg.msg_str, "%d", add_sem_object(object_add));
            write_response(&response);
            break;
        }
		case GET_SEM_OBJECT_INDEX_OP:
		{
			if(num_args != 2)
                return -1;
			int host_index = strtol(req_args[0], NULL, 10);
			int sem_id = strtol(req_args[1], NULL, 10);
			response.msg.msg_type = GET_SEM_OBJECT_INDEX_OP;
			sprintf(response.msg.msg_str, "%d", get_sem_object_index(host_index, sem_id));
            write_response(&response);
			break;
		}
		case GET_SEM_OBJECT_OP:
		{
			if(num_args != 2)
                return -1;
			int host_index = strtol(req_args[0], NULL, 10);
			int sem_id = strtol(req_args[1], NULL, 10);
			SEM_OBJECT sem_object = all_sem_objects[get_sem_object_index(host_index, sem_id)];
			response.msg.msg_type = GET_SEM_OBJECT_OP;
			sprintf(response.msg.msg_str, "%d %llx %llx", sem_object.owner, sem_object.readers, sem_object.writers);
            write_response(&response);
			break;
		}
		case REMOVE_SEM_OBJECT_OP:
		{
			if(num_args != 2)
                return -1;
			int host_index = strtol(req_args[0], NULL, 10);
			int sem_id = strtol(req_args[1], NULL, 10);
			response.msg.msg_type = REMOVE_SEM_OBJECT_OP;
			sprintf(response.msg.msg_str, "%d", remove_sem_object(host_index, sem_id));
            write_response(&response);
			break;
		}


        case COPY_SUBJECT_FDS_OP:
        {
            if(num_args != 2)
                return -1;
            int src_sub_id_index = strtol(req_args[0], NULL, 10);
            int dstn_sub_id_index = strtol(req_args[1], NULL, 10);
            response.msg.msg_type = COPY_SUBJECT_FDS_OP;
			sprintf(response.msg.msg_str, "%d", copy_subject_info(src_sub_id_index, dstn_sub_id_index));
            write_response(&response);
            break;
        }


        default:
            return -2;
    }

	printf("Operation response %d : %s\n",operation,response.msg.msg_str);

    return 0;
}

int start_server() {
    while(1) {
        MQ_BUFFER request;
		int read_val;
        while(((read_val = read_request(&request)) == -1) && (errno == EINTR));
		if(read_val == -1) {
			perror("Stopping database server due to error in read");
			return -1;
		}
        printf("Received request of type %s with args:%s\n", get_request_msg(request.msg.msg_type), request.msg.msg_str);
        char **req_args = (char**)malloc(MAX_REQUEST_LENGTH * sizeof(char*));
        int num_args = get_args_from_request(req_args, request.msg.msg_str);
        int operation = request.msg.msg_type;
        int ret = do_operation(operation, req_args, num_args, request.pid);
        printf("Operation result:%d\n\n",ret);
        if(ret!=0) {
			printf("Stopping database server!\n\n");
			key_t key = ftok(MQ_FILE_PATH, RESPONSE_MQ_PROJ_ID);
			int msgqid = msgget(key, 0666 | IPC_CREAT);
			msgctl(msgqid, IPC_RMID, NULL);
            return ret;
		}
    }
}

int main() {
    start_server();

    return 0;
}
