/*****************************************************************************
	Author: Victor Azizi
	Studentnumber : 6277861
	E-mail: veterazizi@hotmail.com
*****************************************************************************/
//very simple structures for creating a dynamicly sized queue

typedef struct message {
	char * inhoud;
	struct message * next;
} message;

typedef struct shead {
	message * head;
} shead;
