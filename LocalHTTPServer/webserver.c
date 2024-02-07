#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

#define PORT 8081
#define BUFFER_SIZE 1024

//https://bruinsslot.jp/post/simple-http-webserver-in-c/

//SOCKET........DONE
//BIND..........DONE
//LISTEN........DONE
//ACCEPT........DONE
//READ/WRITE....DONE

int main(){
    printf("starting webserver...\n");
    
    char buffer[BUFFER_SIZE];
    char resp[] = "HTTP/1.0 200 OK\r\n"
                  "Server: webserver-c\r\n"
                  "Content-type: text/html\r\n\r\n"
                  "<html><h1>test server</h1></html>\r\n";

    //create a socket
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd==-1){
        //print error if something goes wrong
        perror("webserver (socket)");
        return 1;
    }
    printf("socket created sucessfuly!\n");
    
    //create host adress
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //create client adress
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    //bind socket
    if(bind(sockfd,(struct sockaddr *)&host_addr,host_addrlen)!=0){
        //print error if something goes wrong
        perror("webserver (bind)");
        return 1;
    }
    printf("sucessfuly bound socket to adress!\n");

    //listen for incoming connections
    if(listen(sockfd,SOMAXCONN)!=0){
        //once again print error if something goes wrong
        perror("webserver (listen)");
        return 1;
    }
    printf("server listening for connections...\n");

    printf("server setup is now complete!\n");

    //keep looking for connections to accept
    for(;;){
        //accept incoming connections
        int newsockfd = accept(sockfd,(struct sockaddr *)&host_addr,(socklen_t *)&host_addrlen);
        if(newsockfd<0){
            //show error if something goes wrong, but keep accepting more connections
            perror("webserver (accept)");
            continue;
        }
        printf("connection accepted!\n");
        
        //get client adress
        int sockn = getpeername(newsockfd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addrlen);
        if(sockn<0){
            perror("webserver (getsockname)");
            continue;
        }

        //start reading from socket
        int valread = read(newsockfd,buffer,BUFFER_SIZE);
        if(valread<0){
            perror("webserver (read)");
            continue;
        }
        
        //read request
        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", method, uri, version);
        
        //print data
        printf("[%s:%u] method:%s version:%s uri:%s\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),method,version,uri);

        //write to socket
        int valwrite = write(newsockfd,resp,strlen(resp));
        if(valwrite<0){
            perror("webserver (write)");
            continue;
        }

        close(newsockfd);
    }

    return 0;
}
