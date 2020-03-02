#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define QUEUE_END -1

// This struct is where you should define your queue datatype. You may need to
// add another struct (e.g. a node struct) depending on how you choose to
// implement your queue.
typedef struct node {
    int data;
    struct node* next;
} node_t;


typedef struct queue {
    node_t* head;
} queue_t;


/**
 * Initialize a queue pointed to by the parameter q.
 * \param q  This points to allocated memory that should be initialized to an
 *           empty queue.
 */
void queue_init(queue_t* q) {
    q->head = NULL;
}

/**
 * Add a value to a queue.
 * \param q       Points to a queue that has been initialized by queue_init.
 * \param value   The integer value to add to the queue
 */
void queue_put(queue_t* q, int value) {
    // If q->head is null, make a newNode set to head
    if (q->head == NULL){
        // Malloc data for newNode
        node_t* newNode = (node_t*) malloc(sizeof(node_t));
        newNode->data = value;
        newNode->next = NULL;
        q->head = newNode;
    } else {
        // Malloc data for newNode
        node_t* newNode = (node_t*) malloc(sizeof(node_t));
        node_t* current = q->head;
        // Move to end of linked list 
        while (current->next != NULL){
            current = current->next;
        }
        // Add data to newNode and the set to newNode
        newNode->data = value;
        newNode->next = NULL;
        current->next = newNode;
    }
}

/**
 * Check if a queue is empty.
 * \param q   Points to a queue initialized by queue_init.
 * \returns   True if the queue is empty, otherwise false.
 */
bool queue_empty(queue_t* q) {
    if (q->head == NULL){
        return true;
    } else {
        return false;
    }
}

/**
 * Take a value from a queue.
 * \param q   Points to a queue initialized by queue_init.
 * \returns   The value that has been in the queue the longest time. If the
 *            queue is empty, return QUEUE_END.
 */
int queue_take(queue_t* q) {
    // If head is null return QUEUE_END
    if (q->head == NULL){
        return QUEUE_END;
    } else {
        // Store the node to be removed and the next head
        node_t* toBeRemoved = q->head;
        node_t* nextHead = q->head->next;
        // Set the new head to nextHead
        q->head = nextHead;
        // Save the data and free the node to be removed
        int ret = toBeRemoved->data;
        free(toBeRemoved);
        return ret;
    }
}

/**
 * Free any memory allocated inside the queue data structure.
 * \param q   Points to a queue initialized by queue_init. The memory referenced
 *            by q should *NOT* be freed.
 */
void queue_destroy(queue_t* q) {
    node_t* current = q->head;
    if (current == NULL){
        return;
    } else {
        // Set followPtr to current
        node_t* followPtr = current;
        // Iterate the linkedlist to the end
        while (current != NULL){
            // Free each node till the end
            current = current->next;
            free(followPtr);
            followPtr = current;
        }
    }
}

int main(int argc, char** argv) {
    // Set up and initialize a queue
    queue_t q;
    queue_init(&q);

    // Read lines until the end of stdin
    char* line = NULL;
    size_t line_size = 0;
    while(getline(&line, &line_size, stdin) != EOF) {
        int num;

        // If the line has a take command, take a value from the queue
        if(strcmp(line, "take\n") == 0) {
            if(queue_empty(&q)) {
                printf("The queue is empty.\n");
            } else {
                printf("%d\n", queue_take(&q));
            }
        } else if(sscanf(line, "put %d\n", &num) == 1) {
            queue_put(&q, num);
        } else {
            printf("unrecognized command.\n");
        }
    }

    // Free the space allocated by getline
    free(line);

    // Clean up the queue
    queue_destroy(&q);

    return 0;
}
