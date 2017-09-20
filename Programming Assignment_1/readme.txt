Instructions to run the proxy server:

1. Login to remote server: cse01.cse.unt.edu
2. compile the pserver.c file using
   >:$ gcc pserver.c -o pserver

3. Execute the pserver using
   >:$ ./pserver <port no.>

where pserver is the proxy server executable and port_number is the port number on which the 
proxy server listens

/***************************************************/
Instructions to run the client program:

1. Login to remote server: cse02.cse.unt.edu
2. compile the client.c file using
   >:$ gcc client.c -o client

3. execute the client using
   >:$ ./client <port no.> <URL>

where client is the client executable, 
port_number is the port number on which the client connects the server
and url is the requested url. 
