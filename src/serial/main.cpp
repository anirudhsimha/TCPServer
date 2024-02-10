#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unordered_map>

using namespace std;

unordered_map<string, string> mymap;

void error(const char* msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char** argv) {
    int portno; /* port to listen on */
    struct sockaddr_in server_sockaddr, client_sockaddr;
    socklen_t client_socklen = sizeof(client_sockaddr);
    bzero((char*)&server_sockaddr, sizeof(server_sockaddr));
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = INADDR_ANY;

    /* check command line arguments */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Server port number taken as a command line argument
    portno = atoi(argv[1]);

    // Create a Socket for the server
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0) {
        error("Error at socket creation");
    }

    int option = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // Bind the created socket to a port
    server_sockaddr.sin_port = htons(portno);
    if (bind(serverfd, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) < 0) {
        error("Error at binding to port");
    }

    // Listen for connections from some remote client
    if (listen(serverfd, 5) < 0) {
        error("Error at listening");
    }

    // Accept connection from a client
    int acceptfd = accept(serverfd, (struct sockaddr*)&client_sockaddr, &client_socklen);
    if (acceptfd < 0) {
        error("Error at accepting connection");
    }

    char buffer[256];

    int n = read(acceptfd, buffer, 255);

    stringstream inputstr;
    inputstr << string(buffer);

    string input_command;
    string input_key;
    string input_value;

    while (1) {
        inputstr >> input_command;

        if (input_command == "WRITE") {
            inputstr >> input_key;
            inputstr >> input_value;

            printf("Input key is on write %s and input_value be %s\n", input_key.c_str(), input_value.c_str());
            input_value = input_value.substr(1, input_value.size() - 1);
            mymap[input_key] = input_value;
            write(acceptfd, "FIN\n", 4);
        } else if (input_command == "READ") {
            inputstr >> input_key;
            printf("Input key is on read %s\n", input_key.c_str());
            if (mymap.find(input_key) == mymap.end())
                write(acceptfd, "NULL\n", 5);
            else {
                write(acceptfd, (mymap[input_key] + "\n").c_str(), mymap[input_key].size() + 1);
            }
        } else if (input_command == "COUNT") {
            string size_str = to_string(mymap.size());
            write(acceptfd, (size_str + "\n").c_str(), size_str.size() + 1);
            write(acceptfd, "\n", 1);
        } else if (input_command == "DELETE") {
            inputstr >> input_key;
            printf("Input key is on delete %s\n", input_key.c_str());
            if (mymap.erase(input_key))
                write(acceptfd, "FIN\n", 4);
            else {
                write(acceptfd, "NULL\n", 5);
            }
        } else if (input_command == "END") {
            break;
        } else {
            error("Invalid Input");
        }
    }

    close(acceptfd);
    close(serverfd);

    return 0;
}
