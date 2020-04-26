#include "stack.hh"
#include <pthread.h>
#include <stdlib.h>

// Initialize a stack
void stack_init(my_stack_t *stack) {
  pthread_mutex_init(&stack->lock, NULL);
  pthread_mutex_lock(&stack->lock);
  stack->size = 0;
  stack->top = NULL;
  pthread_mutex_unlock(&stack->lock);
}

// Destroy a stack
void stack_destroy(my_stack_t *stack) {
  pthread_mutex_lock(&stack->lock);
  node_t *cur = stack->top;
  while (cur != NULL) {
    node_t *next = cur->next;
    free(cur);
    cur = next;
  }
  pthread_mutex_unlock(&stack->lock);
}

// Push an element onto a stack
void stack_push(my_stack_t *stack, int element) {
  node_t *newNode = (node_t *)malloc(sizeof(node_t));
  newNode->data = element;

  pthread_mutex_lock(&stack->lock);
  newNode->next = stack->top;
  stack->top = newNode;
  stack->size++;
  pthread_mutex_unlock(&stack->lock);
}

// Check if a stack is empty
bool stack_empty(my_stack_t *stack) {
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
