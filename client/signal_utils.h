#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include "underlying_libc_functions.h"

int get_uid_from_pid(int pid) {
    char path[1024],uid[10],pid_char[10];
    char *line = NULL;
    size_t len=0, read=0;

    for(int i=0;i<10;i++)
        pid_char[i]='\0';

    sprintf(pid_char, "%d", pid);

    strcpy(path, "/proc/");
    strcat(path, pid_char);
    strcat(path, "/status");
    FILE *fp = underlying_fopen(path,"r");

    if(!fp)
        return -1;

    while((read = getline(&line, &len, fp)) != -1)
    {
        if(line[0]=='U' && line[1]=='i' && line[2]=='d' && line[3]==':')
        {
            char c = line[5];
            int count = 0;
            while(c>='0'&&c<='9')
            {
                uid[count++] = c;
                c = line[5+count];
            }
            return atoi(uid);
        }
    }
    return -1;
}

int filter_processes(const struct dirent *cur_dir)
{
    if(cur_dir->d_type != DT_DIR)
        return 0;
    for(int i=0; i<strlen(cur_dir->d_name); i++)
    {
        if(!isdigit(cur_dir->d_name[i]))
            return 0;
    }

    return 1;
}

int get_all_permitted_pids_from_pid(int pid, char ***pids)
{
    int count=0;
    *pids = (char **)malloc(1024 * sizeof(char *));
    for(int i=0;i<1024;i++)
        (*pids)[i] = (char *)malloc(10 * sizeof(char));

    int cur_uid = get_uid_from_pid(pid);

    struct dirent **namelist;
    int n = scandir("/proc", &namelist, &filter_processes, alphasort);

    if(n<0)
        printf("Error!");
    else
    {
        while(n--)
        {
            if((cur_uid == 0 || get_uid_from_pid(atoi(namelist[n]->d_name)) == cur_uid) && pid != atoi(namelist[n]->d_name))
                strcpy((*pids)[count++], namelist[n]->d_name);

            free(namelist[n]);
        }

        free(namelist);
    }

    return count;
}

int get_group_pids_from_pid(int pid, char ***pids)
{
    int count=0;
    *pids = (char **)malloc(1024 * sizeof(char *));
    for(int i=0;i<1024;i++)
        (*pids)[i] = (char *)malloc(10 * sizeof(char));

    struct dirent **namelist;
    int n = scandir("/proc", &namelist, &filter_processes, alphasort);

    int pgid = (pid>=0 ? getpgid(pid) : (-pid));

    if(n<0)
        printf("Error!");
    else
    {
        while(n--)
        {
            if((getpgid(atoi(namelist[n]->d_name)) == pgid) && (atoi(namelist[n]->d_name) != pid))
                strcpy((*pids)[count++], namelist[n]->d_name);

            free(namelist[n]);
        }

        free(namelist);
    }

    return count;
}
