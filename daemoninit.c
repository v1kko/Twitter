/*****************************************************************************
	Author: Victor Azizi
	Studentnumber : 6277861
	E-mail: veterazizi@hotmail.com
*****************************************************************************/
 /*
 * Author : Victor Azizi
 * 
 * Some of the daemon initialization code (skeleton) is written by :
 * Peter Lombardo (peter AT lombardo DOT info) in  5/1/2006
 *
 */
 
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <syslog.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

#include "daemon.h"

int yes=1;
 
void PrintUsage(int argc, char *argv[]) {
	if (argc >=1) {
		printf("Usage: %s -h -nn", argv[0]);
		printf("  Options:n");
		printf("	  -ntDon't fork off as a daemon.n");
		printf("	  -htShow this help screen.n");
		printf("n");
	}
}

int main(int argc, char *argv[]) {

	int daemonize = 1;
 
	// Setup signal handling before we start
	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
 
	//argument checking should happen here
	if (argc == 2) {
		if(argv[1][0] == 'n') daemonize = 0;
 	}

	//Set a password for the connecting client
	fprintf(stdout, "Set password (max 19 characters):");
	fscanf(stdin, "%s", password);

	syslog(LOG_INFO, "%s daemon starting up", DAEMON_NAME);
 
	// Setup syslog logging - see SETLOGMASK(3)
	setlogmask(LOG_UPTO(LOG_INFO));
	openlog(DAEMON_NAME, LOG_CONS, LOG_USER);

 
	/* Our process ID and Session ID */
	pid_t pid, sid;
 
	if (daemonize) {
		syslog(LOG_INFO, "starting the daemonizing process");
 
		/* Fork off the parent process */
		pid = fork();
		if (pid < 0) exit(EXIT_FAILURE);
		
		if (pid > 0) exit(EXIT_SUCCESS);
 	
		/* Change the file mode mask */
		umask(0);
 
		/* Create a new SID for the child process */
		sid = setsid();
		if (sid < 0) exit(EXIT_FAILURE);
 
		/* Change the current working directory */
		if ((chdir("/")) < 0) exit(EXIT_FAILURE);
	}

	//Init mutexen
	pthread_mutex_init(&namemutex, NULL);
	pthread_mutex_init(&sdmutex, NULL);

	//Create shared memory
	int shmid;
    key = 7332;
	
    if ((shmid = shmget(key, 24, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
        exit(1);
    }
		
	//Create a listening socket to the internet

	sa_6.sin6_family = AF_INET6;
	sa_6.sin6_scope_id = 2;
	sa_6.sin6_port = htons(12242);
	if (-1 == inet_pton(AF_INET6, "ff02::1", &sa_6.sin6_addr)) perror("pton_error");
	
	if ( 0 > (sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)))
		exit(EXIT_FAILURE);

	 struct ipv6_mreq {
		 struct in6_addr ipv6mr_multiaddr;
	 	unsigned int    ipv6mr_interface;
	 };

	//Reuse addres (handy for ADDRINUSE error);
 	if (setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(int)) == -1) perror("reuse addres");

	if ( 0 > bind(sd, (struct sockaddr *)&sa_6, sizeof(struct sockaddr_in6))) {
		perror("Error (bind 12242)");
		exit(EXIT_FAILURE);
	}

	//Add sd to multicast group
	struct ipv6_mreq mreq6;
	if (-1 == inet_pton(AF_INET6, "ff02::1", &mreq6.ipv6mr_multiaddr)) perror("pton_error");
	mreq6.ipv6mr_interface = 0;
	if (setsockopt(sd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6)) == -1 ) {
		perror("Socket Option1"); 
		exit(1);
	}

	//Create a socket to the client
	sa_6.sin6_port = htons(12241);
	if (-1 == inet_pton(AF_INET6, "::1", &sa_6.sin6_addr)) perror("pton_error");
	
	if ( 0 > (csd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)))
		exit(EXIT_FAILURE);
	
	//Reuse addres (handy for ADDRINUSE error);
 	if (setsockopt(csd,SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(int)) == -1) perror("reuse addres");
	
	if ( 0 > bind(csd, (struct sockaddr *)&sa_6, sizeof (struct sockaddr_in6))) {
		perror("Error (bind 12241)");
		exit(EXIT_FAILURE);
	}
	
	//Create  a sending socket to the internet
	sa_6.sin6_port = htons(12242);
	if (-1 == inet_pton(AF_INET6, "ff02::1", &sa_6.sin6_addr)) perror("pton_error");
	if ( 0 > (ssd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)))
		exit(EXIT_FAILURE);

int loop = 1;
	if (setsockopt(ssd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loop, sizeof(loop))== -1) {
		perror("Socket Option2"); 
		exit(1);
	}
	
	if (daemonize) {
		/* Close out the standard file descriptors */
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}
	/*Create client and internet listener*/
	pid = fork();
	if (pid < 0) exit(EXIT_FAILURE);
	if (pid > 0) daemonclient();
	daemonmain();
}

void signal_handler(int sig) {
 
	switch(sig) {
		case SIGHUP:
			syslog(LOG_WARNING, "Received SIGHUP signal.");
			break;
		case SIGTERM:
			syslog(LOG_WARNING, "Received SIGTERM signal.");
			exit(0);
			break;
		default:
			syslog(LOG_WARNING, "Unhandled signal");
			exit(0);
			break;
	}
}
