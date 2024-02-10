#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_BUFFER_SIZE 256

typedef struct {
    int clientfd;
} ThreadArg;

void error(const char* msg) {
    perror(msg);
    exit(1);
}

void* handleClient(void* arg) {
    ThreadArg* threadArg = (ThreadArg*)arg;
    int clientfd = threadArg->clientfd;
    char buffer[MAX_BUFFER_SIZE];

    int n = read(clientfd, buffer, MAX_BUFFER_SIZE - 1);
    if (n < 0) {
        error("Error reading from socket");
    }

    buffer[n] = '\0';

    if (strstr(buffer, "END") == NULL) {
        close(clientfd);
        free(threadArg);
        pthread_exit(NULL);
    }

  

    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    int portno; 
    struct sockaddr_in server_sockaddr, client_sockaddr;
    socklen_t client_socklen = sizeof(client_sockaddr);
    bzero((char*)&server_sockaddr, sizeof(server_sockaddr));
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = INADDR_ANY;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    portno = atoi(argv[1]);


    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0) {
        error("Error at socket creation");
    }

    int option = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));


    server_sockaddr.sin_port = htons(portno);
    if (bind(serverfd, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) < 0) {
        error("Error at binding to port");
    }


    if (listen(serverfd, 5) < 0) {
        error("Error at listening");
    }


    while (1) {
        int acceptfd = accept(serverfd, (struct sockaddr*)&client_sockaddr, &client_socklen);
        if (acceptfd < 0) {
            error("Error at accepting connection");
        }

        ThreadArg* threadArg = (ThreadArg*)malloc(sizeof(ThreadArg));
        threadArg->clientfd = acceptfd;

        pthread_t new_client;
        printf("Creating new thread\n");

        pthread_create(&new_client, NULL, handleClient, (void*)threadArg);
        // pthread_join(new_client, NULL);
        printf("After joining back\n");
    }

    return 0;
}
