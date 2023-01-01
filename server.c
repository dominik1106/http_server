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
const char* CONTENT_PATH = "./content/";
const char* HTTP_VERSION = "HTTP/1.0";

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
char* get_MIME(char* extension);

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
    // printf("Request:\n%s\n",buffer);

    //No support for any http types except GET right now
    if(strncmp(buffer, "GET", 3) != 0) {
        n = write(sock, "HTTP/1.0 404 Not found", strlen("HTTP/1.0 404 Not found"));
        if (n < 0) error("ERROR writing to socket");
    }

    //Get the filename
    char* start; char* end;
    start = strchr(buffer, '/')+1;
    end = strchr(start, ' ');
    int path_length = end - start;


    //Concat the static ./content/ location to the filename
    char* file_path = malloc(path_length + strlen(CONTENT_PATH) + 1);
    strcpy(file_path, CONTENT_PATH);
    strncpy((file_path + strlen(CONTENT_PATH)), start, path_length);
    file_path[path_length + strlen(CONTENT_PATH)] = '\0';

    //Get MIME corresponding to the fileextension
    char* content_type = get_MIME(strrchr(file_path, '.'));
    // printf("Test %s", strrchr(file_path, '.'));

    char* content = read_file(file_path);
    if(content == NULL) {
        n = write(sock,"HTTP/1.0 404",strlen("HTTP/1.0 404"));
        if (n < 0) error("ERROR writing to socket");
        return;
    }

    int response_length = strlen("HTTP/1.0 200 OK\nContent-Type: ") + strlen(content_type) + strlen(content) + 3;
    char* response = malloc(response_length);
    start = response;
    
    strcpy(response, "HTTP/1.0 200 OK\nContent-Type: ");
    response += strlen("HTTP/1.0 200 OK\nContent-Type: ");
    strcpy(response, content_type);
    response += strlen(content_type);
    *(response++) = '\n';
    *(response++) = '\n';
    strcpy(response, content);
    response += strlen(content);
    *response = '\0';
    response = start;

    n = write(sock,response,strlen(response));
    if (n < 0) error("ERROR writing to socket");

    printf("%s\n", response);

    free(file_path);
    free(content);
    free(content_type);
    free(response);
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
            fread(buffer, 1, length, f);
        }
        fclose(f);
        buffer[length] = '\0';
    } else return NULL;
    
    return buffer;
}

char* get_MIME(char* extension) {
    char* type;
    if(strcmp(extension, ".html") == 0) {
        type = malloc(10);
        strcpy(type, "text/html");
        type[9] = '\0';
    } else if(strcmp(extension, ".css") == 0) {
        type = malloc(9);
        strcpy(type, "text/css");
        type[8] = '\0';
    } else {
        type = malloc(11);
        strcpy(type, "text/plain");
        type[10] = '\0';
    }
    return type;
}