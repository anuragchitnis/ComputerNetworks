/**
* Programming Assignment #4
*
* Demonstration of TCP 3-way handshake and closing a TCP connection using a client-server architecture.  
* Author : Anurag Chitnis
*/


#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>

#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Structure of the TCP header
struct tcp_hdr{
                unsigned short int src;
                unsigned short int des;
                unsigned int seq;
                unsigned int ack;
                unsigned short int hdr_flags;
                unsigned short int rec;
                unsigned short int cksum;
                unsigned short int ptr;
                unsigned int opt;
              };

unsigned short int computeChecksum(struct tcp_hdr tcp_seg);
int verifyChecksum(struct tcp_hdr tcp_seg);
void printAndWriteToFile(struct tcp_hdr tcp_seg);
void logMessage(char* message);
void logHeader(struct tcp_hdr tcp_seg);

int main(int argc, char *argv[])
{
	struct tcp_hdr client_tcp_seg, server_tcp_seg;
        int sockfd, connected, bytes_recieved , true = 1;
	unsigned short int segment_arr[12];
       // char send_data [1024] , recv_data[1024];

      if(argc == 1){
        fprintf(stderr, "USAGE: ./tcp-server <port number>\n");
        exit(2);
      }       
 
        struct sockaddr_in server_addr,client_addr;    
        int sin_size;
         
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Socket");
            exit(1);
        }
 
        if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) == -1) {
            perror("Error in Setsockopt");
            exit(1);
        }
         
        server_addr.sin_family = AF_INET;         
        server_addr.sin_port = htons(atoi(argv[1]));     
        server_addr.sin_addr.s_addr = INADDR_ANY; 
  	memset(&(server_addr.sin_zero),0,8);
        if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))== -1) 
   	{
            perror("Unable to bind");
            exit(1);
        }
 
        if (listen(sockfd, 5) == -1) 
        {
            perror("Listen");
            exit(1);
        }
   
        printf("\n\nTCP-Server is waiting on port %d \n", atoi(argv[1]));
        fflush(stdout);
         
 
	sin_size = sizeof(struct sockaddr_in); 

	connected = accept(sockfd, (struct sockaddr *)&client_addr,&sin_size);

	read(connected, segment_arr,sizeof(segment_arr));
	memcpy(&client_tcp_seg, segment_arr, 24); //Copying 24 bytes

 logMessage("################# Demonstrating the TCP 3-way handshake #############\n");
 logMessage("#####################################################################\n\n");
 logMessage("************** Received TCP segment from the client ****************\n");
	// print the header
	printAndWriteToFile(client_tcp_seg);
	
	// if checksum is not verified correctly, print the corrupted data message
	if(!verifyChecksum(client_tcp_seg)) {
		logMessage("Corrupted data\n");
	} else
                logMessage("Checksum verification successfull!\n");



  // The  server  responds  to  the  request  by  creating  a  connection  granted  TCP  segment 
  server_tcp_seg.src = atoi(argv[1]);
  server_tcp_seg.des = (unsigned short int) ntohs(client_addr.sin_port);
  server_tcp_seg.seq = 20; // arbitary sequence number
  server_tcp_seg.ack = client_tcp_seg.seq + 1;
  server_tcp_seg.hdr_flags = (sizeof(client_tcp_seg)/4 << 12) | 0b010010;
  server_tcp_seg.rec = 0;
  server_tcp_seg.ptr = 0;
  server_tcp_seg.opt = 0;
  server_tcp_seg.cksum = computeChecksum(server_tcp_seg);

	// print the header
	logMessage("************** Response segment, created by server ****************\n");
	printAndWriteToFile(server_tcp_seg);

    memcpy(segment_arr, &server_tcp_seg, 24); //Copying 24 bytes

    //Write  the tcp segment to the socket
    if( write(connected , segment_arr , sizeof(segment_arr)) < 0)
    {
        puts("Write failed");
        return 1;
    }

	read(connected, segment_arr,sizeof(segment_arr));
	memcpy(&client_tcp_seg, segment_arr, 24); //Copying 24 bytes

	logMessage("************** Received TCP segment from the client ****************\n");
	printAndWriteToFile(client_tcp_seg);
	if(!verifyChecksum(client_tcp_seg)) {
		logMessage("Corrupted data\n");
	}
	else if(client_tcp_seg.ack == (server_tcp_seg.seq + 1)) {
		logMessage("Received ACK - y, Connection successfull! Client is live.\n\n");
	}

 logMessage("###################### Finished the 3-way handshake ###################\n\n");

	read(connected, segment_arr,sizeof(segment_arr));
	memcpy(&client_tcp_seg, segment_arr, 24); //Copying 24 bytes

 logMessage("################ Demonstrating the closing of TCP connection ###################\n");
 logMessage("################################################################################\n\n");
 logMessage("************** Received TCP close request from the client ****************\n");
	// print the header
	printAndWriteToFile(client_tcp_seg);
	
	if(!verifyChecksum(client_tcp_seg)) {
		logMessage("Corrupted data\n");
	} else
		logMessage("Checksum verification successfull!\n");

  // The  server  responds  to  the  request  by  creating  a  new segment 

  server_tcp_seg.seq++;
  server_tcp_seg.ack = client_tcp_seg.seq + 1;
  server_tcp_seg.hdr_flags = (sizeof(client_tcp_seg)/4 << 12) | 0b010010;
  server_tcp_seg.rec = 0;
  server_tcp_seg.ptr = 0;
  server_tcp_seg.opt = 0;
  server_tcp_seg.cksum = computeChecksum(server_tcp_seg);

	// print the header
	logMessage("************** Response segment to a connection close request, created by server ****************\n");
	printAndWriteToFile(server_tcp_seg);

	memcpy(segment_arr, &server_tcp_seg, 24); //Copying 24 bytes
    //Write  the tcp segment to the socket
    if( write(connected , segment_arr , sizeof(segment_arr)) < 0)
    {
        puts("Write failed");
        return 1;
    }

  server_tcp_seg.seq++;
  server_tcp_seg.ack = client_tcp_seg.seq + 1;
  server_tcp_seg.hdr_flags = (sizeof(client_tcp_seg)/4 << 12) | 0b000001;
  server_tcp_seg.rec = 0;
  server_tcp_seg.ptr = 0;
  server_tcp_seg.opt = 0;
  server_tcp_seg.cksum = computeChecksum(server_tcp_seg);

	// print the header
	logMessage("************** Close request by the server, sending it to client ****************\n");
	printAndWriteToFile(server_tcp_seg);

    memcpy(segment_arr, &server_tcp_seg, 24); //Copying 24 bytes

    //Write  the tcp segment to the socket
    if( write(connected , segment_arr , sizeof(segment_arr)) < 0)
    {
        puts("Write failed");
        return 1;
    }

	read(connected, segment_arr,sizeof(segment_arr));
	memcpy(&client_tcp_seg, segment_arr, 24); //Copying 24 bytes

	logMessage("************** Received final ACK from the client ****************\n");

	// print the header
	printAndWriteToFile(client_tcp_seg);
	
	if(!verifyChecksum(client_tcp_seg)) {
		logMessage("Corrupted data\n");
	}  else
                logMessage("Checksum verification successfull!\n");

	logMessage("###################### Finished closing the TCP connection ###################\n\n");

	close(connected);


	return 0;
}

