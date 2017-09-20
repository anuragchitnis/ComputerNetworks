/*

Programming assignment #3

Proxy Server program to accept a single client's request
Implemented web-caching and web-site blacklisting.
Implemented logging of the activities and blocking of the web-page with inappropriate content.

References: 
http://coding.debuntu.org/system/files/htmlget.c

Author : Anurag Chitnis

*/


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int create_tcp_socket();
char *get_ip(char *host);
char *build_get_query(char *host, char *page);
char *getCurrentDateTime();
void cache_web_page(char *host_url, char *htmlcontent, char *file_name);
void updateCacheList(char *url , char *cache_file_name);
char *getCachedFile(char *hostUrl);
void logMsg(char *url, char *statusCode);
int isInappropriateLanguage(char *webPage);

#define HOST "www.example.com"
#define PAGE "/"
#define PORT 80
#define CACHE_SIZE 5
#define INITIAL_ALLOC 512

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

   //Check if the received URL is blacklisted
   if(isBlacklisted(message_from_client)) {
	char blockedMessage[100] = "This URL is blocked";
	 fprintf(stdout, "\nSending the message that URL is blocked\n");
         send(connected,blockedMessage,strlen(blockedMessage),0);
	 logMsg(message_from_client, "BLOCK"); // log this to a file
         close (connected);
   }
   else {

   page = PAGE;
   host = message_from_client;
   char *cachedPage = getCachedFile(host);

   if(cachedPage != NULL) {
	fprintf(stdout, "\nSending the cached web page\n");
         send(connected,cachedPage,strlen(cachedPage),0);
	 logMsg(host, "CACHE"); // log this to a file
         close (connected);
   }
   else{
	// For fetching the web-page from the host, if it is not blacklisted and cached

    // Create socket for communication with host
    sock = create_tcp_socket();

      // Fetch the IP address of the host URL
      ip = get_ip(host);
      fprintf(stderr, "IP is %s\n", ip);
      remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
      remote->sin_family = AF_INET;
      tmpres = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));

      remote->sin_port = htons(PORT);
     
	// connect to remote host
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
      char *cacheFileName;
      char *completePage = (char *) malloc(20 * BUFSIZ);
      int isSuccess = 0;
      while((tmpres = recv(sock, buf, BUFSIZ, 0)) > 0){
        if(htmlstart == 0)
        {
          htmlcontent = strstr(buf, "\r\n\r\n");
          if(htmlcontent != NULL){
           htmlstart = 1;
            htmlcontent += 4;
          }
	 if(strstr(buf, "200 OK")){
		isSuccess = 1;
		cacheFileName = getCurrentDateTime();
		updateCacheList(host, cacheFileName);
		logMsg(host, "OK200"); // log this to a file
	 }
	 else {
	 	logMsg(host, "BADRQ"); // log this to a file
	 }

        } else {
		htmlcontent = buf;
	}

        if(htmlstart){

	  // check if response is successfull
	  if(isSuccess){
	//	strcat(completePage, htmlcontent);
		//completePage = (char*) realloc(completePage, strlen(completePage) + BUFSIZ + 1);		
		strcat(completePage, htmlcontent);
	//	cache_web_page(host, htmlcontent, cacheFileName);		
	  }

        // fprintf(stdout, "\nSending HTML content to the client...\n");
        // int c = send(connected, htmlcontent,strlen(htmlcontent),0); 	        
        }      
	
        memset(buf, 0, tmpres);
      }

	//char *completePage = getCachedFile(host);
	// don't send the page if language is inappropriate
	if(!isInappropriateLanguage(completePage)) {
		cache_web_page(host, completePage, cacheFileName);
		fprintf(stdout, "\nSending HTML content to the client\n");
        	send(connected,completePage,strlen(completePage),0);
	}
	else {
	 char inAppLang[100] = "Language used on this page is inappropriate";
	 fprintf(stdout, "\nSending the message that language is inappropriate\n");
         send(connected,inAppLang,strlen(inAppLang),0);
	 logMsg(host, "INAPP"); // log this to a file
	}
	free(completePage);
	close (connected);
     }
    }
      free(get);
      free(remote);
      free(ip);
      close(sock);
      close(sock_proxy);

}

