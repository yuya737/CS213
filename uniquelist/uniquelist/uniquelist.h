#ifndef UNIQUELIST_H
#define UNIQUELIST_H


/// Node for linked list
typedef struct node {
  int data;
  struct node* next;
} node_t;

/// Uniquelist stores the head of the linkedlist
typedef struct uniquelist {
  node_t* root;
} uniquelist_t;


/// Initialize a new uniquelist
void uniquelist_init(uniquelist_t* s);

/// Destroy a uniquelist
void uniquelist_destroy(uniquelist_t* s);

/// Add an element to a uniquelist, unless it's already in the uniquelist
void uniquelist_insert(uniquelist_t* s, int n);

/// Print all the numbers in a uniquelist
void uniquelist_print_all(uniquelist_t* s);

#endif
