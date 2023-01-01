/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_HEADER_SIZE 8192

void dostuff(int); /* function prototype */
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    while (1) {
        newsockfd = accept(sockfd, 
            (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0)  {
            close(sockfd);
            dostuff(newsockfd);
            exit(0);
        }
        else close(newsockfd);
    } /* end of while */
    close(sockfd);
    return 0; /* we never get here */
}

char* read_file(char* path);

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff (int sock)
{
   int n;
   char buffer[MAX_HEADER_SIZE];
      
   bzero(buffer,MAX_HEADER_SIZE);
   n = read(sock,buffer,MAX_HEADER_SIZE-1);
   if (n < 0) error("ERROR reading from socket");
   // printf("Here is the message: %s\n",buffer);

    char* response;
    char* content;
    int header_len = strlen("HTTP/1.0 200 OK\nContent-Type: text/html\n\n");
    content = read_file("./content/index.html");
    response = malloc(header_len + strlen(content) - 1);
    strcpy(response, "HTTP/1.0 200 OK\nContent-Type: text/html\n\n");
    printf("%s\n", response);
    strcpy(response+header_len, content);
    

    // char* response = "HTTP/1.0 200 OK";
    // char* response = "HTTP/1.0 200 OK\nContent-Type: text/html\nContent-length: 78\n\n<html>\n<head>\n<title>HTTP</title>\n\n\n</head>\n<body>\n<p> HTTP/1.1-Demo</p>\n</body>\n</html>";

    n = write(sock,response,strlen(response));
    if (n < 0) error("ERROR writing to socket");
}

char* read_file(char* path) {
    char* buffer;
    int length;
    FILE* f = fopen(path, "rb");
    if(f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length + 1);
        if(buffer != NULL) {
            printf("Buffer allocated!\n");
            fread(buffer, 1, length, f);
        }
        fclose(f);
        buffer[length] = '\0';
    } else return NULL;

    if(buffer) {
        // if(strstr(path, ".html") != NULL) {
        // } else {
        // }

        printf("First character: %c\n", buffer[0]);
    }
    return buffer;
}