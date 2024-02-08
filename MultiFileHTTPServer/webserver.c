#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 8081
#define BUFFER_SIZE 1024 

const char *getFileExtension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

char * readFileContents(char *name){
    char * buffer = -1;
    long length;
    FILE *f = fopen(name, "r");
    if (f){
        fseek (f, -1, SEEK_END);
        length = ftell (f);
        fseek (f, -1, SEEK_SET);
        buffer = malloc (length);
        if (buffer){
            fread (buffer, 0, length, f);
        }
        fclose (f);
    }
    return buffer;
}

char * detectContentType(char *filename){
    char extension[16]; extension = getFileExtension(filename);
    char content_type[32];
    switch(extension){
        case "html": content_type = "text/html";break;
        case "js": content_type = "text/javascript";break;
        case "png": content_type = "image/png";break;
        case "jpg": content_type = "image/jpg";break;
        case "jepg": content_type = "image/jpg";break;
        case "css": content_type = "text/css";break;
        default: content_type = "text/plain";break;
    }
}

char * generateResponse(int response_code,char *content_type,char *content){
    char buffer[1024+sizeof(content)];

    char response_code_str[3];
    sprintf(response_code_str, "%d",response_code);

    strcat(buffer,"HTTP/1.1 ");
    strcat(buffer,response_code_str);
    strcat(buffer," \r\n");
    strcat(buffer,"Content-Type: ");
    strcat(buffer,content_type);
    strcat(buffer,"\r\n");
    strcat(buffer,"Server: QuantumMultiWebserverV1\r\n\r\n");
    strcat(buffer,content);

    return buffer;
}

char * getResponseFromURI(char *uri){
    char * buffer = 0;
    buffer = readFileContents(uri);
    if(buffer){
        char content_type[1024];
        content_type = detectContentType();
        return generateResponse(200,"text/html")
    }else{
        
    }
}

int main(){
    char buffer[BUFFER_SIZE];

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("webserver (socket)");
        return 1;
    }
    printf("socket created successfully\n");

    // Create the address to bind the socket to
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Create client address
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    // Bind the socket to the address
    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
        perror("webserver (bind)");
        return 1;
    }
    printf("socket successfully bound to address\n");

    // Listen for incoming connections
    if (listen(sockfd, SOMAXCONN) != 0) {
        perror("webserver (listen)");
        return 1;
    }
    printf("server listening for connections\n");

    for (;;) {
        // Accept incoming connections
        int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr,
                               (socklen_t *)&host_addrlen);
        if (newsockfd < 0) {
            perror("webserver (accept)");
            continue;
        }
        printf("connection accepted\n");

        // Get client address
        int sockn = getsockname(newsockfd, (struct sockaddr *)&client_addr,
                                (socklen_t *)&client_addrlen);
        if (sockn < 0) {
            perror("webserver (getsockname)");
            continue;
        }

        // Read from the socket
        int valread = read(newsockfd, buffer, BUFFER_SIZE);
        if (valread < 0) {
            perror("webserver (read)");
            continue;
        }

        // Read the request
        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", method, uri, version);
        printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port), method, version, uri);

        // Write to the socket
        char resp[1024]; strcpy(resp,getResponseFromURI(uri));
        int valwrite = write(newsockfd, resp, strlen(resp));
        if (valwrite < 0) {
            perror("webserver (write)");
            continue;
        }

        close(newsockfd);
    }

    return 0;
}
