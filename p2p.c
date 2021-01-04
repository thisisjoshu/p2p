/*

COMP3331 Assignment
Written by Joshua Z 
Student ID: z5196042

*/

#include "p2p.h"


// thread functions
void *inputHandler();
void *detectClose();
void *requestJoin();
void *pingSuccessors();
void *receivePings();
void *handleTCPMessages();


// processing input functions
void processJoinRequest(struct message *msg);
void forwardJoinRequest(struct message *msg);
void receiveJoinRequest(struct message *msg);
void changeSuccessorsJoin(struct message *msg);
void changeSuccessorsQuit(struct message *msg);
void processStoreRequest(struct message *msg);
void processRetrieveRequest(struct message *msg);
void processRetrieveResponse(struct message *msg);
void gracefullyQuit(struct message *msg);
void processQuitRequest(struct message *msg);
void handleQuitResponse(struct message *msg);


int main(int argc, char *argv[]) {
    if (argc != 5 && argc != 6) {
        printf("Usage: %s init peer first_successor ", argv[0]);
        printf("second_successor ping_interval\n");
        printf("OR\n");
        printf("Usage: %s join peer known_peer ping_interval\n", argv[0]);
        exit(1);
    }
    
    strcpy(type, argv[1]);
    peerID = atoi(argv[2]);
    fileList = newList();
    lostPings[0] = 0;
    lostPings[1] = 0;
    
    if (strcmp(type, INIT) == 0) {
        fsID = atoi(argv[3]);
        ssID = atoi(argv[4]);
        pingInterval = atoi(argv[5]);
        status = NORMAL;
        
    } else if (strcmp(type, JOIN) == 0) {
        knownPeer = atoi(argv[3]);
        pingInterval = atoi(argv[4]);
        status = JOINING;
        
    } else {
        exit(1);
    }
	
    pthread_t pthS;
    pthread_t pthR;
    pthread_t pthJR;
    pthread_t pthRJ;
    pthread_t pthI;
    pthread_t pthP;
    
    pthread_create(&pthJR, NULL, handleTCPMessages, NULL);
    usleep(100000);
    pthread_create(&pthRJ, NULL, requestJoin, NULL);
    usleep(100000);
    pthread_create(&pthS, NULL, pingSuccessors, NULL);
    usleep(10000);
    pthread_create(&pthR, NULL, receivePings, NULL);
    usleep(1000);
    pthread_create(&pthI, NULL, inputHandler, NULL);
    usleep(1000);
    pthread_create(&pthP, NULL, detectClose, NULL);
    usleep(1000);
	    
    while(1);
    
    return 0;
}


// Handles input from the peer 
void *inputHandler() {
    char buffer[100];
    
    while (fgets(buffer, 100, stdin) != NULL) {
        char buffer2[100];
        strcpy(buffer2, buffer);
        char *command = strtok(buffer2, " ");
        
        if (strcasecmp(command, QUIT) == 0) {
            status = DEPARTING;
            quit(GRACEFULLY, peerID);
            
        } else if (strcasecmp(command, STORE) == 0) {
            int file = processFilename(buffer);
            if (file > 0) storeFile(file);
            
        } else if (strcasecmp(command, REQ) == 0) {
            int file = processFilename(buffer);
            if (file > 0) retrieveFile(file);
            
        } else {
        	printf("Command not recognized\n");
       	}
    }
}


// Detects whether or not a peer is alive
void *detectClose() {
    while(true) {
    	
        if (lostPings[0] >= 3) {
            quit(ABRUPT, fsID);
        	// wait 1 sec to make sure lostPings is reset
        	usleep(1000000);
        } else if (lostPings[1] >= 3) {
        	// wait 1 sec before getting successors
        	usleep(1000000);
        	quit(ABRUPT, ssID);
        	// wait 1 sec to make sure lostPings is reset
        	usleep(1000000);
       	}
    }
}


// Handles the scenario where a peer wants to join the
// existing network 
void *requestJoin() {
    if (status == JOINING) {
    
        Message joinRequest;
        joinRequest = createMessage(peerID, JOIN_REQUEST, peerID, 
                                    knownPeer, -1, -1);
        
        sendViaTCP(knownPeer, joinRequest);
    }
}


