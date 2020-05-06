#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "socket.h"
#include "ui.h"

// Citations:
// https://www.linuxtoday.com/blog/blocking-and-non-blocking-i-0.html

// Max number of characters to send/receive
#define MAXCHARS 256
pthread_mutex_t lock;
pthread_mutex_t listLock;

// Struct to hold socket informaton
typedef struct node {
  int socket;
  struct node *next;
} node_t;

// Keep the username in a global so we can access it from the callback
const char *username;
node_t *list = NULL;
int centralSocketID;
bool central;

void add(int socket) {
  pthread_mutex_lock(&listLock);
  node_t *newNode = (node_t *)malloc(sizeof(node_t));
  newNode->socket = socket;
  newNode->next = NULL;
  if (list == NULL) {
    list = newNode;
  } else {
    newNode->next = list;
    list = newNode;
  }
  pthread_mutex_unlock(&listLock);
}

void destroy() {
  if (list != NULL) {
    node_t *cur = list;
    while (cur != NULL) {
      node_t *trailNode = cur;
      cur = trailNode->next;
      free(trailNode);
    }
  }
}

void *acceptConnection(void *server_socket_fd) {
  while (1) {
    int client_socket_fd = server_socket_accept(*((int *)server_socket_fd));
    if (client_socket_fd == -1) {
      perror("accept failed");
      exit(2);
    }
    ui_display(username, "Client connected");
    // fcntl(client_socket_fd, F_SETFL, fcntl(client_socket_fd, F_GETFL) |
    // O_NONBLOCK);
    add(client_socket_fd);
  }
}

typedef struct dataSize {
  int usernameLength;
  int messageLength;
} data_t;

typedef struct arguments {
  const char *message;
  int centralSocketId;
} arg_t;

void *thread_RecieveDisplay(void *arg) {
  arg_t *arguments = (arg_t *)arg;
  int i = 0;
  while (1) {
    if (list == NULL) {
      continue;
    }
    fd_set readFDs;
    FD_ZERO(&readFDs);
    int maxFD = list->socket;
    node_t *cur = list;
    pthread_mutex_lock(&listLock);
    while (cur != NULL) {
      int socket = cur->socket;
      if (socket > maxFD)
        maxFD = socket;
      FD_SET(socket, &readFDs);
      cur = cur->next;
    }
    pthread_mutex_unlock(&listLock);
    int rc = select(maxFD + 1, &readFDs, NULL, NULL, NULL);

    int fd;
    cur = list;
    pthread_mutex_lock(&listLock);
    while (cur != NULL) {
      if (FD_ISSET(cur->socket, &readFDs)) {
        data_t dataSize;
        int readError = read(cur->socket, &dataSize, sizeof(data_t));
        if (readError < 0) {
          perror("dataSize read error");
        }
        char username[dataSize.usernameLength + 1];
        char message[dataSize.messageLength + 1];

        readError = read(cur->socket, username, dataSize.usernameLength);
        if (readError < 0) {
          perror("username read error");
        }

        readError = read(cur->socket, message, dataSize.messageLength);
        if (readError < 0) {
          perror("message read error");
        }

        username[dataSize.usernameLength] = '\0';
        message[dataSize.messageLength] = '\0';
        ui_display(username, message);
      }
      cur = cur->next;
    }
    pthread_mutex_unlock(&listLock);
  }
}

// This function is run whenever the user hits enter after typing a message
void input_callback(const char *message) {
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  } else {
    ui_display(username, message);
  }
  if (!central) {
    int i = pthread_mutex_lock(&lock);
    if (i < 0) {
      perror("locking error");
    }
    data_t dataSize;
    // printf("%s, %s\n", username, message);
    // printf("%d, %d\n", (int)strlen(username), (int)strlen(message));
    dataSize.usernameLength = (int)strlen(username);
    dataSize.messageLength = (int)strlen(message);

    i = write(centralSocketID, &dataSize, sizeof(data_t));
    if (i < 0) {
      perror("write datasize error");
    }

    i = write(centralSocketID, username, (int)strlen(username));
    if (i < 0) {
      perror("write username error");
    }
    i = write(centralSocketID, message, (int)strlen(message));
    if (i < 0) {
      perror("write message error");
    }
    i = pthread_mutex_unlock(&lock);
    if (i < 0) {
      perror("unlocking error");
    }
  }
}

int main(int argc, char **argv) {
  // Make sure the arguments include a username
  if (argc != 2 && argc != 4) {
    fprintf(stderr, "Usage: %s <username> [<peer> <port number>]\n", argv[0]);
    exit(1);
  }

  if (pthread_mutex_init(&lock, NULL) != 0) {
    printf("\n mutex init failed\n");
    return 1;
  }

  if (pthread_mutex_init(&listLock, NULL) != 0) {
    printf("\n mutex init failed\n");
    return 1;
  }

  // Save the username in a global
  username = argv[1];

  // TODO: Set up a server socket to accept incoming connections

  unsigned short port = 0;
  int server_socket_fd = server_socket_open(&port);
  if (server_socket_fd == -1) {
    perror("Server socket was not opened");
    exit(2);
  }

  if (listen(server_socket_fd, 1)) {
    perror("listen failed");
    exit(2);
  }

  printf("listening on port %u\n", port);

  // Did the user specify a peer we should connect to?
  if (argc == 4) {
    // Unpack arguments
    char *peer_hostname = argv[2];
    unsigned short peer_port = atoi(argv[3]);

    central = false;

    // TODO: Connect to another peer in the chat network

    int socket_fd = socket_connect(peer_hostname, peer_port);
    if (socket_fd == -1) {
      perror("Failed to connect");
      exit(2);
    }
    centralSocketID = socket_fd;
  } else {
    // This is the central peer
    central = true;
    pthread_t thread;
    if (pthread_create(&thread, NULL, acceptConnection,
                       (void *)&server_socket_fd)) {
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
  }

  // Set up the user interface. The input_callback function will be called
  // each time the user hits enter to send a message.
  ui_init(input_callback);

  // Once the UI is running, you can use it to display log messages
  ui_display("INFO", "This is a handy log message.");

  char temp[MAXCHARS];
  sprintf(temp, "listening on port %u\n", port);
  ui_display(temp, "");

  if (central) {
    pthread_t thread1;
    if (pthread_create(&thread1, NULL, thread_RecieveDisplay, NULL)) {
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
  }

  // Run the UI loop. This function only returns once we call ui_stop()
  // somewhere in the program.
  ui_run();

  destroy();
  return 0;
}
