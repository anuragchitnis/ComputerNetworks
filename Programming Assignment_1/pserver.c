/*

Proxy Server program to accept a single client's request

author : Anurag Chitnis

*/


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include<string.h>
#include<stdlib.h>

int create_tcp_socket();
char *get_ip(char *host);
char *build_get_query(char *host, char *page);

#define HOST "www.example.com"
#define PAGE "/"
#define PORT 80
#define USERAGENT "HTMLGET 1.0"

int main(int argc, char *argv[])
{
        int sock_proxy, connected, bytes_recieved , true = 1;  
        char send_data [1024] , recv_data[1024]; 
      
      struct sockaddr_in *remote;
      int sock;
      int tmpres;
      char *ip;
      char *get;
      char buf[BUFSIZ+1];
      char *host;
      char *page;

      if(argc == 1){
        fprintf(stderr, "USAGE: ./pserver <port number>\n");
        exit(2);
      }       
 
        struct sockaddr_in server_addr,client_addr;    
        int sin_size;
         
        if ((sock_proxy = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Socket");
            exit(1);
        }
 
        if (setsockopt(sock_proxy,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) == -1) {
            perror("Setsockopt");
            exit(1);
        }
         
        server_addr.sin_family = AF_INET;         
        server_addr.sin_port = htons(atoi(argv[1]));     
        server_addr.sin_addr.s_addr = INADDR_ANY; 
        //bzero(&(server_addr.sin_zero),8); --This is for POSIX based systems
  memset(&(server_addr.sin_zero),0,8);
        if (bind(sock_proxy, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))== -1) 
    {
            perror("Unable to bind");
            exit(1);
        }
 
        if (listen(sock_proxy, 5) == -1) 
        {
            perror("Listen");
            exit(1);
        }
   
        printf("\n\nProxy Server is waiting on port %d \n", atoi(argv[1]));
        fflush(stdout);
         
 
   sin_size = sizeof(struct sockaddr_in);
 
   connected = accept(sock_proxy, (struct sockaddr *)&client_addr,&sin_size);
 
   char message_from_client[9999];
   recv(connected,message_from_client,sizeof(message_from_client),0);
   printf("\nReceived URL: %s \n\n",message_from_client);

// For fetching the web-page from the host
    page = PAGE;
    host = message_from_client;
    // Create socket for communication with host
    sock = create_tcp_socket();

      // Fetch the IP address of the host URL
      ip = get_ip(host);
      fprintf(stderr, "IP is %s\n", ip);
      remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
      remote->sin_family = AF_INET;
      tmpres = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));
      if( tmpres < 0)  
      {
        perror("Can't set remote->sin_addr.s_addr");
        exit(1);
      }else if(tmpres == 0)
      {
        fprintf(stderr, "%s is not a valid IP address\n", ip);
        exit(1);
      }
      remote->sin_port = htons(PORT);
     
      if(connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0){
        perror("Could not connect");
        exit(1);
      }

      // build the query form the host or URL received from the client
      get = build_get_query(host, page);
      fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);
     
      //Send the query to the server
      int sent = 0;
      while(sent < strlen(get))
      {
        tmpres = send(sock, get+sent, strlen(get)-sent, 0);
        if(tmpres == -1){
          perror("Can't send query");
          exit(1);
        }
        sent += tmpres;
      }

      //Logic to receive the HTML page starts from here
      memset(buf, 0, sizeof(buf));
      int htmlstart = 0;
      char * htmlcontent;
      while((tmpres = recv(sock, buf, BUFSIZ, 0)) > 0){
        if(htmlstart == 0)
        {
          /* Under certain conditions this will not work.
          * If the \r\n\r\n part is splitted into two messages
          * it will fail to detect the beginning of HTML content
          */
          htmlcontent = strstr(buf, "\r\n\r\n");
          if(htmlcontent != NULL){
            htmlstart = 1;
            htmlcontent += 4;
          }
        }else{
          htmlcontent = buf;
        }
        if(htmlstart){
         fprintf(stdout, "\nSending HTML content to the client...\n");
         int c = send(connected,htmlcontent,strlen(htmlcontent),0);
         close (connected);
        }
     
        memset(buf, 0, tmpres);
      }
      if(tmpres < 0)
      {
        perror("Error receiving data");
      }
      free(get);
      free(remote);
      free(ip);
      close(sock);
      close(sock_proxy);

}

/*
*
* Function to create the socket for HTTP request to host server
*
*/
    int create_tcp_socket()
    {
      int sock;
      if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        perror("Can't create TCP socket");
        exit(1);
      }
      return sock;
    }

/*
*
* Function to find the IP address from the host URL
*
*/    
    char *get_ip(char *host)
    {
      struct hostent *hent;
      int iplen = 15; //XXX.XXX.XXX.XXX
      char *ip = (char *)malloc(iplen+1);
      memset(ip, 0, iplen+1);
      if((hent = gethostbyname(host)) == NULL)
      {
        herror("Can't get IP");
        exit(1);
      }
      if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == '\0')
      {
        perror("Can't resolve host");
        exit(1);
      }
      return ip;
    }
    
/*
*
* Function to build the HTTP request query
*
*/
    char *build_get_query(char *host, char *page)
    {
      char *query;
      char *getpage = page;
      char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
      if(getpage[0] == '/'){
        getpage = getpage + 1;
        fprintf(stderr,"Removing leading \"/\", converting %s to %s\n", page, getpage);
      }
      // -5 is to consider the %s %s %s in tpl and the ending \0
      query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);
      sprintf(query, tpl, getpage, host, USERAGENT);
      return query;
    }
