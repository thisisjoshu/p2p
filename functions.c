/*

COMP3331 Assignment
Written by Joshua Z 
Student ID: z5196042

*/

#include "p2p.h"

// creates a new message of type 'struct message'. A value of -1 means
// no value
struct message *createMessage(int sender, int type, int origin, int known, 
                              int newFirst, int newSecond) {
                                                
    Message msg = malloc(sizeof(struct message));
    msg->sender = sender;
    msg->type = type;
    msg->origin = origin;
    msg->known = known;
    msg->newFirst = newFirst;
    msg->newSecond = newSecond;
    
    return msg;
}

// creates a new sockaddr_in
struct sockaddr_in createSockAddr(sa_family_t fam, in_port_t port, 
                                  uint32_t addr) {
                                  
    struct sockaddr_in SA;
    
    memset(&SA, 0, sizeof(SA)); 
    SA.sin_family = fam; 
    SA.sin_port = port; 
    SA.sin_addr.s_addr = addr; 

    return SA;
}

// Processes the filename input
int processFilename(char buffer[]) {
    char fileString[10];
    char temp[10];
    int file = -1;
    
    sscanf(buffer, "%s %s", temp, fileString);
    if (validFile(fileString) == TRUE) {
        file = atoi(fileString);
        return file;
    } else {
        printf("> You entered an invalid file name.\n");
        return file;
    }
}

// Starts a quit request upon determining which type of quit 
// should be used
void quit(int type, int quitter) {
    
    Message quitRequest;
    if (type == GRACEFULLY) {
        quitRequest = createMessage(peerID, QUIT_REQUEST, quitter, 
                                    type, fsID, ssID);
    } else if (type == ABRUPT && quitter == fsID) {
		printf("> Peer %d is no longer alive\n", quitter);
    	int formerFirst = fsID;
    	fsID = ssID;
        quitRequest = createMessage(peerID, QUIT_REQUEST, quitter, 
                                    type, formerFirst, -1);
    } else {
    	printf("> Peer %d is no longer alive\n", quitter);
    	quitRequest = createMessage(peerID, QUIT_REQUEST, quitter, 
                                    type, -1, fsID);
    }
    
    sendViaTCP(fsID, quitRequest);
}


// Initiates a store file request to successor
void storeFile(int file) {
    int position = hashFunction(file);
    
    if (peerID != position) {
        Message storeRequest;
        storeRequest = createMessage(peerID, STORE_REQUEST, file, 
                                     position, -1, -1);
        
        sendViaTCP(fsID, storeRequest);
        printf("> Store %d forwarded to my successor\n", file);    
    } else {
    	printf("> Store %d request accepted\n", file);
		addFileToStorage(file);
    }
}


// Initiates a retrieve file request  
void retrieveFile(int file) {
	int position = hashFunction(file);
	
	if (peerID != position) {
		Message retrieveRequest;
		retrieveRequest = createMessage(peerID, RETRIEVE_REQUEST, file,
										position, peerID, -1);
		
		sendViaTCP(fsID, retrieveRequest);
		printf("> File request for %d has been sent to my successor\n", file);
	} else {
		printf("> I already have File %d\n", file);
	}

}

// Converts file into Message type and sends to peer who requested the
// file. The basis of this function was inspired from this source:
// See: https://stackoverflow.com/questions/174531/how-to-read-the-content-of-a-file-to-a-string-in-c
void sendFile(int receiver, int fileName) {
	char *buff;
	char file[8];
    sprintf(file, "%d", fileName);
    strcat(file, ".txt");
    
	FILE *fp;
	fp = fopen(file, "r");
										

	if (fp == NULL) {
		printf("Error opening File\n");
		return;
	}

	Message retrieveResponse;
	retrieveResponse = createMessage(peerID, RETRIEVE_RESPONSE, 
                                     fileName, -1, -1, -1);
                                     
	fseek(fp, 0, SEEK_END);
  	int length = ftell (fp);
  	fseek(fp, 0, SEEK_SET);
  	buff = malloc(length);
  	
  	fread(buff, 1, length, fp);
  	
  	strcpy(retrieveResponse->payload, buff);
  	
	fclose (fp);	
	sendViaTCP(receiver, retrieveResponse);
}

// Receives a file from a peer. Prints it into a another file with
// the same name
void receiveFile(int fileName, char msg[]) {
	char file[8];
	char newFile[9];
	
	strcat(newFile, "_");
    sprintf(file, "%d", fileName);
    strcat(file, ".txt");
    strcat(newFile, file);
	FILE *fp;
	fp = fopen(newFile, "w");
	
	if (fp == NULL) {
		printf("Error opening File\n");
		return ;
	}
	
	fprintf(fp, "%s", msg);
	
	fclose(fp);
}

// creates a socket
int createSocket(int type) {
    int sock_fd = socket(AF_INET, type, 0);
	
    if (sock_fd < 0) { 
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE); 
    }
    
    return sock_fd;
}

// Performs the procedure of sending a file via TCP. Since most of the 
// messages sent on the network are sent via TCP, this wrapper function 
// saves space
void sendViaTCP(int port, struct message *msg) {
	int sock_fd = createSocket(SOCK_STREAM);
	            
    struct sockaddr_in myaddr;
    myaddr = createSockAddr(AF_INET, htons(SERVER_PORT + port), 
	                        inet_addr(LOCALHOST));
	                                    
    int conn = connect(sock_fd, (struct sockaddr*)&myaddr, sizeof(myaddr));

    if (conn != 0) {  
        perror("Connection with the peer failed"); 
        exit(EXIT_FAILURE); 
    }
	
	send(sock_fd, msg, sizeof(struct message), 0);
	
    close(sock_fd);
}


// Simply adds a file to the peer's list
void addFileToStorage(int fileName) {
    ListInsert(fileList, fileName);
}

// Returns closest peer to a hash 
int closestPeer(int hash, int peer1, int peer2) {
	int diff1 = abs(hash - peer1);
	int diff2 = abs(hash - peer2);
	
	if (diff1 < diff2) return peer1;
	else if (diff1 > diff2) return peer2;
	else return peer2;

}

// Returns hash 
int hashFunction(int file) {
    return file % 256;
}

// Determines whether a join request should be forwarded or not
int needToForward(int origin, int known) {
    int forward = FALSE;
    
    if (origin > peerID && origin > fsID &&
        fsID != known) {
        forward = TRUE;   
    } else if (origin < peerID && origin < fsID &&
    		   fsID != known) {
		forward = TRUE;
	}
    
    return forward;
}

// Simply print successors of a peer 
void printPeers(int first, int second, char type[]) {
    printf("> My %sfirst successor is Peer %d\n", type, first);
    printf("> My %ssecond successor is Peer %d\n", type, second);
}

// Determines whether or not a filename is valid
int validFile(char file[]) {
    if (strlen(file) != 4) return FALSE;
    
    for (int i = 0; i < strlen(file); i++) {
        if (isdigit(file[i]) == 0) return FALSE;
    }
    return TRUE;
}
