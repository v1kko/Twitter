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
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "daemon.h"

void sendqueuemessage(char *);
/*structures for message handling*/
char magiccode[2] = {116, 119};
struct sockaddr_in6 client, dummy;
struct addrinfo *adr;
int smc;
	
//Create a socket to store messages
struct addrinfo ad;
	  
void
daemonmain()
{
	memset(&ad, 0, sizeof(ad));	
	ad.ai_family = AF_INET6;
	ad.ai_socktype = SOCK_STREAM;
	ad.ai_protocol = IPPROTO_TCP;
	if ( 0 > getaddrinfo(NULL, "12241", &ad, &adr)) exit(EXIT_FAILURE);
	
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
	
	char * m = NULL, * s;
	int type;
	//Main loop
	while (1) {	
		switch ((type = twitterlisten(&m)) ) {
		case 2:	  
			s = strchr (m, ' ');
			if (s != NULL) * s = '\0';
			if(strcmp(name, &m[1])) {
				free(m);
				break;
			}
			* s = ' ';
			sendqueuemessage (m);
			break;
		case 3:
		case 1:
			sendqueuemessage(m);
			free(m);
			break;
		case PONG:
			receivepong(&m[1]);
			free(m);
			break;
		case 4:
			sendpong();
			free(m);
			break;	
		default:
			free(m);
			break;
		}
	}
}

int
twitterlisten (char ** m) {
	unsigned int type, fromlen = sizeof(struct sockaddr_in6);
	char *s, *t = *m;
	if ( (t = malloc(sizeof(char)*145)) == NULL ) daemonfree(OUT_OF_MEMORY);
	memset(t, 0, 145);
	do {
		//Listen to multicast
		recvfrom(sd, t, sizeof(char)*144, 0, (struct sockaddr *)&dummy, &fromlen);
	}while ( !(t[0] == magiccode[0] && t[1] == magiccode[1]) ); //Not a valid message
	if ((type = t[3]) > 5) type = 0;
	if ( (s = malloc((strlen(&t[4])+2)*sizeof(char))) == NULL ) perror("malloc");
	s[0]=type;
	strcpy(&s[1], &t[4]);
	free(t);
	*m = s;
	return type;
}

void
receivepong(char * m) {
if (!*namever) return;
if (strcmp(name, m)) return;
pthread_mutex_lock(&namemutex);
*namever = 0;
pthread_mutex_unlock(&namemutex);
}

void
sendpong() {
	char pong[20]= {0}, *n;	 
	client.sin6_port = 12242;
	pong[0] = magiccode[0]; 
	pong[1] = magiccode[1];
	pong[3] = PONG;
	if (name[0] != '\0' && !*namever) {
		strcpy(&pong[4], name);
		int t, s = strlen(name)+5;
		n = pong;
		pthread_mutex_lock(&sdmutex);
		while (s > (t = sendto(sd, n, s, 0, (struct sockaddr*)&sa_6, sizeof(struct sockaddr_in6)))) {
			if (t == -1) break;
			s -= t;
			n = &n[t];
		}
		pthread_mutex_unlock(&sdmutex);
	}
}

void
daemonfree(int err)
{
	switch(err) {
	case OUT_OF_MEMORY:
		syslog(LOG_INFO, "No memory available for message, Trying to shutdown gracefully");
		break;
	default:
		break;
	}
	char * m;
	while ((m = dequeuemessage()) != NULL) free(m);
	close(sd);
	close(csd);
	close(ccsd);
	close(smc);
	freeaddrinfo(adr);
	syslog(LOG_INFO, "%s daemon exiting", DAEMON_NAME);
	exit(0);
}

void sendqueuemessage(char * m) {
	if ( 0 > (smc = socket(adr->ai_family, adr->ai_socktype, adr->ai_protocol)) )
		perror("sendqueuemsgfail");
	char buffer[200];
	strcpy(buffer, password);
	buffer[strlen(password)+1]=4;
	strcpy(&buffer[strlen(password)+2], m);
	if (connect(smc, adr->ai_addr, adr->ai_addrlen) == -1) {
		perror("msg queue connect fail");
	}
	send(smc, buffer,strlen(password)+strlen(m)+3, 0);
	close(smc);
}
