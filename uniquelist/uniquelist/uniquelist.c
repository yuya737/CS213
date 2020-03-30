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
  node_t *current = s->root;
  node_t *prevNode;

  // Traverse through linked list, keeping a pointer to the previous node and freeing appropriately
  while (current != NULL)
  {
    prevNode = current;
    current = current->next;
    free(prevNode);
  }
}

/// Add an element to a uniquelist, unless it's already in the uniquelist
void uniquelist_insert(uniquelist_t *s, int n)
{
  if (s->root == NULL) // If the root is NULL, we have an empty linkedlist so make a new entry
  {
    s->root = (node_t *)malloc(sizeof(node_t));
    s->root->data = n;
    s->root->next = NULL;
  }
  else
  {
    node_t *current = s->root;

    while (current->data != n) // Traverse until we find an entry with the same number
    {
      if (current->next == NULL) // If we reach the end of the list, we add an entry at the end
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
    return; // If we find a matching entry, return
  }
}

/// Print all the numbers in a uniquelist
void uniquelist_print_all(uniquelist_t *s)
{
  node_t *current = s->root;

  // With the copy of the head in hand, print the uniquelist by travsering to the end
  while (current != NULL)
  {
    if (current->next == NULL){ // If we are the last element, print the element and the newline character
      printf("%d\n", current->data);
      current = current->next;
    } else{ // Otherwise, print a space after the element.
      printf("%d ", current->data);
      current = current->next;
    }
  }
}
