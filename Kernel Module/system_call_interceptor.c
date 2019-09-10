/**
 * @file    system_call_interceptor.c
 * @author  Anonymized
 * @date    1 Sept 2019
 * @version 0.1
 * @brief   A system call interceptor kernel module that replaces
 *          default system calls with rwfm secured system calls
 *          WARNING:
 *          THIS IS A VERY DANGEROUS PIECE OF CODE
 *          PLEASE DO NOT MODIFY OR TRY TO EXECUTE IF YOU DO NOT KNOW WHAT YOU ARE DOING
 *          IT CAN POTENTIALLY DAMAGE YOUR OS AND DEEM IT UNBOOTABLE
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <linux/kallsyms.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/utsname.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/uidgid.h>

#define MAX_HOST_SIZE 128

static DEFINE_MUTEX(clone_lock);
  
MODULE_LICENSE("GPL");
  
MODULE_AUTHOR("Anonymized");

MODULE_DESCRIPTION("A system call interceptor LKM that replaces default system calls with rwfm secured system calls");

MODULE_VERSION("0.1");

typedef unsigned long long int USER_SET;
typedef char * HOST;
typedef unsigned int uint;
typedef unsigned long ulong;

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

int add_user_to_label(int user_to_add, USER_SET * label) {
    unsigned long long int max = -1;
    USER_SET tmp = 1;

    if(*label == max)
        return -1;

    tmp <<= user_to_add;

    *label |= tmp;

    return 0;
}

int remove_user_from_set(int user_to_remove, USER_SET * set) {
    unsigned long long int max = -1;
    USER_SET tmp = 1;

    if(*set == max)
        return -1;

    tmp <<= user_to_remove;
    tmp = ~tmp;

    *set &= tmp;

    return 0;
}

int is_user_in_set(int user_id_index, USER_SET * set) {
    USER_SET tmp = 1;
    tmp <<= user_id_index;
	
    return ((*set) & tmp) ? 1 : 0;
}

USER_SET set_union(USER_SET * set1, USER_SET * set2) {
    return *set1 | *set2;
}

USER_SET set_intersection(USER_SET * set1, USER_SET * set2) {
    return *set1 & *set2;
}

//Is set1 subset of set2
int is_subset_of(USER_SET * set1, USER_SET * set2) {
    USER_SET result = *set1 & *set2;
    if(*set1 == result)
        return 1;

    return 0;
}

//Is set1 superset of set2
int is_superset_of(USER_SET * set1, USER_SET * set2) {
    USER_SET result = *set1 | *set2;
    if(*set1 == result)
        return 1;

    return 0;
}

USER_SET get_all_users(int number_of_users) {
    USER_SET ans = 1;
    int i;
    for(i=0; i<number_of_users; i++) {
        ans *= 2;
    }
    return ans-1;
}

int num_hosts = 0;
HOST * all_hosts = NULL;

void release_hosts(void) {
    int i;
    for(i=0;i<num_hosts;i++)
        kfree(all_hosts[i]);
    kfree(all_hosts);
}

int match_hosts(HOST h1, HOST h2) {
    return !strcmp(h1, h2);
}

int add_host(HOST new_host) {
    int i=0;
    for(i=0;i<num_hosts;i++) {
        if(match_hosts(all_hosts[i], new_host))
            return i;
    }

    all_hosts = (HOST *)krealloc(all_hosts, (num_hosts+1) * sizeof(HOST), GFP_KERNEL);
    all_hosts[num_hosts] = (HOST)kmalloc(MAX_HOST_SIZE * sizeof(char), GFP_KERNEL);
    strcpy(all_hosts[num_hosts], new_host);

    return num_hosts++;
}

int get_host_index(HOST new_host) {
    int i=0;
    for(i=0;i<num_hosts;i++) {
        if(match_hosts(all_hosts[i], new_host))
            return i;
    }

    return -1;
}

int num_user_ids = 0;
USER_ID * all_user_ids = NULL;

void release_user_ids(void) {
    kfree(all_user_ids);
}

int match_user_ids(USER_ID u1, USER_ID u2) {
    return ((u1.host_id_index == u2.host_id_index) && (u1.uid == u2.uid));
}

int add_user_id(USER_ID new_user_id) {
    int i;
    for(i=0;i<num_user_ids;i++) {
        if(match_user_ids(all_user_ids[i], new_user_id))
            return i;
    }

    all_user_ids = (USER_ID *)krealloc(all_user_ids, (num_user_ids+1) * sizeof(USER_ID), GFP_KERNEL);
    all_user_ids[num_user_ids] = new_user_id;

    return num_user_ids++;
}

int get_user_id_index(USER_ID user_id) {
    int i;
    for(i=0;i<num_user_ids;i++) {
        if(match_user_ids(all_user_ids[i], user_id))
            return i;
    }

    return -1;
}

int get_number_of_users(void) {
    return num_user_ids;
}

int num_group_ids = 0;
GROUP_ID * all_group_ids = NULL;

void release_group_ids(void) {
    kfree(all_group_ids);
}

int match_group_ids(GROUP_ID u1, GROUP_ID u2) {
    return (u1.host_id_index == u2.host_id_index) && (u1.gid == u2.gid);
}

int add_group_id(GROUP_ID new_group_id) {
    int i;
    for(i=0;i<num_group_ids;i++) {
        if(match_group_ids(all_group_ids[i], new_group_id))
            return i;
    }

    all_group_ids = (GROUP_ID *)krealloc(all_group_ids, (num_group_ids+1) * sizeof(GROUP_ID), GFP_KERNEL);
    all_group_ids[num_group_ids] = new_group_id;

    return num_group_ids++;
}

int get_group_id_index(GROUP_ID group_id) {
    int i;
    for(i=0;i<num_group_ids;i++) {
        if(match_group_ids(all_group_ids[i], group_id))
            return i;
    }

    return -1;
}

USER_SET get_members_from_group_id(int group_id_index) {
    return all_group_ids[group_id_index].members;
}

int num_object_ids = 0;
OBJECT_ID * all_object_ids = NULL;

void release_object_ids(void) {
    kfree(all_object_ids);
}

int match_object_ids(OBJECT_ID obj_id1, OBJECT_ID obj_id2) {
    return obj_id1.host_id_index == obj_id2.host_id_index
            && obj_id1.device_id == obj_id2.device_id
            && obj_id1.inode_number == obj_id2.inode_number;
}

int add_object_id(OBJECT_ID new_object_id) {
    int i;
    for(i=0;i<num_object_ids;i++) {
        if(match_object_ids(all_object_ids[i], new_object_id))
            return i;
    }

    all_object_ids = (OBJECT_ID *)krealloc(all_object_ids, (num_object_ids+1) * sizeof(OBJECT_ID), GFP_KERNEL);
    all_object_ids[num_object_ids] = new_object_id;

    return num_object_ids++;
}

int get_object_id_index(OBJECT_ID object_id) {
    int i;
    for(i=0;i<num_object_ids;i++) {
        if(match_object_ids(all_object_ids[i], object_id))
            return i;
    }

    return -1;
}

int num_objects = 0;
OBJECT * all_objects = NULL;

void release_objects(void) {
    kfree(all_objects);
}

int match_objects(OBJECT obj1, OBJECT obj2) {
    return obj1.obj_id_index == obj2.obj_id_index;
}

int add_object(OBJECT new_object) {
    int i;
    for(i=0;i<num_objects;i++) {
        if(match_objects(all_objects[i], new_object))
            return i;
    }

    all_objects = (OBJECT *)krealloc(all_objects, (num_objects+1) * sizeof(OBJECT), GFP_KERNEL);
    all_objects[num_objects] = new_object;

    return num_objects++;
}

int get_object_from_obj_id_index(int obj_id_index) {
    int i;
    for(i=0;i<num_objects;i++) {
        if(all_objects[i].obj_id_index == obj_id_index)
            return i;
    }

    return -1;
}

OBJECT get_object(int obj_id) {
    return all_objects[get_object_from_obj_id_index(obj_id)];
}

int update_object_label(int obj_id_index, USER_SET readers, USER_SET writers) {
    int obj_index = get_object_from_obj_id_index(obj_id_index);
    all_objects[obj_index].readers = readers;
    all_objects[obj_index].writers = writers;

    return 0;
}

int num_subject_ids = 0;
SUBJECT_ID * all_subject_ids = NULL;

void release_subject_ids(void) {
    kfree(all_subject_ids);
}

int match_subject_ids(SUBJECT_ID sub_id1, SUBJECT_ID sub_id2) {
    return sub_id1.host_id_index == sub_id2.host_id_index
            && sub_id1.uid == sub_id2.uid
            && sub_id1.pid == sub_id2.pid;
}

int add_subject_id(SUBJECT_ID new_subject_id) {
    int i;
    for(i=0;i<num_subject_ids;i++) {
        if(match_subject_ids(all_subject_ids[i], new_subject_id))
            return i;
    }

    all_subject_ids = (SUBJECT_ID *)krealloc(all_subject_ids, (num_subject_ids+1) * sizeof(SUBJECT_ID), GFP_KERNEL);
    all_subject_ids[num_subject_ids] = new_subject_id;

    return num_subject_ids++;
}

int get_subject_id_index(SUBJECT_ID subject_id) {
    int i;
    for(i=0;i<num_subject_ids;i++) {
        if(match_subject_ids(all_subject_ids[i], subject_id))
            return i;
    }

    return -1;
}

int num_subjects = 0;
SUBJECT * all_subjects = NULL;

void release_subjects(void) {
    kfree(all_subjects);
}

int match_subjects(SUBJECT sub1, SUBJECT sub2) {
    return sub1.sub_id_index == sub2.sub_id_index;
}

int add_subject(SUBJECT new_subject) {
    int i;
    for(i=0;i<num_subjects;i++) {
        if(match_subjects(all_subjects[i], new_subject))
            return i;
    }

    all_subjects = (SUBJECT *)krealloc(all_subjects, (num_subjects+1) * sizeof(SUBJECT), GFP_KERNEL);
    all_subjects[num_subjects] = new_subject;

    return num_subjects++;
}

int get_subject_from_sub_id_index(int sub_id_index) {
    int i;
    for(i=0;i<num_subjects;i++) {
        if(all_subjects[i].sub_id_index == sub_id_index)
            return i;
    }

    return -1;
}

SUBJECT get_subject(int sub_id) {
    return all_subjects[get_subject_from_sub_id_index(sub_id)];
}

int update_subject_label(int sub_id_index, USER_SET readers, USER_SET writers) {
    int sub_index = get_subject_from_sub_id_index(sub_id_index);
    all_subjects[sub_index].readers = readers;
    all_subjects[sub_index].writers = writers;

    return 1;
}

int infer_file_labels(OBJECT * object, struct kstat * object_info, int host_id_index) {
    USER_ID cur_user_id;
    cur_user_id.host_id_index = host_id_index;
    cur_user_id.uid = object_info->uid.val;
    int user_id_index = get_user_id_index(cur_user_id);
	object->readers = 0;
	object->writers = 0;

	int is_user_reader = object_info->mode & S_IRUSR ? 1 : 0;
    int is_user_writer = object_info->mode & S_IWUSR ? 1 : 0;

    if(user_id_index==-1)
        return -1;
    
    if(is_user_reader)
        add_user_to_label(user_id_index, &(object->readers));

    if(is_user_writer)
        add_user_to_label(user_id_index, &(object->writers));

    GROUP_ID cur_grp_id;
    cur_grp_id.host_id_index = host_id_index;
    cur_grp_id.gid = object_info->gid.val;
    int group_id_index = get_group_id_index(cur_grp_id);    
    USER_SET group_members = get_members_from_group_id(group_id_index);
    
    if(object_info->mode & S_IRGRP)
        object->readers |= group_members;

    if(object_info->mode & S_IWGRP)
        object->writers |= group_members; 

    if(!is_user_reader)
        remove_user_from_set(user_id_index, &(object->readers));
    
    if(!is_user_writer)
        remove_user_from_set(user_id_index, &(object->writers));
    
    int number_of_users = get_number_of_users();
    USER_SET all_users = get_all_users(number_of_users);

    USER_SET other_members = all_users & (~group_members);
    remove_user_from_set(user_id_index, &other_members);
    
    if(object_info->mode & S_IROTH)
        object->readers |= other_members;

    if(object_info->mode & S_IWOTH)
        object->writers |= other_members;

    return 0;       
}

int add_all_hosts(void) {
    return add_host("parjanya-VirtualBox");
}

void add_all_users(int host_id) {
    USER_ID user_to_add;
    user_to_add.host_id_index = host_id;

    user_to_add.uid = 0;
    add_user_id(user_to_add);

    user_to_add.uid = 1;
    add_user_id(user_to_add);

    user_to_add.uid = 2;
    add_user_id(user_to_add);

    user_to_add.uid = 3;
    add_user_id(user_to_add);

    user_to_add.uid = 4;
    add_user_id(user_to_add);

    user_to_add.uid = 5;
    add_user_id(user_to_add);

    user_to_add.uid = 6;
    add_user_id(user_to_add);

    user_to_add.uid = 7;
    add_user_id(user_to_add);

    user_to_add.uid = 8;
    add_user_id(user_to_add);

    user_to_add.uid = 9;
    add_user_id(user_to_add);

    user_to_add.uid = 10;
    add_user_id(user_to_add);

    user_to_add.uid = 13;
    add_user_id(user_to_add);

    user_to_add.uid = 33;
    add_user_id(user_to_add);

    user_to_add.uid = 34;
    add_user_id(user_to_add);

    user_to_add.uid = 38;
    add_user_id(user_to_add);

    user_to_add.uid = 39;
    add_user_id(user_to_add);

    user_to_add.uid = 41;
    add_user_id(user_to_add);

    user_to_add.uid = 65534;
    add_user_id(user_to_add);

    user_to_add.uid = 100;
    add_user_id(user_to_add);

    user_to_add.uid = 101;
    add_user_id(user_to_add);

    user_to_add.uid = 102;
    add_user_id(user_to_add);

    user_to_add.uid = 103;
    add_user_id(user_to_add);

    user_to_add.uid = 104;
    add_user_id(user_to_add);

    user_to_add.uid = 105;
    add_user_id(user_to_add);

    user_to_add.uid = 106;
    add_user_id(user_to_add);

    user_to_add.uid = 107;
    add_user_id(user_to_add);

    user_to_add.uid = 108;
    add_user_id(user_to_add);

    user_to_add.uid = 109;
    add_user_id(user_to_add);

    user_to_add.uid = 110;
    add_user_id(user_to_add);

    user_to_add.uid = 111;
    add_user_id(user_to_add);

    user_to_add.uid = 112;
    add_user_id(user_to_add);

    user_to_add.uid = 113;
    add_user_id(user_to_add);

    user_to_add.uid = 114;
    add_user_id(user_to_add);

    user_to_add.uid = 115;
    add_user_id(user_to_add);

    user_to_add.uid = 116;
    add_user_id(user_to_add);

    user_to_add.uid = 117;
    add_user_id(user_to_add);

    user_to_add.uid = 118;
    add_user_id(user_to_add);

    user_to_add.uid = 119;
    add_user_id(user_to_add);

    user_to_add.uid = 120;
    add_user_id(user_to_add);

    user_to_add.uid = 1000;
    add_user_id(user_to_add);

    user_to_add.uid = 1001;
    add_user_id(user_to_add);
}

void add_all_groups(int host_id) {
    GROUP_ID group_to_add;
    USER_ID uid_to_find;
    group_to_add.host_id_index = host_id;
    uid_to_find.host_id_index = host_id;

    group_to_add.gid = 0;
    group_to_add.members = 0;
	uid_to_find.uid = 0;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 1;
    group_to_add.members = 0;
	uid_to_find.uid = 1;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 10;
    group_to_add.members = 0;
	uid_to_find.uid = 10;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 1000;
    group_to_add.members = 0;
	uid_to_find.uid = 1000;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 1001;
    group_to_add.members = 0;
	uid_to_find.uid = 1001;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 102;
    group_to_add.members = 0;
	uid_to_find.uid = 100;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 103;
    group_to_add.members = 0;
	uid_to_find.uid = 101;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 104;
    group_to_add.members = 0;
	uid_to_find.uid = 102;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 105;
    group_to_add.members = 0;
	uid_to_find.uid = 103;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 108;
    group_to_add.members = 0;
	uid_to_find.uid = 104;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 110;
    group_to_add.members = 0;
	uid_to_find.uid = 106;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 111;
    group_to_add.members = 0;
	uid_to_find.uid = 107;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 113;
    group_to_add.members = 0;
	uid_to_find.uid = 1000;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 114;
    group_to_add.members = 0;
	uid_to_find.uid = 108;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 116;
    group_to_add.members = 0;
	uid_to_find.uid = 109;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 119;
    group_to_add.members = 0;
	uid_to_find.uid = 110;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 12;
    group_to_add.members = 0;
	uid_to_find.uid = 6;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 120;
    group_to_add.members = 0;
	uid_to_find.uid = 111;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 122;
    group_to_add.members = 0;
	uid_to_find.uid = 119;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 123;
    group_to_add.members = 0;
	uid_to_find.uid = 113;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 124;
    group_to_add.members = 0;
	uid_to_find.uid = 117;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 126;
    group_to_add.members = 0;
	uid_to_find.uid = 118;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 127;
    group_to_add.members = 0;
	uid_to_find.uid = 119;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 128;
    group_to_add.members = 0;
	uid_to_find.uid = 1000;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 13;
    group_to_add.members = 0;
	uid_to_find.uid = 13;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 2;
    group_to_add.members = 0;
	uid_to_find.uid = 2;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 24;
    group_to_add.members = 0;
	uid_to_find.uid = 1000;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 27;
    group_to_add.members = 0;
	uid_to_find.uid = 1000;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 29;
    group_to_add.members = 0;
	uid_to_find.uid = 114;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
	uid_to_find.uid = 117;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 3;
    group_to_add.members = 0;
	uid_to_find.uid = 3;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 30;
    group_to_add.members = 0;
	uid_to_find.uid = 1000;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 33;
    group_to_add.members = 0;
	uid_to_find.uid = 33;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 34;
    group_to_add.members = 0;
	uid_to_find.uid = 34;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 38;
    group_to_add.members = 0;
	uid_to_find.uid = 38;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 39;
    group_to_add.members = 0;
	uid_to_find.uid = 39;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 4;
    group_to_add.members = 0;
	uid_to_find.uid = 1000;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
	uid_to_find.uid = 104;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 41;
    group_to_add.members = 0;
	uid_to_find.uid = 41;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 46;
    group_to_add.members = 0;
	uid_to_find.uid = 1000;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
	uid_to_find.uid = 120;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 60;
    group_to_add.members = 0;
	uid_to_find.uid = 5;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 65534;
    group_to_add.members = 0;
	uid_to_find.uid = 105;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
	uid_to_find.uid = 112;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
	uid_to_find.uid = 116;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
	uid_to_find.uid = 4;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
	uid_to_find.uid = 65534;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 7;
    group_to_add.members = 0;
	uid_to_find.uid = 7;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
	uid_to_find.uid = 115;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 8;
    group_to_add.members = 0;
	uid_to_find.uid = 8;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);

    group_to_add.gid = 9;
    group_to_add.members = 0;
	uid_to_find.uid = 9;
    add_user_to_label(get_user_id_index(uid_to_find), &group_to_add.members);
    add_group_id(group_to_add);
}

void initialize(void) {
    int host_id = add_all_hosts();
    add_all_users(host_id);
    add_all_groups(host_id);
}

void clone_check(char* host_name, int uid, int child_pid, int parent_pid) {
    int host_id_index = get_host_index(host_name);
    SUBJECT_ID parent_sub_id, child_sub_id;
    parent_sub_id.host_id_index = host_id_index;
    parent_sub_id.uid = uid;
    parent_sub_id.pid = parent_pid;
    child_sub_id.host_id_index = host_id_index;
    child_sub_id.uid = uid;
    child_sub_id.pid = child_pid;
    int parent_sub_id_index = get_subject_id_index(parent_sub_id);
    int child_sub_id_index = add_subject_id(child_sub_id);
    if(parent_sub_id_index == -1) {
        USER_ID owner_id;
        owner_id.host_id_index = host_id_index;
        owner_id.uid = uid;
        int owner = get_user_id_index(owner_id);
        USER_SET readers = get_all_users(get_number_of_users());
        USER_SET writers = 0;
        add_user_to_label(owner, &writers);
        SUBJECT child_subject;
        child_subject.sub_id_index = child_sub_id_index;
        child_subject.owner = owner;
        child_subject.readers = readers;
        child_subject.writers = writers;
        add_subject(child_subject);
    }
    else {
        SUBJECT parent_subject = get_subject(parent_sub_id_index);
        SUBJECT child_subject;
        child_subject.sub_id_index = child_sub_id_index;
        child_subject.owner = parent_subject.owner;
        child_subject.readers = parent_subject.readers;
        child_subject.writers = parent_subject.writers;
        add_subject(child_subject);
    }
}

int file_open_check(char * host_name, struct kstat * file_info){
    int host_id_index = get_host_index(host_name);
    OBJECT_ID obj_id;
    obj_id.host_id_index = host_id_index;
    obj_id.device_id = file_info->dev;
    obj_id.inode_number = file_info->ino;
    int object_id_index = get_object_id_index(obj_id);
    
	if(object_id_index == -1) {
        object_id_index = add_object_id(obj_id);
   
        OBJECT object;
        object.obj_id_index = object_id_index;
        USER_ID cur_user;
        cur_user.host_id_index = host_id_index;
        cur_user.uid = file_info->uid.val;
        object.owner = get_user_id_index(cur_user);
        infer_file_labels(&object, file_info, host_id_index);
        
        add_object(object);
    }

    return 1;
}

int file_read_check(char * host_name, int uid, int pid, int fd) {
	struct kstat file_info;
	if(vfs_fstat(fd, &file_info) != 0)
        return -1;

    int host_id_index = get_host_index(host_name);
    SUBJECT_ID cur_sub_id;
    cur_sub_id.host_id_index = host_id_index;
    cur_sub_id.uid = uid;
    cur_sub_id.pid = pid;
    int sub_id_index = get_subject_id_index(cur_sub_id);
    OBJECT_ID cur_obj_id;
    cur_obj_id.host_id_index = host_id_index;
    cur_obj_id.device_id = file_info.dev;
    cur_obj_id.inode_number = file_info.ino;
    int obj_id_index = get_object_id_index(cur_obj_id);
	int ret_val = 0;

    //If fd is not a file fd
    if(sub_id_index == -1 || obj_id_index == -1)
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

    return ret_val;
}

int file_write_check(char * host_name, int uid, int pid, int fd) {
	struct kstat file_info;
	if(vfs_fstat(fd, &file_info) != 0)
        return -1;
    int host_id_index = get_host_index(host_name);
    SUBJECT_ID cur_sub_id;
    cur_sub_id.host_id_index = host_id_index;
    cur_sub_id.uid = uid;
    cur_sub_id.pid = pid;
    int sub_id_index = get_subject_id_index(cur_sub_id);
    OBJECT_ID cur_obj_id;
    cur_obj_id.host_id_index = host_id_index;
    cur_obj_id.device_id = file_info.dev;
    cur_obj_id.inode_number = file_info.ino;
    int obj_id_index = get_object_id_index(cur_obj_id);
	int ret_val = 0;

    //If fd is not a file fd
    if(sub_id_index == -1 || obj_id_index == -1)
        ret_val = -1;
	else {
		SUBJECT subject = get_subject(sub_id_index);
		OBJECT object = get_object(obj_id_index);

		if(is_user_in_set(subject.owner, &object.writers)
		    && is_superset_of(&subject.readers, &object.readers)
		    && is_subset_of(&subject.writers, &object.writers))
		    ret_val = 1;
	}

    return ret_val;
}

void release_memory(void) {
    release_hosts();
    release_user_ids();
    release_group_ids();
    release_object_ids();
    release_objects();
    release_subject_ids();
    release_subjects();
}

int getuid(void) {
    return __kuid_val(current_uid());
}

pid_t getpid(void) {
    return task_pid_nr(current);
}

static void disable_page_protection(void) 
{
  unsigned long value;
  asm volatile("mov %%cr0, %0" : "=r" (value));

  if(!(value & 0x00010000))
    return;

  asm volatile("mov %0, %%cr0" : : "r" (value & ~0x00010000));
}

static void enable_page_protection(void) 
{
  unsigned long value;
  asm volatile("mov %%cr0, %0" : "=r" (value));

  if((value & 0x00010000))
    return;

  asm volatile("mov %0, %%cr0" : : "r" (value | 0x00010000));
}

static unsigned long **syscall_table;

asmlinkage pid_t (*original_clone)(unsigned long, unsigned long, void* __user, void* __user, struct pt_regs*);
asmlinkage long (*original_open)(const char __user *filename, int flags, umode_t mode);
asmlinkage size_t (*original_read)(unsigned int fd, char __user *buff, size_t count);
asmlinkage size_t (*original_write)(unsigned int fd, const char __user *buff, size_t count);

asmlinkage pid_t rwfm_clone(unsigned long flags, unsigned long newsp, void* __user parent_tid, void* __user child_tid, struct pt_regs *regs)
{
    pid_t pid = original_clone(flags, newsp, parent_tid, child_tid, regs);
    if(pid != -1)
        clone_check("parjanya-VirtualBox", getuid(), pid, getpid());

    return pid;
}

asmlinkage long rwfm_open(const char __user *filename, int flags, umode_t mode)
{
    long fd = original_open(filename, flags, mode);
    struct kstat file_info;

	if(fd >= 0 && vfs_fstat(fd, &file_info) == 0 && S_ISREG(file_info.mode))
        file_open_check("parjanya-VirtualBox", &file_info);

    return fd;
}

asmlinkage size_t rwfm_read(unsigned int fd, char __user *buff, size_t count)
{
    struct kstat file_info;

    if(vfs_fstat(fd, &file_info) == 0 && S_ISREG(file_info.mode) && file_read_check("parjanya-VirtualBox", getuid(), getpid(), fd) != 1)
        pr_warning("READ NOT ALLOWED!\n");

    return original_read(fd, buff, count);
}

asmlinkage size_t rwfm_write(unsigned int fd, const char __user *buff, size_t count)
{
    struct kstat file_info;

    if(vfs_fstat(fd, &file_info) == 0 && S_ISREG(file_info.mode) && file_write_check("parjanya-VirtualBox", getuid(), getpid(), fd) != 1)
        pr_warning("WRITE NOT ALLOWED!\n");

    return original_write(fd, buff, count);
}

static void replace_syscalls(void)
{
    pr_info("replacing system calls\n");

    disable_page_protection();
    original_clone = (void *)syscall_table[__NR_clone];
    original_open = (void *)syscall_table[__NR_open];
    original_read = (void *)syscall_table[__NR_read];
    original_write = (void *)syscall_table[__NR_write];
//    getuid_call = (void *)sys_call_table[__NR_getuid];

    syscall_table[__NR_clone] = (unsigned long *)rwfm_clone;
    syscall_table[__NR_open] = (unsigned long *)rwfm_open;
    syscall_table[__NR_read]  = (unsigned long *)rwfm_read;
    syscall_table[__NR_write] = (unsigned long *)rwfm_write;

    enable_page_protection();

    pr_info("system calls replaced\n");
}

static void restore_syscalls(void)
{
    pr_info("trying to restore system calls\n");
    disable_page_protection();

    /* make sure no other modules have made changes before restoring */
    if(syscall_table[__NR_clone] == (unsigned long *)rwfm_clone)
    {
            syscall_table[__NR_clone] = (unsigned long *)original_clone;
            pr_info("restored sys_clone\n");
    }
    else
    {
            printk(KERN_WARNING "sys_clone not restored - address mismatch\n");
    }

    if(syscall_table[__NR_open] == (unsigned long *)rwfm_open)
    {
            syscall_table[__NR_open] = (unsigned long *)original_open;
            pr_info("restored sys_open\n");
    }
    else
    {
            printk(KERN_WARNING "sys_open not restored - address mismatch\n");
    }

    if(syscall_table[__NR_read] == (unsigned long *)rwfm_read)
    {
            syscall_table[__NR_read] = (unsigned long *)original_read;
            pr_info("restored sys_read\n");
    }
    else
    {
            printk(KERN_WARNING "sys_read not restored - address mismatch\n");
    }

    if(syscall_table[__NR_write] == (unsigned long *)rwfm_write)
    {
            syscall_table[__NR_write] = (unsigned long *)original_write;
            pr_info("restored sys_write\n");
    }
    else
    {
            printk(KERN_WARNING "sys_write not restored - address mismatch\n");
    }

    enable_page_protection();
    pr_info("system call restoration complete\n");
}

static int __init syscall_interceptor_start(void) 
{
    pr_info("Initializing system call interceptor\n");
    syscall_table = (void *) kallsyms_lookup_name("sys_call_table");
    replace_syscalls();
    initialize();

    return 0;
} 
  
static void __exit syscall_interceptor_end(void) 
{
    restore_syscalls();
    pr_info("Removing system call interceptor\n");
    release_memory();
} 

module_init(syscall_interceptor_start);
module_exit(syscall_interceptor_end);
