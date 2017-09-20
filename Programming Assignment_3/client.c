/**
*
* Client program to send the request with URL to proxy-server
* author : Anurag Chitnis
*
*/

#include <sys/types.h>

#include <sys/socket.h>

#include <netdb.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>



#define URL "www.example.com"

 

int main(int argc,char **argv)

{

    int sockfd, n, tmpres;

    int len = sizeof(struct sockaddr);

    char sendline[100];

    char recvline[40960];
    char buf[BUFSIZ+1];

    char *message , server_reply[40960];
    char *htmlcontent;



    struct sockaddr_in servaddr;



    if(argc == 1){

        fprintf(stderr, "USAGE: ./client <port number> <URL>\n");

        fprintf(stderr, "Default URL is www.example.com\n");

        exit(2);

    }

    if(argc == 2) {

      puts("USAGE: ./client <port number> <URL>\n");

      puts("Default URL is www.example.com\n\n");

      message = URL;

    }

    else {

      message = argv[2];

    }

 

    sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    bzero(&servaddr,sizeof(servaddr));

 

    servaddr.sin_family=AF_INET;

    servaddr.sin_port=htons(atoi(argv[1]));

    // hard coded the proxy server's IP address

    inet_pton(AF_INET,"129.120.151.94",&(servaddr.sin_addr));

 

    connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));



    //Send the captured URL

    if( send(sockfd , message , strlen(message) , 0) < 0)

    {

        puts("Send failed");

        return 1;

    }

    FILE *mypage = fopen("MyPage.html","w");

    memset(buf, 0, sizeof(buf));

    puts("Reply received\n");


   //Receive a reply from the server
    while((tmpres = recv(sockfd, buf, BUFSIZ, MSG_WAITALL)) > 0){
	htmlcontent = buf;

	fprintf(stdout, htmlcontent);
	fputs(htmlcontent, mypage);
	memset(buf, 0, tmpres);

    }


   fclose(mypage);

    //puts(buf);
   close(sockfd);



   return 0;

 

}