/**
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

/**
*
* Function to find the IP address from the host URL
*
*/    
char *get_ip(char *host)
{
      struct hostent *hent;
      int iplen = 15;
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
    
/**
*
* Function to build the HTTP request query
*
*/
char *build_get_query(char *host, char *page)
{
      char *query;
      char *getpage = page;
      char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n";
      if(getpage[0] == '/'){
        getpage = getpage + 1;
      }
      query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(tpl)-5);
      sprintf(query, tpl, getpage, host);
      return query;
}

/**
* This function caches the web page for the given URL
*
*/
void cache_web_page(char *host_url, char *htmlcontent, char *file_name) {
	printf("debug : cache_web_page() \n");
	
	FILE *cache_file = fopen(file_name, "a");
	if (cache_file == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}
	
	/* write the HTML content to file */
	fputs(htmlcontent, cache_file);
	fclose(cache_file);
}

/**
* This function performs the file operation required by other function
*/
void performFileOperation(char *url, char *cache_file_name) {
	  FILE *temp = fopen("temp.txt","w");
	  FILE *listFile = fopen("list.txt", "r");
	  char urlTemp[40], fileName[80];
	  char *line = NULL;
	  size_t len = 0;
	  ssize_t readFirst;
	  int lines = 0; //reset the lines variable
	 //delete first line	
  	 while ((readFirst = getline(&line, &len, listFile)) != -1) {
	     if(lines != 0) {
		sscanf(line, "%s %s", urlTemp, fileName);
		/* write the URL and cache file's name to temp.txt */
	  	fprintf(temp, "%s %s\n", urlTemp, fileName);
		//fputs(line, temp);
	     }
	     else {
	        sscanf(line, "%s %s", urlTemp, fileName);
		remove(fileName);
 	    }
	     lines++;
   	 }
	  /* write the URL and cache file's name to temp.txt */
	  fprintf(temp, "%s %s\n", url, cache_file_name);	  
	  fclose(temp);
	  fclose(listFile);
	  remove("list.txt");
          //rename the file temp.txt to original name
          rename("temp.txt", "list.txt");
	 
}

/**
* This function updates the "list.txt" file with the Url name and respective cache file's name.
* This file can hold 5 entries, if the 6th entry is made it deletes the 1 entry in the file.
*/
void updateCacheList(char *url , char *cache_file_name) {
	printf("debug : updateCacheList()\n");
	int lines = 0;
	
	char ch;
	char urlToDelete[40], fileName[15];

	size_t len = 0;
	ssize_t readFirst, readSecond;
	char *line = NULL;
	char urlTemp[40];
	FILE *listFile = fopen("list.txt", "a+");
	if(listFile != NULL) {
  	 while ((readFirst = getline(&line, &len, listFile)) != -1) {
	     if(lines == 0) {
		sscanf(line, "%s %s", urlTemp, fileName);
		//break;
	     }
	     lines++;
   	 }

	if(lines == CACHE_SIZE) {
		performFileOperation(url, cache_file_name);	
	}
	else {
	  /* write the URL and cache file's name to list.txt */
	  fprintf(listFile, "%s %s\n", url, cache_file_name);
	}
	fclose(listFile);
      }

	    
}


/**
* Check if the webUrl is Blacklisted
* return: 1 if the URL is blacklisted, 0 otherwise
*/
int isBlacklisted(char *webUrl) {
	int isBlacklisted = 0;
	char *lineB = NULL;
	size_t lenB = 0;
	ssize_t read;
	//long double startTime, endTime;
	FILE *blacklist = fopen("blacklist.txt", "r");
	if (blacklist == NULL)
	{
	    printf("Error opening blacklist.txt file!\n");
	    exit(1);
	}
        while ((read = getline(&lineB, &lenB, blacklist)) != -1) {
            if ( strstr(lineB, webUrl) ){
		char * pch;
		const char *strTokens[3];
		int i = 0;
  		pch = strtok (lineB," ");
  		while (pch != NULL) {
		    printf ("%s\n",pch);
		    strTokens[i] = pch;
		    pch = strtok (NULL, " ");
		    i++;
		}

		if(isDateTimeInRange(atof(strTokens[1]), atof(strTokens[2]))) {
			isBlacklisted = 1; // true
		}
		break;
            }
        }
	fclose(blacklist);
	return isBlacklisted;
}


