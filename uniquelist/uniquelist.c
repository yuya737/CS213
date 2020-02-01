#include "uniquelist.h"

#include <stdio.h>
#include <stdlib.h>

/// Initialize a new uniquelist
void uniquelist_init(uniquelist_t *s)
{
  s->root = NULL;
}

/// Destroy a uniquelist
void uniquelist_destroy(uniquelist_t *s)
{
  // TODO: implement me
  node_t *current = s->root;
  node_t *next = s->root;

  while (next != NULL)
  {
    next = current->next;
    //free(current);
    current = next;
  }
  //free(s);
}

/// Add an element to a uniquelist, unless it's already in the uniquelist
void uniquelist_insert(uniquelist_t *s, int n)
{
  // TODO: implement me

  if (s->root == NULL)
  {
    s->root = (node_t *)malloc(sizeof(node_t));
    s->root->data = n;
    s->root->next = NULL;
  }
  else
  {
    node_t *current = s->root;

    while (current->data != n)
    {
      if (current->next == NULL)
      {
        node_t *newNode = (node_t *)malloc(sizeof(node_t));
        newNode->data = n;
        newNode->next = NULL;
        current->next = newNode;
        return;
      }
      else
      {
        current = current->next;
      }
    }
    return;
  }
}

/// Print all the numbers in a uniquelist
void uniquelist_print_all(uniquelist_t *s)
{
  // TODO: implement me
  node_t *current = s->root;

  while (current != NULL)
  {
    printf("%d ", current->data);
    current = current->next;
  }
  printf("\n");
}
