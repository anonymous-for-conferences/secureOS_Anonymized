#ifndef _INFER_OBJECT_LABELS_H_
#define _INFER_OBJECT_LABELS_H_

#include "database_queries.h"
#include "user_set_manipulation_functions.h"
#include <sys/stat.h>

USER_SET get_all_users(int number_of_users) {
    USER_SET ans = 1;
    for(int i=0; i<number_of_users; i++) {
        ans *= 2;
    }
    return ans-1;
}

int infer_file_labels(OBJECT * object, struct stat * object_info, int host_id_index) {
    int user_id_index = get_user_id_index(host_id_index, object_info->st_uid);
	object->readers = 0;
	object->writers = 0;

	int is_user_reader = object_info->st_mode & S_IRUSR ? 1 : 0;
    int is_user_writer = object_info->st_mode & S_IWUSR ? 1 : 0;

    if(user_id_index==-1)
        return -1;
    
    if(is_user_reader)
        add_user_to_label(user_id_index, &(object->readers));

    if(is_user_writer)
        add_user_to_label(user_id_index, &(object->writers));

    int group_id_index = get_group_id_index(host_id_index, object_info->st_gid);    
    USER_SET group_members = get_members_from_group_id(group_id_index);
    
    if(object_info->st_mode & S_IRGRP)
        object->readers |= group_members;

    if(object_info->st_mode & S_IWGRP)
        object->writers |= group_members; 

    if(!is_user_reader)
        remove_user_from_set(user_id_index, &(object->readers));
    
    if(!is_user_writer)
        remove_user_from_set(user_id_index, &(object->writers));
    
    int number_of_users = get_number_of_users();
    USER_SET all_users = get_all_users(number_of_users);

    USER_SET other_members = all_users & (~group_members);
    remove_user_from_set(user_id_index, &other_members);
    
    if(object_info->st_mode & S_IROTH)
        object->readers |= other_members;

    if(object_info->st_mode & S_IWOTH)
        object->writers |= other_members;

    return 0;       
}

int infer_msgq_labels(MSGQ_OBJECT * msgq_object, struct msqid_ds * msgq_info, int host_id_index) {
    int user_id_index = get_user_id_index(host_id_index, msgq_info->msg_perm.uid);
	msgq_object->readers = 0;
	msgq_object->writers = 0;

	int is_user_reader = msgq_info->msg_perm.mode & S_IRUSR ? 1 : 0;
    int is_user_writer = msgq_info->msg_perm.mode & S_IWUSR ? 1 : 0;

    if(user_id_index==-1)
        return -1;
    
    if(is_user_reader)
        add_user_to_label(user_id_index, &(msgq_object->readers));

    if(is_user_writer)
        add_user_to_label(user_id_index, &(msgq_object->writers));

    int group_id_index = get_group_id_index(host_id_index, msgq_info->msg_perm.gid);    
    USER_SET group_members = get_members_from_group_id(group_id_index);
    
    if(msgq_info->msg_perm.mode & S_IRGRP)
        msgq_object->readers |= group_members;

    if(msgq_info->msg_perm.mode & S_IWGRP)
        msgq_object->writers |= group_members; 

    if(!is_user_reader)
        remove_user_from_set(user_id_index, &(msgq_object->readers));
    
    if(!is_user_writer)
        remove_user_from_set(user_id_index, &(msgq_object->writers));
    
    int number_of_users = get_number_of_users();
    USER_SET all_users = get_all_users(number_of_users);

    USER_SET other_members = all_users & (~group_members);
    remove_user_from_set(user_id_index, &other_members);
    
    if(msgq_info->msg_perm.mode & S_IROTH)
        msgq_object->readers |= other_members;

    if(msgq_info->msg_perm.mode & S_IWOTH)
        msgq_object->writers |= other_members;

    return 0;       
}

int infer_sem_labels(SEM_OBJECT * sem_object, struct semid_ds * sem_info, int host_id_index) {
    int user_id_index = get_user_id_index(host_id_index, sem_info->sem_perm.uid);
	sem_object->readers = 0;
	sem_object->writers = 0;

	int is_user_reader = sem_info->sem_perm.mode & S_IRUSR ? 1 : 0;
    int is_user_writer = sem_info->sem_perm.mode & S_IWUSR ? 1 : 0;

    if(user_id_index==-1)
        return -1;
    
    if(is_user_reader)
        add_user_to_label(user_id_index, &(sem_object->readers));

    if(is_user_writer)
        add_user_to_label(user_id_index, &(sem_object->writers));

    int group_id_index = get_group_id_index(host_id_index, sem_info->sem_perm.gid);    
    USER_SET group_members = get_members_from_group_id(group_id_index);
    
    if(sem_info->sem_perm.mode & S_IRGRP)
        sem_object->readers |= group_members;

    if(sem_info->sem_perm.mode & S_IWGRP)
        sem_object->writers |= group_members; 

    if(!is_user_reader)
        remove_user_from_set(user_id_index, &(sem_object->readers));
    
    if(!is_user_writer)
        remove_user_from_set(user_id_index, &(sem_object->writers));
    
    int number_of_users = get_number_of_users();
    USER_SET all_users = get_all_users(number_of_users);

    USER_SET other_members = all_users & (~group_members);
    remove_user_from_set(user_id_index, &other_members);
    
    if(sem_info->sem_perm.mode & S_IROTH)
        sem_object->readers |= other_members;

    if(sem_info->sem_perm.mode & S_IWOTH)
        sem_object->writers |= other_members;

    return 0;       
}

#endif
