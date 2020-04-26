#include "queue.hh"
#include <pthread.h>
#include <stdlib.h>

// Initialize a new queue
void queue_init(my_queue_t *queue)
{
  pthread_mutex_init(&queue->lock, NULL);
  pthread_mutex_lock(&queue->lock);
  queue->size = 0;
  queue->last = NULL;
  pthread_mutex_unlock(&queue->lock);
}

// Destroy a queue
void queue_destroy(my_queue_t *queue)
{
  pthread_mutex_lock(&queue->lock);
  node_t *cur = queue->last;
  while (cur != NULL)
  {
    node_t *next = cur->next;
    free(cur);
    cur = next;
  }
  pthread_mutex_unlock(&queue->lock);
}

// Put an element at the end of a queue
void queue_put(my_queue_t *queue, int element)
{
  node_t *newNode = (node_t *)malloc(sizeof(node_t));
  newNode->data = element;
  newNode->next = NULL;

  pthread_mutex_lock(&queue->lock);
  node_t *cur = queue->last;

  if (cur == NULL)
  {
    queue->last = newNode;
  }
  else
  {
    node_t *trailCur = queue->last;
    while (cur != NULL)
    {
      trailCur = cur;
      cur = cur->next;
    }
    trailCur->next = newNode;
  }
  queue->size++;
  pthread_mutex_unlock(&queue->lock);
}

// Check if a queue is empty
bool queue_empty(my_queue_t *queue)
{
  pthread_mutex_lock(&queue->lock);
  bool ret = false;
  if (queue->size == 0)
  {
    ret = true;
  }
  pthread_mutex_unlock(&queue->lock);
  return ret;
}

// Take an element off the front of a queue
int queue_take(my_queue_t *queue)
{
  int ret;
  pthread_mutex_lock(&queue->lock);
  if (queue->size == 0)
  {
    ret = -1;
  }
  else
  {
    node_t *newLast = queue->last->next;
    ret = queue->last->data;
    free(queue->last);
    queue->last = newLast;
    queue->size--;
  }
  pthread_mutex_unlock(&queue->lock);
  return ret;
}
