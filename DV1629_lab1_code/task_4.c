#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h> 
#include <sys/wait.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <string.h>
#include <sys/msg.h>

#define PERMS 0644

struct my_msgbuf {
   long mtype;
   int mvalue;
};

void msgqrecv() {
   struct my_msgbuf buf;
   int msqid;
   int toend;
   key_t key;

   if ((key = ftok("msgq.txt", 'B')) == -1) {
      perror("ftok");
      exit(1);
   }
    printf("RECV");
   if ((msqid = msgget(key, PERMS)) == -1) { /* connect to the queue */
      perror("msgget");
      exit(1);
   }
   printf("message queue: ready to receive messages.\n");

   int counter = 0;
   for(;;) { /* normally receiving never ends but just to make conclusion */
             /* this program ends with string of end */
      if (msgrcv(msqid, &buf, sizeof(buf.mvalue), 0, 0) == -1) {
         printf("\nItems recieved: %d\n", counter);
         perror("msgrcv");
         exit(1);
      }
      toend = buf.mvalue;
      if (toend == -1)
        break;
      counter++;
      
      printf("recvd: \"%d\"\n", buf.mvalue);
   }
   printf("\nItems recieved: %d\n", counter);
   printf("message queue: done receiving messages.\n");
   system("rm msgq.txt");
}

void msgqsend() {
   struct my_msgbuf buf;
   int msqid;
   int len;
   key_t key;
   srand(time(NULL));
   system("touch msgq.txt");

   if ((key = ftok("msgq.txt", 'B')) == -1) {
      perror("ftok");
      exit(1);
   }
      printf("SEND");
   if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1) {

      perror("msgget");
      exit(1);
   }
   printf("message queue: ready to send messages.\n");
   printf("Enter lines of text, ^D to quit:\n");
   buf.mtype = 1; /* we don't really care in this case */
   
   int counter = 0;

   char _;
   scanf("%c",&_);
   for(int i = 0; i < 50; i++) {
        int res = rand() % 10;
        buf.mvalue = res;

        printf("%d\n", buf.mvalue);

        len = sizeof(res);

        if (msgsnd(msqid, &buf, sizeof(buf.mvalue), 0) == -1) /* +1 for '\0' */
            perror("msgsnd");
      counter++;
   }

   buf.mvalue = -1;
   len = sizeof(buf.mvalue);
   if (msgsnd(msqid, &buf, len, 0) == -1) /* +1 for '\0' */
      perror("msgsnd");
   printf("\nItems sent: %d\nInput anyting to continue: ", counter);

   // Can fix with semaphores. recv post semaphore when all is done and send can continue
   scanf("%c",&_);

   if (msgctl(msqid, IPC_RMID, NULL) == -1) {
      perror("msgctl");
      exit(1);
   }

   printf("message queue: done sending messages.\n");
}

int main() {
    pid_t pid;
    pid = fork();

    if(pid != 0) {
        msgqsend();
    } else {
        sleep(1);
        msgqrecv();
    }

    return 0;
}
