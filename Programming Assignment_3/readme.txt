##############################################
Programming assignment - 3

Proxy Server program to accept a single client's request
Implemented web-caching and web-site blacklisting.
Implemented logging of the activities and blocking of the web-page with inappropriate content.

References: 
http://coding.debuntu.org/system/files/htmlget.c

Tested with the following URLS:

www.unt.edu
www.unc.edu
www.example.com

Author : Anurag Chitnis
##############################################

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

/************************************/
Code Structure:

Description of functions

/**
*
* Function to create the socket for HTTP request to host server
*
*/
    int create_tcp_socket()


/**
*
* Function to find the IP address from the host URL
*
*/    
char *get_ip(char *host)


/**
*
* Function to build the HTTP request query
*
*/
char *build_get_query(char *host, char *page)


/**
* This function caches the web page for the given URL
*
*/
void cache_web_page(char *host_url, char *htmlcontent, char *file_name)



/**
* This function performs the file operation required by other function
*/
void performFileOperation(char *url, char *cache_file_name)


/**
* This function updates the "list.txt" file with the Url name and respective cache file's name.
* This file can hold 5 entries, if the 6th entry is made it deletes the 1 entry in the file.
*/
void updateCacheList(char *url , char *cache_file_name)



/**
* Check if the webUrl is Blacklisted
* return: 1 if the URL is blacklisted, 0 otherwise
*/
int isBlacklisted(char *webUrl)



/**
* This method finds if we have a cached file for the current hostUrl, if it exists we return the all the contet in the file in string format.
* If the cache file doesn't exist, it returns the NULL pointer.
*/
char *getCachedFile(char *hostUrl)



/**
* This function checks if the current time is in the range of startTime and endTime.
* return : 1 if it is in the range, 0 otherwise
*/
int isDateTimeInRange(double startTime, double endTime)



/**
* Returns the Date and Time in the format YYYYMMDDhhmmss. Where YYYY is the year, MM is the month, 
* DD is the day, hh is the hour in 24-hour format, mm is the minutes, and ss is the seconds
*
*/
char * getCurrentDateTime()



/**
* Add the log of activity to the log file, with time-stamp, URL and status code
* File name is log.txt
*
*/
void logMsg(char *url, char *statusCode)



/**
* This function checks if the language used in the web-page is inappropriate. It checks it from the words provided in the file lang.txt
* returns 1 if the language is inappropriate, 0 if it is appropriate
*/
int isInappropriateLanguage(char *webPage)