/**
This function computes the checksum from the tcp header passed to it as an argument.
Returns the checksum value.
*/
unsigned short int computeChecksum(struct tcp_hdr tcp_seg) {

  unsigned short int cksum_arr[12];
  unsigned int i,sum=0, cksum;
  tcp_seg.cksum = 0; // reset the checksum field to zero

  memcpy(cksum_arr, &tcp_seg, 24); //Copying 24 bytes
 
  for (i=0;i<12;i++)               // Compute sum
   sum = sum + cksum_arr[i];

  cksum = sum >> 16;              // Fold once
  sum = sum & 0x0000FFFF; 
  sum = cksum + sum;

  cksum = sum >> 16;             // Fold once more
  sum = sum & 0x0000FFFF;
  cksum = cksum + sum;

  /* XOR the sum for checksum */
  return (0xFFFF^cksum);
}

/**
This function verifies if the checksum is correct and returns 1 if it is.
It returns 0 if checksum doesn't match indicating the corrupted data.
*/
int verifyChecksum(struct tcp_hdr tcp_seg) {
	return (computeChecksum(tcp_seg) == tcp_seg.cksum);
}

/**
* This function prints the output to the console.
*/
void printAndWriteToFile(struct tcp_hdr tcp_seg) {

  printf("Src : %d\n", tcp_seg.src); // Printing all values
  printf("Dest : %d\n", tcp_seg.des);
  printf("Seq : %d\n", tcp_seg.seq);
  printf("Ack : %d\n", tcp_seg.ack);
  printf("Header Flags : 0x%04X\n", tcp_seg.hdr_flags);
  printf("Rec : 0x%04X\n", tcp_seg.rec);
  printf("Checksum : 0x%04X\n", tcp_seg.cksum);
  printf("Ptr : 0x%04X\n", tcp_seg.ptr);
  printf("Optional : 0x%08X\n\n", tcp_seg.opt);

  // log the header to a file
  logHeader(tcp_seg);

}

/**
* This function prints the message to console and logs it to the file "server.out"
*
*/
void logMessage(char* message) {
  // print the message to the console
  printf(ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET,message);
  FILE *logFile = fopen("server.out", "a+");
  // Writing all values to a file
  fputs(message, logFile);
  fclose(logFile);

}

/**
* This function logs the tcp header to the file "server.out"
*
*/
void logHeader(struct tcp_hdr tcp_seg) {

  FILE *logFile = fopen("server.out", "a+");
  // Writing all values to a file
  fprintf(logFile,"Src : %d\n", tcp_seg.src);
  fprintf(logFile,"Dest : %d\n", tcp_seg.des);
  fprintf(logFile,"Seq : %d\n", tcp_seg.seq);
  fprintf(logFile,"Ack : %d\n", tcp_seg.ack);
  fprintf(logFile,"Header Flags : 0x%04X\n", tcp_seg.hdr_flags);
  fprintf(logFile,"Rec : 0x%04X\n", tcp_seg.rec);
  fprintf(logFile,"Checksum : 0x%04X\n", tcp_seg.cksum);
  fprintf(logFile,"Ptr : 0x%04X\n", tcp_seg.ptr);
  fprintf(logFile,"Optional : 0x%08X\n\n", tcp_seg.opt);

  fclose(logFile);
}
