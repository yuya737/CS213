#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "socket.h"
#include "ui.h"


// Struct to hold socket informaton
typedef struct node{
    int socket;
    struct node* next;
} node_t;

typedef struct list{
    node_t* head;
} list_t;

// Keep the username in a global so we can access it from the callback
const char* username;
list_t* list = NULL;
bool central;

void add(int socket){
    node_t* newNode = (node_t*) malloc(sizeof(node_t));
    newNode->socket = socket;
    if (list == NULL){
        list->head = newNode;
    } else {
        node_t* cur = list->head;
        while (cur!=NULL){
            cur = cur->next;
        }
        cur = newNode;
    }
}

void destroy(){
    if (list != NULL){
        node_t* cur = list->head;
        while (cur != NULL){
            node_t* trailNode = cur;
            free(cur);
            cur = trailNode->next;
        }
    }
}

// This function is run whenever the user hits enter after typing a message
void input_callback(const char* message) {
    if(strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
        ui_exit();
    } else {
        ui_display(username, message);
    }
}

int main(int argc, char** argv) {
    // Make sure the arguments include a username
    if(argc != 2 && argc != 4) {
        fprintf(stderr, "Usage: %s <username> [<peer> <port number>]\n", argv[0]);
        exit(1);
    }

    // Save the username in a global
    username = argv[1];

    // TODO: Set up a server socket to accept incoming connections

    unsigned short port = 0;
    int server_socket_fd = server_socket_open(&port);
    if (server_socket_fd == -1){
        perror("Server socket was not opened");
        exit(2);
    }

    if (listen(server_socket_fd, 1)){
        perror("listen failed");
        exit(2);
    }

    printf("listening on port %u\n", port);



    // Did the user specify a peer we should connect to?
    if(argc == 4) {
        // Unpack arguments
        char* peer_hostname = argv[2];
        unsigned short peer_port = atoi(argv[3]);

        central = false;

        // TODO: Connect to another peer in the chat network

        int socket_fd = socket_connect(peer_hostname, peer_short);
        if (socket_fd == -1){
            perror("Failed to connect");
            exit(2);
        }
    } else {
        // This is the central peer
        central = true;
        int client_socker_fd =  server_socket_accept(server_socker_fd);
        if (client_socket_fd == -1){
            perror("accept failed");
            exit(2);
        }

    }

    // Set up the user interface. The input_callback function will be called
    // each time the user hits enter to send a message.
    ui_init(input_callback);

    // Once the UI is running, you can use it to display log messages
    ui_display("INFO", "This is a handy log message.");

    // Run the UI loop. This function only returns once we call ui_stop() somewhere in the program.
    ui_run();

    return 0;
}
