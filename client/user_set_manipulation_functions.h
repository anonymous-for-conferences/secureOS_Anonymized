/*
This file includes all the functions which performs all user set based functions using bit manipulation. Functions like union,intersection of user sets,removing/adding
a user from a user_set, checking whether a user is present in a user_set etc have been implemented here.
*/

#ifndef _USER_SET_MANIPULATION_FUN_H_
#define _USER_SET_MANIPULATION_FUN_H_

#include "database_model.h"

int add_user_to_label(int user_to_add, USER_SET * label) {
    unsigned long long int max = -1;
    if(*label == max)
        return -1;

    USER_SET tmp = 1;
    tmp <<= user_to_add;

    *label |= tmp;

    return 0;
}

int remove_user_from_set(int user_to_remove, USER_SET * set) {
    unsigned long long int max = -1;
    if(*set == max)
        return -1;

    USER_SET tmp = 1;
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

#endif
