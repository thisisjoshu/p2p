/*

COMP3331 Assignment
Written by Joshua Z 
Student ID: z5196042

*/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// creates a new message of type 'struct message'. A value of -1 means
// no value
struct message *createMessage(int sender, int type, int origin, 
                              int known, int newFirst, int newSecond);
                              
// creates a new sockaddr_in                              
struct sockaddr_in createSockAddr(sa_family_t fam, in_port_t port, 
                                  uint32_t addr);

// Processes the filename input                                  
int processFilename(char buffer[]);

// Starts a quit request upon determining which type of quit 
// should be used
void quit(int type, int quitter);

// Initiates a store file request to successor
void storeFile(int file);

// Initiates a retrieve file request 
void retrieveFile(int file);

// Converts file into Message type and sends to peer who requested the
// file
void sendFile(int receiver, int fileName);

// Receives a file from a peer. Prints it into a another file with
// the same name
void receiveFile(int fileName, char msg[]);

// creates a socket
int createSocket(int type);

// Performs the procedure of sending a file via TCP. Since most of the 
// messages sent on the network are sent via TCP, this wrapper function 
// saves space
void sendViaTCP(int port, struct message *msg);

// Simply adds a file to the peer's list
void addFileToStorage(int fileName);

// Returns closest peer to a hash
int closestPeer(int hash, int peer1, int peer2); 

// Returns hash 
int hashFunction(int file);	

// Determines whether a join request should be forwarded or not
int needToForward(int origin, int known);

// Simply print successors of a peer 
void printPeers(int first, int second, char type[]); 

// Determines whether or not a filename is valid
int validFile(char file[]);	

#endif
