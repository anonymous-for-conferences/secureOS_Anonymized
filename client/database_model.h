/*
This file contains all the structures for all the objects which we keep in our database
*/

#ifndef _DB_MODEL_
#define _DB_MODEL_

#include "database_macros.h"

typedef unsigned long long int USER_SET;
typedef char * HOST;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef struct mq_msg {
	int msg_type;
	char msg_str[MAX_REQUEST_LENGTH];
} MQ_MSG;

typedef struct mq_buffer {
	long pid;
	MQ_MSG msg;
} MQ_BUFFER;

typedef struct user_id {
    uint uid;
    uint host_id_index;
} USER_ID;

typedef struct group_id {
    uint gid;
    uint host_id_index;
    USER_SET members;
} GROUP_ID;

typedef struct object_id {
    ulong device_id;
    ulong inode_number;
    int host_id_index;
} OBJECT_ID;

typedef struct object {
    uint obj_id_index;
    uint owner;//index of the user id in ALL_UID array
    USER_SET readers;//the bits in places where uid are present from ALL_UID are 1
    USER_SET writers;//the bits in places where uid are present from ALL_UID are 1
} OBJECT;

typedef struct subject_id {
    uint uid;
    uint pid;
    int host_id_index;
} SUBJECT_ID;

typedef struct subject {
    uint sub_id_index;
    uint owner;
    USER_SET readers;
    USER_SET writers;
} SUBJECT;

typedef struct address {
	ulong ip;
	uint port;
} ADDRESS;

typedef struct socket_connection_object {
	ADDRESS src, dstn;
	uint num_peers;
    int peer_ids[2];
    USER_SET readers;
    USER_SET writers; 
} SOCKET_CONNECTION_OBJECT;

typedef struct pipe_object {
	int host_id_index;
	ulong device_id;
	ulong inode_number;
	int pipe_ref_count;
	USER_SET readers;
	USER_SET writers;
} PIPE_OBJECT;

typedef struct pipe_ref_map {
	uint sub_id_index;
	uint pipe_index;
	uint ref_count;
} PIPE_REF_MAP;

typedef struct msgq_object {
    uint host_index;
	uint msgq_id;
    uint owner;
    USER_SET readers;
    USER_SET writers;
} MSGQ_OBJECT;

typedef struct sem_object {
	uint host_index;
	uint sem_id;
    uint owner;
    USER_SET readers;
    USER_SET writers;
} SEM_OBJECT;

struct semid_ds {
    struct ipc_perm sem_perm;  /* Ownership and permissions */
    time_t          sem_otime; /* Last semop time */
    time_t          sem_ctime; /* Last change time */
    unsigned long   sem_nsems; /* No. of semaphores in set */
};

struct sembuf {
	unsigned short  sem_num;	/* semaphore index in array */
	short		sem_op;		/* semaphore operation */
	short		sem_flg;	/* operation flags */
};

union semun {
	int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO */
};

#endif
