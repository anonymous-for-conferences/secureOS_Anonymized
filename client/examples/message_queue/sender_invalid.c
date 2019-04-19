#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msgbuff {
	long type;
	char msg_str[20];
};

int main() {
	int fd = open("secret_file", O_RDONLY);
	char buf[1024];
    if(fd<0){
        printf("\nFailed to open '1'");
        return 1;
    }else{
        int n = read(fd, buf, 1024);
		if(n<0) {
			printf("\nRead failed!\n");
			return 1;
		}
        buf[n]='\0';
        printf("%s\n", buf);
        close(fd);
    }
	key_t key = ftok("./tmp_file", 1);
	struct msgbuff msg;
	msg.type = 1;
	sprintf(msg.msg_str, "%s", buf);
	int msgqid = msgget(key, 0666 | IPC_CREAT);
	msgsnd(msgqid, &msg, sizeof(msg.msg_str), 0);

	return 0;
}
