#ifndef DICT_H
#define DICT_H

#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct rwlock_t {
  sem_t lock;
  sem_t writelock;
  int readers;
} rwlock_t;


typedef struct my_node {
  const char* key;
  int value;
  struct my_node* left;
  struct my_node* right;
} node_t;

typedef struct my_dict {
  node_t* root;
  rwlock_t rwlock;
} my_dict_t;

// Initialize a dictionary
void dict_init(my_dict_t* dict);

// Destroy a dictionary
void dict_destroy(my_dict_t* dict);

// Set a value in a dictionary
void dict_set(my_dict_t* dict, const char* key, int value);

// Check if a dictionary contains a key
bool dict_contains(my_dict_t* dict, const char* key);

// Get a value in a dictionary
int dict_get(my_dict_t* dict, const char* key);

// Remove a value from a dictionary
void dict_remove(my_dict_t* dict, const char* key);

#endif
