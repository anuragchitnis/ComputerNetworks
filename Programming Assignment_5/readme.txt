##############################################
Programming assignment - 5

Demonstration of TCP 3-way handshake, data transfer and closing a TCP connection using a client-server architecture. 

Author : Anurag Chitnis
##############################################

Instructions to run the tcp server program:

1. Login to remote server: cse01.cse.unt.edu
2. compile the tcp-server.c file using
   >:$ gcc tcp-server.c -o tcp-server

3. Execute the pserver using
   >:$ ./tcp-server <port no.>

where tcp-server is the tcp server program executable and port_number is the port number on which the 
tcp server listens

/***************************************************/
Instructions to run the tcp client program:

1. Login to remote server: cse02.cse.unt.edu
2. compile the client.c file using
   >:$ gcc tcp-client.c -o tcp-client

3. execute the client using
   >:$ ./tcp-client <port no.>

where client is the tcp client executable, 
port_number is the port number on which the client connects the tcp server. 

/************************************/
Code Structure:

Following functions are used in the client and server program:

/**
* This function logs the tcp header to the file.
*
*/
void logHeader(struct tcp_hdr tcp_seg)


/**
* This function prints the message to console and logs it to the file.
*
*/
void logMessage(char* message)


/**
* This function prints the output to the console.
*/
void printAndWriteToFile(struct tcp_hdr tcp_seg)


/**
This function verifies if the checksum is correct and returns 1 if it is.
It returns 0 if checksum doesn't match indicating the corrupted data.
*/
int verifyChecksum(struct tcp_hdr tcp_seg)

/**
This function computes the checksum from the tcp header passed to it as an argument.
Returns the checksum value.
*/
unsigned short int computeChecksum(struct tcp_hdr tcp_seg)

/**
 * @brief This method reads the data starting from offset from the file.
 * @param offset
 * @return pointer to the string read from the file
 */
char* readDataFromFile(long int offset)


/**
 * @brief This function writes the string to the file.
 * @param data
 */
void writeDataToFile(char* data)


Note : The server program writes the output to the "server.out" file and client program writes the output to "client.out" file.