/**
* This method finds if we have a cached file for the current hostUrl, if it exists we return the all the contet in the file in string format.
* If the cache file doesn't exist, it returns the NULL pointer.
*/
char *getCachedFile(char *hostUrl) {
	char *buffer = NULL;
	size_t size = 0, len =0;
	ssize_t read;
	char *line = NULL;
	char url[40], fileName[15] = "init";
	FILE *listFile = fopen("list.txt", "r");
	if(listFile != NULL) {
  	 while ((read = getline(&line, &len, listFile)) != -1) {
	     if(strstr(line, hostUrl) != NULL) {
		sscanf(line, "%s %s", url, fileName);
		break;
	     }
   	 }

	FILE *fp = fopen(fileName, "r");
	/* Open your_file in read-only mode */
	if(fp != NULL) {

		/* Get the buffer size */
		fseek(fp, 0, SEEK_END); /* Go to end of file */
		size = ftell(fp); /* How many bytes did we pass ? */	
		/* Set position of stream to the beginning */
		rewind(fp);
		buffer = malloc((size + 1) * sizeof(*buffer)); /* size + 1 byte for the \0 */
		fread(buffer, size, 1, fp); /* Read 1 chunk of size bytes from fp into buffer */
		buffer[size] = '\0';
	}
	}

	return buffer;
}

/**
* This function checks if the current time is in the range of startTime and endTime.
* return : 1 if it is in the range, 0 otherwise
*/
int isDateTimeInRange(double startTime, double endTime) {
	double dateToCheck = atof(getCurrentDateTime());
	return dateToCheck >= startTime && dateToCheck < endTime;
}

/**
* Returns the Date and Time in the format YYYYMMDDhhmmss. Where YYYY is the year, MM is the month, 
* DD is the day, hh is the hour in 24-hour format, mm is the minutes, and ss is the seconds
*
*/
char * getCurrentDateTime() {
	
	/*The filename format is YYYYMMDDhhmmss. Where YYYY is the year, MM is the month, DD is the day, hh is the hour in 24-hour 
	format, mm is the minutes, and ss is the seconds when the web site was visited */

	time_t now;
        time(&now);
	struct tm* now_tm;
	now_tm = localtime(&now);

	char *dateTimeString = malloc(80);
	strftime (dateTimeString, 80, "%Y%m%d%H%M%S", now_tm);
	printf("debug : getCurrentDateTime() : %s\n", dateTimeString);

	return dateTimeString;
}

/**
* Add the log of activity to the log file, with time-stamp, URL and status code
* File name is log.txt
*
*/
void logMsg(char *url, char *statusCode) {
	FILE *logFile = fopen("log.txt", "a+");

	char *timeStamp = getCurrentDateTime();
	/* write the URL and cache file's name to list.txt */
	fprintf(logFile, "%s %s %s\n", timeStamp, url, statusCode);

	fclose(logFile);
}

/**
* This function checks if the language used in the web-page is inappropriate. It checks it from the words provided in the file lang.txt
* returns 1 if the language is inappropriate, 0 if it is appropriate
*/
int isInappropriateLanguage(char *webPage) {

	size_t len =0;
	ssize_t read;
	char word[50];
	char *line = NULL;
	FILE *langFile = fopen("lang.txt", "r");
	int result = 0;
	if(langFile != NULL) {
  	 while ((read = getline(&line, &len, langFile)) != -1) {
	     //scan the word on that line
	     sscanf(line, "%s", word);
	     if(strstr(webPage, word) != NULL) {
		result = 1;
		break;
	     }
   	 }
	fclose(langFile);
	}

//	printf("debug : isInappropriateLanguage() : %d\n", result);
	return result;
}



