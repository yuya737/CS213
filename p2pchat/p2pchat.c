#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
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
// https://www.linuxquestions.org/questions/programming-9/how-to-check-if-a-socket-is-valid-in-c-494523/

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
int socketID;
bool central;
bool pipeClosed = false;

void sigpipe_handler() {
  printf("SIGPIPE caught\n");
  pipeClosed = true;
}

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

void delete (int socket) {
  // pthread_mutex_lock(&listLock);
  node_t *cur = list;
  node_t *trailNode;

  // If the node of interest is the head then delete the head and return
  if (cur != NULL && cur->socket == socket) {
    list = list->next;
    free(cur);
    // pthread_mutex_unlock(&listLock);
    return;
  }

  while (cur != NULL) {
    if (cur->socket == socket) {
      trailNode->next = cur->next;
      free(cur);
      // pthread_mutex_unlock(&listLock);
      return;
    }
    trailNode = cur;
    cur = cur->next;
  }
  // pthread_mutex_unlock(&listLock);
  return;
}

void destroy() {
  pthread_mutex_lock(&listLock);
  if (list != NULL) {
    node_t *cur = list;
    while (cur != NULL) {
      node_t *trailNode = cur;
      close(cur->socket);
      cur = cur->next;
      free(trailNode);
    }
  }
  pthread_mutex_unlock(&listLock);
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

void broadcastMessage(int socketFrom, const char *username,
                      const char *message) {
  node_t *cur = list;
  while (cur != NULL) {
    int socket = cur->socket;
    if (socket == socketFrom) {
      cur = cur->next;
      continue;
    }
    data_t dataSize;
    dataSize.usernameLength = (int)strlen(username);
    dataSize.messageLength = (int)strlen(message);

    int writeError = write(socket, &dataSize, sizeof(data_t));
    if (writeError < 0) {
      perror("write datasize error to broadcast");
    }

    writeError = write(socket, username, (int)strlen(username));
    if (writeError < 0) {
      perror("write username error to broadcast");
    }

    writeError = write(socket, message, (int)strlen(message));
    if (writeError < 0) {
      perror("write message error to broadcast");
    }
    cur = cur->next;
  }
}

void *thread_ReceiveBroadcast(void *args) {
  while (1) {
    data_t dataSize;
    int readError = read(centralSocketID, &dataSize, sizeof(data_t));

    if (readError < 0) {
      perror("read dataSize error in broadcast thread");
    }

    char username[dataSize.usernameLength + 1];
    char message[dataSize.messageLength + 1];

    readError = read(centralSocketID, username, dataSize.usernameLength);
    if (readError < 0) {
      perror("read username error in broadcast thread");
    }

    readError = read(centralSocketID, message, dataSize.usernameLength);
    if (readError < 0) {
      perror("read username error in broadcast thread");
    }
    username[dataSize.usernameLength] = '\0';
    message[dataSize.messageLength] = '\0';

    ui_display(username, message);
  }
  return NULL;
}

/**
 * @brief  Function that runs on the central per to receive message from other
 * peers
 * @note
 * @param  *arg:
 * @retval None
 */
void *thread_RecieveAndDisplay(void *arg) {
  while (1) {
    pthread_mutex_lock(&listLock);
    if (list == NULL) {
      pthread_mutex_unlock(&listLock);
      continue;
    }

    // initiaize fields needed to listen and detect username/message combination
    fd_set readFDs;
    FD_ZERO(&readFDs);

    int maxFD = list->socket;
    node_t *cur = list;

    // traverse through the linked list add to FD_SET to check for input
    while (cur != NULL) {
      int socket = cur->socket;
      if (socket > maxFD)
        maxFD = socket;
      FD_SET(socket, &readFDs);
      cur = cur->next;
    }

    // lock the list and call select to check for input set timeout to 0 so it
    // doesn't block
    struct timeval time;
    time.tv_sec = 0;
    time.tv_usec = 0;
    int rc = select(maxFD + 1, &readFDs, NULL, NULL, &time);
    if (rc <= 0) {
      pthread_mutex_unlock(&listLock);
      continue;
    }

    int fd;
    cur = list;

    while (cur != NULL) {

      // traverse the linked list again to check which fd has input available
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

        if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
          node_t *temp = cur->next;
          delete (cur->socket);
          pthread_mutex_unlock(&listLock);
          cur = temp;
          continue;
        }

        ui_display(username, message);
        broadcastMessage(cur->socket, username, message);
      }
      cur = cur->next;
      pthread_mutex_unlock(&listLock);
    }
  }
}

// This function is run whenever the user hits enter after typing a message
void input_callback(const char *message) {
  if (!central) {

    data_t dataSize;

    dataSize.usernameLength = (int)strlen(username);
    dataSize.messageLength = (int)strlen(message);

    int i = write(centralSocketID, &dataSize, sizeof(data_t));
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

  } else {
    broadcastMessage(centralSocketID, username, message);
  }
  if (strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  } else {
    ui_display(username, message);
  }
}

int main(int argc, char **argv) {

  // signal(SIGPIPE, sigpipe_handler);

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

  int socket_fd;

  // Did the user specify a peer we should connect to?
  if (argc == 4) {
    // Unpack arguments
    char *peer_hostname = argv[2];
    unsigned short peer_port = atoi(argv[3]);

    central = false;

    // TODO: Connect to another peer in the chat network

    socket_fd = socket_connect(peer_hostname, peer_port);
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

  pthread_t thread;

  if (central) {
    if (pthread_create(&thread, NULL, thread_RecieveAndDisplay, NULL)) {
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
  } else {
    if (pthread_create(&thread, NULL, thread_ReceiveBroadcast, NULL)) {
      perror("pthread create failed");
      exit(EXIT_FAILURE);
    }
  }

  // Run the UI loop. This function only returns once we call ui_stop()
  // somewhere in the program.
  ui_run();

  if (!central)
    close(socket_fd);
  else
    close(centralSocketID);

  close(server_socket_fd);
  pthread_kill(thread, 0);

  destroy();
  return 0;
}
