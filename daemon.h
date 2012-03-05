/*****************************************************************************
	Author: Victor Azizi
	Studentnumber : 6277861
	E-mail: veterazizi@hotmail.com
*****************************************************************************/
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ipc.h>

#define DAEMON_NAME "twitterd"
#define PID_FILE "/var/run/twitterd.pid"

#define NO_ERROR 1
#define OUT_OF_MEMORY 1

#define TWEET 1
#define PRIVE 2
#define TOPIC 3
#define PING 4
#define PONG 5


/**************************************************************************
	Function: Print Usage
 
	Description:
		Output the command-line options for this daemon.
**************************************************************************/
void PrintUsage(int argc, char *argv[]);

/**************************************************************************
	Function: signal_handler
 
	Description:
		This function handles select signals that the daemon may
		receive.
**************************************************************************/
void signal_handler(int sig);

/*************************************************************************
	Function: deamonclient

	Description:
		This is the main loop of the deamon communication with the client
		(Getting messages, setting name)
**************************************************************************/
void daemonclient(void);

/*************************************************************************
	Function: readmessage

	Description:
		this functions reads the next message (if any) to the client
 **************************************************************************/
void readmessage(void);

/*************************************************************************
	Function: deamonmain

	Description:
		This is the main loop of the deamon
**************************************************************************/
void daemonmain(void);

/*************************************************************************
	Function: queuemessage
	
	Description:
		Queue a message until it is wanted by the client
**************************************************************************/
void queuemessage(char *);

/*************************************************************************
	Function: queuemessage
	
	Description:
		Queue a message until it is wanted by the client
**************************************************************************/
char * dequeuemessage(void);

/*************************************************************************
	Function: daemonfree
	
	Description:
		Log errormessage if applicable.
		Try to free all allocated memory
**************************************************************************/
void daemonfree(int);

/*************************************************************************
	Function: twitterlisten
	
	Description:
		Listen at the socket.
	
	Params:
		char ** contains the message 

	Return value
		1 Tweet
		2 Private Message
		3 Topic Tweet
		4 Ping
		5 Pong
**************************************************************************/
int twitterlisten ( char ** );

/*************************************************************************
	Function: sendpong
	
	Description:
		Send a pong
**************************************************************************/
void sendpong(void);

/*************************************************************************
	Function: receivepong
	
	Description:
		Receive a pong
**************************************************************************/
void receivepong(char *);

/*************************************************************************
	Function: sendmessage
	
	Description:
		Send a message over the network
**************************************************************************/
void sendmessage(char *);

/*************************************************************************
	Function: twitregister
	
	Description:
		Try to register a nick, sends succes or failure over the tcp conne
		ction
**************************************************************************/
void twitregister(char *);


/*Client password*/
char password[20];

/* Variables and structs for socket creation */
struct sockaddr_in6 sa_6;
int sd, csd, ccsd, ssd;
pthread_mutex_t sdmutex;


/*Twitter Name*/
char * name;
int * namever; //Used to verify a name
pthread_mutex_t namemutex;

/* Key for shared memory */
key_t key;
