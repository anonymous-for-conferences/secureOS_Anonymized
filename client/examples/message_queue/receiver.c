#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msgbuff {
	long type;
	char msg_str[20];
};

int main() {
	key_t key = ftok("./tmp_file", 1);
	struct msgbuff msg;
	int msgqid = msgget(key, 0666 | IPC_CREAT);
	msgrcv(msgqid, &msg, sizeof(msg.msg_str), 0, 0);
	printf("Received msg of type: %ld\nContent: %s\n",msg.type, msg.msg_str);
	msgctl(msgqid, IPC_RMID, NULL);

	return 0;
}
