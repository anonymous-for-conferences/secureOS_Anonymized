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
	msg.type = 1;
	sprintf(msg.msg_str, "I am awesome");
	int msgqid = msgget(key, 0666 | IPC_CREAT);
	msgsnd(msgqid, &msg, sizeof(msg.msg_str), 0);

	return 0;
}