// Send pings its successor every 10 seconds as long as the 
// peer is in a 'NORMAL' state. Pings are sent via UDP
void *pingSuccessors() {
    // ping message
    Message request;
    request = createMessage(peerID, REQUEST, -1, -1, -1, -1);
    
	struct sockaddr_in recvaddr;
	 
	int sock_fd = createSocket(SOCK_DGRAM); 
	
    while (status == NORMAL) {
	    recvaddr = createSockAddr(AF_INET, htons(SERVER_PORT + fsID),
	                              inet_addr(LOCALHOST));
	    
	    // send ping request to first successor
	    sendto(sock_fd, request, sizeof(struct message), 0, 
	           (const struct sockaddr *)&recvaddr, sizeof(recvaddr));
		
		lostPings[0]++;
		
		recvaddr = createSockAddr(AF_INET, htons(SERVER_PORT + ssID),
	                              inet_addr(LOCALHOST));
	    
	    // send ping request to second successor
	    sendto(sock_fd, request, sizeof(struct message), 0, 
	           (const struct sockaddr *)&recvaddr, sizeof(recvaddr));
		
			    
		printf("> Ping requests sent to Peers %d and %d\n", 
		       fsID, ssID);
		
		lostPings[1]++;
		// wait 10 seconds before pinging again
	    usleep(10000000);
	}
    close(sock_fd); 
}


