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

char *detectContentType(char *filename){
    char extension[16]; strcpy(extension,getFileExtension(filename));
    static char content_type[32]; strcpy(content_type,"text/plain");
    if(strcmp(extension,"html")==0){strcpy(content_type,"text/html");}
    if(strcmp(extension,"js")==0){strcpy(content_type,"text/javascript");}
    if(strcmp(extension,"css")==0){strcpy(content_type,"text/css");}
    if(strcmp(extension,"png")==0){strcpy(content_type,"image/png");}
    if(strcmp(extension,"jpg")==0){strcpy(content_type,"image/jpeg");}
    if(strcmp(extension,"ico")==0){strcpy(content_type,"image/png");}
    return content_type;
}

char *generateResponse(int response_code,char *content_type,char *content){
    static char buffer[1024+sizeof(content)];

    char response_code_str[3];
    sprintf(response_code_str, "%d",response_code);

    strcpy(buffer,"HTTP/1.1 ");
    strcat(buffer,response_code_str);
    strcat(buffer," OK \r\n");
    strcat(buffer,"Content-Type: ");
    strcat(buffer,content_type);
    strcat(buffer,"\r\n");
    strcat(buffer,"Server: QuantumMultiWebserverV1\r\n\r\n");
    strcat(buffer,content);

    return buffer;
}

char * getResponseFromURI(char *uri){

    char file_path[256];
    strcpy(file_path,"./data");
    if(strcmp(uri,"/")==0){
        strcat(file_path,"/index.html");
    }else{
        strcat(file_path,uri);
    }
    
    printf("filename: %s\n\n",file_path);

    //load data from file
    char * buffer=0;
    long length;
    FILE *f = fopen(file_path, "rb");
    if (f){
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc (length);
        if (buffer){
            fread (buffer, 1, length, f);
        }
        fclose (f);
    }
    if(buffer){
        char content_type[1024];strcpy(content_type,detectContentType(uri));
        return generateResponse(200,content_type,buffer);
    }else{
       return generateResponse(200,"text/html","<h1>file not found</h1>");
    }
    
    free(buffer);
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
        printf("data sent was: %s\n",resp);
        int valwrite = write(newsockfd, resp, strlen(resp));
        if (valwrite < 0) {
            perror("webserver (write)");
            continue;
        }

        close(newsockfd);
    }

    return 0;
}
