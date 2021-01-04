/*

COMP3331 Assignment
Written by Joshua Z 
Student ID: z5196042

*/

#ifndef P2P_H
#define P2P_H

#include <arpa/inet.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>
#include "List.h"
#include "functions.h"

// message types
#define REQUEST         	0
#define RESPONSE        	1
#define JOIN_REQUEST    	2
#define CHANGE_REQUEST  	3
#define QUIT_REQUEST    	4
#define QUIT_CHANGE     	5
#define QUIT_NOW        	6
#define STORE_REQUEST   	7
#define RETRIEVE_REQUEST	8
#define RETRIEVE_RESPONSE	9

// quit types 
#define ABRUPT          	0
#define GRACEFULLY      	1

// peer statuses
#define NORMAL          	0
#define JOINING         	1
#define DEPARTING       	2

// address info
#define SERVER_PORT     	12200
#define LOCALHOST       	"127.0.0.1"

// commands
#define INIT            	"init"
#define JOIN            	"join"
#define QUIT            	"quit\n"
#define STORE				"store"
#define REQ					"request"

#define TRUE            	1
#define FALSE           	0

// Messages sent over the network will use this struct
struct message {
    int sender;
    int type;
    int origin;
    int known;
    int newFirst;
    int newSecond;
    char payload[1000];
};

typedef struct message *Message;

// list of filenames
List fileList;

int peerID;
int fsID;
int ssID;
int pingInterval;
int status;
int knownPeer;
char type[5];
int lostPings[2];


#endif
