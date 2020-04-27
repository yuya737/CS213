#include "stack.hh"
#include <pthread.h>
#include <stdlib.h>

// Initialize a stack
void stack_init(my_stack_t *stack) {
    // Lock the mutex lock and initialize the size and top fields
    pthread_mutex_init(&stack->lock, NULL);
    pthread_mutex_lock(&stack->lock);
    stack->size = 0;
    stack->top = NULL;
    // unlock
    pthread_mutex_unlock(&stack->lock);
}

// Destroy a stack
void stack_destroy(my_stack_t *stack) {
    // Lock the mutex lock, traverse through the stack and destroy nodes
    pthread_mutex_lock(&stack->lock);
    node_t *cur = stack->top;
    while (cur != NULL) {
        node_t *next = cur->next;
        free(cur);
        cur = next;
    }
    // unlock
    pthread_mutex_unlock(&stack->lock);
}

// Push an element onto a stack
void stack_push(my_stack_t *stack, int element) {
    // Alocate space for a new node and assign values
    node_t *newNode = (node_t *)malloc(sizeof(node_t));
    newNode->data = element;
    // Lock the lock and add node to the head
    pthread_mutex_lock(&stack->lock);
    newNode->next = stack->top;
    stack->top = newNode;
    stack->size++;
    pthread_mutex_unlock(&stack->lock);
}

// Check if a stack is empty
bool stack_empty(my_stack_t *stack) {
    // Lock the lock and check if the size is 0 or not
    pthread_mutex_lock(&stack->lock);
    bool ret = false;
    if (stack->size == 0) {
        ret = true;
    }
    pthread_mutex_unlock(&stack->lock);
    return ret;
}

// Pop an element off of a stack
int stack_pop(my_stack_t *stack) {
    int ret;
    // Lock the lock and find the element to pop. Store the value to return and unlock.
    pthread_mutex_lock(&stack->lock);
    if (stack->size == 0) {
        ret = -1;
    } else {
        node_t *newHead = stack->top->next;
        ret = stack->top->data;
        free(stack->top);
        stack->top = newHead;
        stack->size--;
    }

    pthread_mutex_unlock(&stack->lock);
    return ret;
}
