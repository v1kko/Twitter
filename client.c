/*****************************************************************************
	Author: Victor Azizi
	Studentnumber : 6277861
	E-mail: veterazizi@hotmail.com
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 200

#define SEND_MESSAGE 3
#define RECEIVE_MESSAGE 1
#define REGISTER_NICK 2

#define TOPIC_TWEET 3
#define TWEET 1
#define PRIVE_TWEET 2

char password[20], message[141], nick[16], buffer[BUFFER_SIZE], type;
struct addrinfo *adr;
int(dsd);

void sendmessage(void);
void receivemessage(void);
void registernick(void);

void
printhelp(char * argv){
	printf("\tUsage:\n");
	printf("%s [-p password] [Option] [Nick/Topic] [\"Message\"]\n\n", argv);
	printf("\tDO NOT FORGET TO PUT \"\" AROUND YOUR MESSAGE\n");
	printf("\tOptions:\n");
	printf("\t-r\tregister nick\n");
	printf("\t-m\tsend message\n");
	printf("\t-s\tprivate message\n");
	printf("\t-t\ttopic message\n");
	printf("\t-g\tget all messages\n");
	printf("\t-h\tprint this message\n\n");
	exit(0);
}

int
main(int argc, char * argv[]) {
	
    //Create a socket to the daemon
	struct addrinfo ad;
	memset(&ad, 0, sizeof(ad));	
	ad.ai_family = AF_INET6;
	ad.ai_socktype = SOCK_STREAM;
	ad.ai_protocol = IPPROTO_TCP;
	if ( 0 > getaddrinfo(NULL, "12241", &ad, &adr)) exit(EXIT_FAILURE);
	if ( 0 > (dsd = socket(adr->ai_family, adr->ai_socktype, adr->ai_protocol)) )
		exit(EXIT_FAILURE);
    
	//Read the arguments
	int i;
	if (argc == 1) printhelp(argv[0]);
	for (i = 1 ; i < argc ; i++ ) {
		if (argv[i][0]=='-') {
			switch (argv[i][1]) {
				case 'r':
					if (i+1 < argc) {
						i++;
						strcpy(message, argv[i]); 
					} else {
						printhelp(argv[0]);
					}
					registernick();
					break;
				case 'm':
					if (i+1 < argc) {
						i++;
						strcpy(message, argv[i]);
						type = TWEET ;
					} else {
						printhelp(argv[0]);
					}
					sendmessage();
					break;
				case 's':
					if (i+2 < argc) {
						i++;
						strcpy (nick, argv[i]);
						i++;
						strcpy (message, argv[i]);
						type = PRIVE_TWEET;
					} else {
						printhelp(argv[0]);
					}
					sendmessage();
					break;
				case 'p':
					if (i+1 < argc) {
						i++;
						strcpy(password, argv[i]);
					} else {
						printhelp(argv[0]);
					}
					break;
				case 't':
					if (i+2 < argc) {
						i++;
						strcpy (nick, argv[i]);
						i++;
						strcpy (message, argv[i]);
						type = TOPIC_TWEET;
					} else {
						printhelp(argv[0]);
					}
					sendmessage();
					break;
				case 'g':
					receivemessage();
					break;
				case 'h':
					printhelp(argv[0]);
					break;
			}
		}
	}
	close(dsd);
}

void
registernick() {
	strcpy(buffer, password);
	buffer[strlen(password)+1]=REGISTER_NICK;
	strcpy(&buffer[strlen(password)+2], message);
	if (connect(dsd, adr->ai_addr, adr->ai_addrlen) == -1) {
		perror("is the daemon running?");
	}
	send(dsd, buffer,strlen(password)+strlen(message)+3, 0);
	recv(dsd, buffer, 1, 0);
	if (buffer[0] == 1) {
		printf("Succes!\n");
		return;
	}
	printf("Fail..\n");
}

void
sendmessage() {
	strcpy(buffer, password);
	buffer[strlen(password)+1]=SEND_MESSAGE;
	buffer[strlen(password)+2]=type;
	
	if (type == TWEET) {
		strcpy(&buffer[strlen(password)+3], message);
	} else {
		strcpy(&buffer[strlen(password)+3], nick);
		buffer[strlen(password)+strlen(nick)+3] = ' ';
		strcpy(&buffer[strlen(password)+strlen(nick)+4], message);
	}

	if (connect(dsd, adr->ai_addr, adr->ai_addrlen) == -1) {
		perror("is the daemon running?");
	}
	send(dsd, buffer,strlen(password)+strlen(nick)+strlen(message)+5, 0);
	recv(dsd, buffer, 1, 0);
	if (buffer[0] == 1) {
		printf("Succes!\n");
		return;
	}
	printf("Fail..\n");
}

void
receivemessage() {
	printf("Getting all waiting messages ... \n\n");
	while (1) {
	  	strcpy(buffer, password);
		buffer[strlen(password)+1]=RECEIVE_MESSAGE;
		if (connect(dsd, adr->ai_addr, adr->ai_addrlen) == -1) {
			perror("is the daemon running?");
		}
		send(dsd, buffer,strlen(password)+2, 0);
    	recv(dsd, buffer, 141, 0);
		if (buffer[0] == '\0') break;
		if (buffer[0] == 2) printf("@");
		if (buffer[0] == 3) printf("#");
		printf("%s\n",&buffer[1]);
		
		close(dsd);
		if ( 0 > (dsd = socket(adr->ai_family, adr->ai_socktype, adr->ai_protocol)) )
			perror("receive msg error");
	}
	printf("No more messages\n");
}
