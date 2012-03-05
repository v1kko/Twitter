/*****************************************************************************
	Author: Victor Azizi
	Studentnumber : 6277861
	E-mail: veterazizi@hotmail.com
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <syslog.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "queue.h"
#include "daemon.h"
#define BUFFER_SIZE 200

extern char magiccode[2];
struct sockaddr_in6 clientconn;
char buffer[BUFFER_SIZE];
int  ccsd; 
socklen_t clientconnlen;
char fail = 0, pass = 1;
unsigned int fromlen;
shead hp1, hp2;

void
daemonclient() {
	//Name must be shared
	int shmid;
    if ((shmid = shmget(key, 24, 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((name = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }
	namever = (int *)&name[20];
	*namever = 0;
	name[0]='\0';
	
	hp1.head = NULL;
	hp2.head = NULL;

	
	clientconnlen = sizeof (clientconn);
	listen(csd, 20);
	while(1) {
		while ( 0 > (ccsd  = accept (csd, (struct sockaddr *)&clientconn, &clientconnlen)));
		while (recv(ccsd, buffer, sizeof(char)*BUFFER_SIZE, 0) < 0 );
		if (!strcmp(buffer, password)) {
			switch(buffer[strlen(password)+1]) {
			case 1:
				readmessage();
				break;
			case 2:
				twitregister(&buffer[strlen(password)+2]);
				break;
			case 3:
                sendmessage(&buffer[strlen(password)+2]);
				break;
			case 4:
				queuemessage(&buffer[strlen(password)+2]);
            default:
               	send(ccsd, &fail, 1, 0);
				break;
			}
		} else {
		send(ccsd, &fail, 1, 0);
		}
		close(ccsd);
	}
}

void
sendmessage(char * m) {
	char mbuffer[144]= {0};	 
	mbuffer[0] = magiccode[0]; mbuffer[1] = magiccode[1];
	if ( strlen(m) > 140 || name[0] == '\0') {
		while (1 != send(ccsd, &fail, 1, 0));
		return;	
	}
	strcpy(&mbuffer[3], m);
	int t, s = strlen(m)+4;
	char * n = mbuffer;
	pthread_mutex_lock(&sdmutex);
	while ( s > (t = sendto(ssd, n, s, 0, (struct sockaddr *)&sa_6, sizeof(struct sockaddr_in6)))) {
		if (t == -1) {
			while (1 != send(ccsd, &fail, 1, 0));
			return;	
		}
		s -= t;
		n = &n[t];
	}
	pthread_mutex_unlock(&sdmutex);
	while (1 != send(ccsd, &pass, 1, 0));
	return;	
}

void
readmessage() {
	char * m;
	m = dequeuemessage();
	if ( m == NULL ) {
		while (1 != send(ccsd, &fail, 1, 0));
		return;
	} else {
		send(ccsd, m, strlen(&m[1]) + 2, 0);
		free(m);
	}
	return;
}

void
twitregister (char * nick)
{
	if (strlen(nick) > 15) {
		while (1 != send(ccsd, &fail, 1, 0));
		return;
	}
	pthread_mutex_lock(&namemutex);
	*namever = 1;
	strcpy(name, nick);	
	pthread_mutex_unlock(&namemutex);
	char ping[4] = {magiccode[0], magiccode[1], 0, PING};
	pthread_mutex_lock(&sdmutex);
	if (sendto(ssd,  ping, sizeof(char)*4, 0, (struct sockaddr *)&sa_6, sizeof (struct sockaddr_in6)) == -1) {
		printf("%d\n",errno);
	}
	pthread_mutex_unlock(&sdmutex);
	sleep(2);
	if (*namever) { 
		while (1 != send(ccsd, &pass, 1, 0)) ;
		*namever = 0;
	} else {
		name[0] = '\0';
		while (1 != send(ccsd, &fail, 1, 0));
	}
	return;
}

void
queuemessage( char * m ) {
	message * me = malloc(sizeof(message));
	me->inhoud = malloc(sizeof(char)*(strlen(m)+1));
	strcpy(me->inhoud, m);
	me->next = hp1.head;
	hp1.head = me;
}

char *
dequeuemessage() {
	message * me;
	if (hp1.head == NULL && hp2.head == NULL) return NULL;
	if (hp2.head == NULL) {
		while ((me=hp1.head)!=NULL) {
			hp1.head = me->next;
			me->next = hp2.head;
			hp2.head = me;
		}
	}
	char * in = hp2.head->inhoud;
	me = hp2.head;
	hp2.head = hp2.head->next;
	free(me);
	return in;
}