// Receives pings sent from other peers. It then 
// sends back a ping response. The response is sent via UDP
void *receivePings() {
	Message response;
    response = createMessage(peerID, RESPONSE, -1, -1, -1, -1);
    
	struct sockaddr_in servaddr, cliaddr, myaddr; 
	
	int sock_fd = createSocket(SOCK_DGRAM); 
	
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	servaddr = createSockAddr(AF_INET, htons(SERVER_PORT + peerID), 
	                          htonl(INADDR_ANY));
	
	const int so_reuseaddr = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(int));

	if (bind(sock_fd, (const struct sockaddr *)&servaddr, 
			 sizeof(servaddr)) < 0) { 
		perror("Bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	int n; 
	char buffer[100]; 
	int len = sizeof(cliaddr);
    
    while (status == NORMAL) {
	    n = recvfrom(sock_fd, (char *)buffer, 100, 0, 
	                 (struct sockaddr *) &cliaddr, &len); 
	    buffer[n] = '\0'; 
	    
	    if (((Message) buffer)->type == RESPONSE) {
	        printf("> Ping response message received from Peer %d\n",
	               ((Message) buffer)->sender);
	               
	        if (((Message) buffer)->sender == fsID) lostPings[0] = 0;
	        else if (((Message) buffer)->sender == ssID) lostPings[1] = 0;
	        
	    } else if (((Message) buffer)->type == REQUEST) {
	        printf("> Ping request message received from Peer %d\n",
	               ((Message) buffer)->sender);
	        
	        myaddr = createSockAddr(AF_INET, htons(SERVER_PORT + ((Message)buffer)->sender), 
	                                inet_addr(LOCALHOST));
	        
	        // send response
	        sendto(sock_fd, response, sizeof(struct message), 0, 
	               (const struct sockaddr *) &myaddr, len); 
	    }
	}
}


// Handles all the incomming messages that are via TCP
void *handleTCPMessages() {
    struct sockaddr_in servaddr;
    servaddr = createSockAddr(AF_INET, htons(SERVER_PORT + peerID), 
	                          htonl(INADDR_ANY));
	                                
    int listen_sock = createSocket(SOCK_STREAM); 
    
    if ((bind(listen_sock, (struct sockaddr *)&servaddr,
              sizeof(servaddr))) < 0) {
        perror("Bind failed"); 
		exit(EXIT_FAILURE); 
    }
    
    int wait_size = 10;
    
    if (listen(listen_sock, wait_size) < 0) {
        perror("Could not open socket for listening\n");
        exit(EXIT_FAILURE); 
    }
    
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    
    while (true) {
        int sock = accept(listen_sock, (struct sockaddr *)&client_address,
                          &client_address_len);
                                       
        if (sock < 0) {
            perror("Could not open a socket to accept data\n");
            exit(EXIT_FAILURE); 
        }
               
        char buffer[1000];
        int n = recv(sock, (char *)buffer, 1000, 0); 
        buffer[n] = '\0'; 
        
        // message received is a join request 
        if (((Message) buffer)->type == JOIN_REQUEST) {
            
        	processJoinRequest((Message) buffer);
        // message received requests to change successors 
        // upon join request     
        } else if (((Message) buffer)->type == CHANGE_REQUEST) {
            
            changeSuccessorsJoin((Message) buffer);
         
        // message received is a quit request    
        } else if (((Message) buffer)->type == QUIT_REQUEST) {

        	processQuitRequest((Message) buffer);
        	
        // message received requests to change successors upon a 
        // quit request     
        } else if (((Message) buffer)->type == QUIT_CHANGE) {
        
            changeSuccessorsQuit((Message) buffer);
        
        // message received notifies peer to exit program upon a
        // graceful quit request     
        } else if (((Message) buffer)->type == QUIT_NOW) {
            
            gracefullyQuit((Message) buffer);
        
        // message received requests to store a file         
        } else if (((Message) buffer)->type == STORE_REQUEST) {
            
            processStoreRequest((Message) buffer);
        
        // message received requests to retrieve a file     
        } else if (((Message) buffer)->type == RETRIEVE_REQUEST) {
        	
        	processRetrieveRequest((Message) buffer);
        
        // message received responds to a retrieve request	
        } else if (((Message) buffer)->type == RETRIEVE_RESPONSE) {

        	processRetrieveResponse((Message) buffer);
        
        }
        //close client specific socket
        close(sock);
    }
    //close listening socket
    close(listen_sock);
}


// Processes a join request 
// It will either forward or receive a join request 
void processJoinRequest(struct message *msg) {
	int origin = msg->origin;
	int known = msg->known;
            
	if (needToForward(origin, known))
		forwardJoinRequest(msg);
	else
		receiveJoinRequest(msg);

}


// Forwards the join request to its first successor 
void forwardJoinRequest(struct message *msg) {
	Message joinRequest;
	joinRequest = createMessage(peerID, JOIN_REQUEST, 
								msg->origin, 
                         		msg->known, 
                       			-1, -1);
		                 
	sendViaTCP(fsID, joinRequest);    
	printf("> Peer %d Join request forwarded to my successor\n", 
		   msg->origin);
}


// Receives the join request and responds respectively
void receiveJoinRequest(struct message *msg) {
	printf("> Peer %d Join request received\n", msg->origin);
                        
	int newPeerFirst = fsID;
	int newPeerSecond = ssID;
                        
	ssID = fsID;
	fsID = msg->origin;
                        
	printPeers(fsID, ssID, "new ");
                        
	// send message to predecessor
	Message cRequest;
	cRequest = createMessage(peerID, CHANGE_REQUEST, 
                           	 msg->origin, -1, peerID, 
                           	 msg->origin);
                        
	sendViaTCP(msg->sender, cRequest);
                        
	// send message to new peer
	cRequest->newFirst = newPeerFirst;
	cRequest->newSecond = newPeerSecond;
                        
	sendViaTCP(msg->origin, cRequest);
}


// Changes the successors upon a join request 
void changeSuccessorsJoin(struct message *msg) {
	fsID = msg->newFirst;
	ssID = msg->newSecond;
            
	if (peerID == msg->origin) {
		printf("> Join request has been accepted\n");  
   		printPeers(fsID, ssID, "");
		status = NORMAL;
	} else {
		printf("> Successor Change request received\n");
		printPeers(fsID, ssID, "new ");
	}
}


// Changes the successors upon a quit request 
// It then prints out the peer's new successors 
void changeSuccessorsQuit(struct message *msg) {
	if (msg->known == GRACEFULLY) {
		printf("> Peer %d will depart from network\n", msg->origin);
		            
		fsID = msg->newFirst;
		ssID = msg->newSecond;
		        
		printPeers(fsID, ssID, "new ");
		
	} else {
	
		if (msg->newFirst == -1) {
			ssID = msg->newSecond;
			lostPings[0] = 0;
		} else if (msg->newSecond == -1) {
			ssID = msg->newFirst;
			lostPings[1] = 0;
		}
            
		printPeers(fsID, ssID, "new ");
	}
}


// Processes the store request
// It will either forward or accept store request 
void processStoreRequest(struct message *msg) {
	int closest = closestPeer(msg->known, peerID, fsID);
	
	if ((msg->known != peerID && msg->known > fsID) || 
		(msg->known != peerID && closest != peerID)) {
            	
		printf("> Store %d forwarded to my successor\n", msg->origin);
		Message storeRequest;
		storeRequest = createMessage(peerID, STORE_REQUEST, 
                                	 msg->origin, 
                                  	 msg->known, -1, -1);
                
		sendViaTCP(fsID, storeRequest);
		
	} else {
		printf("> Store %d request accepted\n", msg->origin);
		addFileToStorage(msg->origin);
	}
}


// Processes the retrieve file request 
// It will either forward the request message or send the file 
// to the peer that requested it
void processRetrieveRequest(struct message *msg) {
	int closest = closestPeer(msg->known, peerID, fsID);
	
	if ((msg->known != peerID && msg->known > fsID) || 
		(msg->known != peerID && closest != peerID)) {
            	
		printf("> Request for File %d has been received, but the file is not stored here\n", msg->origin);
		Message retrieveRequest;
		retrieveRequest = createMessage(peerID, RETRIEVE_REQUEST, 
                                                msg->origin, 
                                                msg->known, 
                                                msg->newFirst, -1);	
		            
		sendViaTCP(fsID, retrieveRequest);
                
	} else if (ListContains(fileList, msg->origin) == TRUE) {
		printf("> File %d is stored here\n", msg->origin);
                
		printf("> Sending file %d to Peer %d\n", msg->origin, 
                								 msg->newFirst);
                								 
		// sends file to peer who requested it
		sendFile(msg->newFirst, msg->origin);
		printf("> The file has been sent\n");
	}
}


// Processes the response from a retrieve request 
// It will receive the file 
void processRetrieveResponse(struct message *msg) {
	printf("> Peer %d had File %d\n", msg->sender, msg->origin);
	printf("> Receiving File %d from Peer %d\n", msg->origin, msg->sender);
	
	// receive file 
	receiveFile(msg->origin, msg->payload);
	printf("> File %d received\n", msg->origin);

}


// Simply ends the process (peer)
void gracefullyQuit(struct message *msg) {
	if (msg->known == GRACEFULLY)
		exit(0);
}


// Processes a quit request 
// It will either forward the request or make a response
void processQuitRequest(struct message *msg) {
        
	if (fsID != msg->origin && msg->newSecond != -1 &&
		msg->newFirst != -1) {
            	 
		// forward
		Message quitRequest;
		quitRequest = createMessage(peerID, QUIT_REQUEST, 
                                	msg->origin, 
                                  	msg->known, 
                               		msg->newFirst,
                                 	msg->newSecond);
		                 
   		sendViaTCP(fsID, quitRequest);  
   		  
	} else {
		handleQuitResponse(msg);
	} 
}


// Sends a response to the quit request
void handleQuitResponse(struct message *msg) {

	if (msg->known == GRACEFULLY) {
		printf("> Peer %d will depart from network\n", msg->origin);
                
		fsID = msg->newFirst;
		ssID = msg->newSecond;
		            
		int predFirst = peerID;
		int predSecond = fsID;
		                    
		printPeers(fsID, ssID, "new ");
		                    
		// send message to predecessor
		Message qcRequest;
		qcRequest = createMessage(peerID, QUIT_CHANGE, 
		                          msg->origin, 
		                          msg->known, 
		                          predFirst, predSecond);
		                    
		sendViaTCP(msg->sender, qcRequest);
		                    
		// send message to departing peer
		Message quitMsg;
		quitMsg = createMessage(peerID, QUIT_NOW, msg->origin, 
		                        msg->known, -1, -1);
		                    
		sendViaTCP(msg->origin, quitMsg);
			
	} else {
		// send message to predecessors of the departing peer
		Message qcRequest;
		qcRequest = createMessage(peerID, QUIT_CHANGE, 
		                       	  msg->origin, 
		                          msg->known, 
		                          -1, fsID);
		        	
		if (msg->newFirst == -1) {
			qcRequest->newFirst = fsID;
			qcRequest->newSecond = -1;
		}
		        	
		sendViaTCP(msg->sender, qcRequest);
	}
}
