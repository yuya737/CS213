#include "dict.hh"

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Code and function for reader-writer lock from Operating Systems: Three Easy Pieces by Remzi H. Arpaci-Dusseau and Andrea C. Arpaci-Dusseau

void rwlock_init(rwlock_t *rw) {
  rw->readers = 0;
  sem_init(&rw->lock, 0, 1);
  sem_init(&rw->writelock, 0, 1);
}

void rwlock_acquire_readlock(rwlock_t *rw) {
  sem_wait(&rw->lock);
  rw->readers++;
  if (rw->readers == 1)
    sem_wait(&rw->writelock);
  sem_post(&rw->lock);
}

void rwlock_release_readlock(rwlock_t *rw) {
  sem_wait(&rw->lock);
  rw->readers--;
  if (rw->readers == 0)
    sem_post(&rw->writelock);
  sem_post(&rw->lock);
}

void rwlock_acquire_writelock(rwlock_t *rw) { sem_wait(&rw->writelock); }

void rwlock_release_writelock(rwlock_t *rw) { sem_post(&rw->writelock); }

/**
 * @brief  Initialize the dictionary
 * @note   
 * @param  *dict: dictionary to initialize
 * @retval None
 */
void dict_init(my_dict_t *dict) {
  // set the root to NULL and intialize the reader-writer lock
  dict->root = NULL;
  rwlock_init(&dict->rwlock);
}

/**
 * @brief  Helper function to destroy a dictionary
 * @note   
 * @param  *node: pointer to the root node of the dictionary
 * @retval pointer to a new node - used for recursion
 */
node_t *dict_destroy_helper(node_t *node) {
  // if the node is null, return this node
  if (node == NULL)
    return node;
  else {
    // call this function recursively for each subtree
    if (node->right != NULL)
      node->right = dict_destroy_helper(node->right);
    if (node->left != NULL)
      node->left = dict_destroy_helper(node->left);
    // free current node
    free(node);
  }
  return node;
}

/**
 * @brief  Destroy the dictionary 
 * @note   
 * @param  *dict: dictionary to destroy
 * @retval None
 */
void dict_destroy(my_dict_t *dict) {
  // acquire the writelock, destroy and release the writelock
  rwlock_acquire_writelock(&dict->rwlock);
  dict->root = dict_destroy_helper(dict->root);
  rwlock_release_writelock(&dict->rwlock);
}

/**
 * @brief  Make new node with given key and value
 * @note  
 * @param  *key: Key for the new node
 * @param  value:  Value for the new node
 * @retval pointer to the new node
 */
node_t *makeNode(const char *key, int value) {
  // allocate space for the new node and assign the right values
  node_t *newNode = (node_t *)malloc(sizeof(node_t));
  newNode->key = key;
  newNode->value = value;
  newNode->left = NULL;
  newNode->right = NULL;
  return newNode;
}

/**
 * @brief  Helper function to add a value to a dictionary
 * @note Citation: https://www.geeksforgeeks.org/binary-search-tree-set-1-search-and-insertion/
 * @param  *node: node to which the new entry is to be added
 * @param  *key: key of the new entry
 * @param  value: value of the new entry
 * @retval pointer to a new node - used for recursion
 */
node_t *dict_set_helper(node_t *node, const char *key, int value) {
  if (node == NULL) {
    // If the current node is null, then make a new node at the current position.
    node_t *newNode = makeNode(key, value);
    return newNode;
  } else {
    // compare the current key with the new entry and recursively call this function on the appropriate subtree
    if (strcmp(key, node->key) == 0) {
      node->value = value;
    } else if (strcmp(key, node->key) < 0) {
      node->left = dict_set_helper(node->left, key, value);
    } else if (strcmp(key, node->key) > 0) {
      node->right = dict_set_helper(node->right, key, value);
    } else {
      perror("Issue in dict_set_helper");
    }
    return node;
  }
}

/**
 * @brief  Add a new entry to the dictionary
 * @note   
 * @param  *dict: pointer to the dictionary the entry is to be added
 * @param  *key: key of the new entry
 * @param  value: value of the new entry
 * @retval None
 */
void dict_set(my_dict_t *dict, const char *key, int value) {
  // acquire the writelock, add the entry, and release the writelock
  rwlock_acquire_writelock(&dict->rwlock);
  dict->root = dict_set_helper(dict->root, key, value);
  rwlock_release_writelock(&dict->rwlock);
}

/**
 * @brief  Helper function to check if an entry in present
 * @note   
 * @param  *node: root node for the dictionary
 * @param  *key: key to check the existence
 * @retval 
 */
bool dict_contains_helper(node_t *node, const char *key) {
  // if the node is null, the dictionary doesn't have the value
  if (node == NULL) {
    return false;
  } else {
    // compare the key with the interested key and run this function recursively on the appropriate subtree
    if (strcmp(key, node->key) == 0) {
      return true;
    } else if (strcmp(key, node->key) < 0) {
      return dict_contains_helper(node->left, key);
    } else if (strcmp(key, node->key) > 0) {
      return dict_contains_helper(node->right, key);
    }
  }
  perror("Reaching invalid line in dict_contains_helper");
  return false;
}
/**
 * @brief  Function to check if the given key is in the dictionary
 * @note   
 * @param  *dict: dictionary to check
 * @param  *key: key to check the existence of
 * @retval boolean
 */
bool dict_contains(my_dict_t *dict, const char *key) {
  // acquire the readlock, find whether or not the value is present and release the lock
  rwlock_acquire_readlock(&dict->rwlock);
  bool ret = dict_contains_helper(dict->root, key);
  rwlock_release_readlock(&dict->rwlock);
  return ret;
}

/**
 * @brief  Helper function to retrieve the value of an entry
 * @note   
 * @param  *node: root node of the dictionary
 * @param  *key: key of the interested entry
 * @retval the value of the corresponding key, if not present -1
 */
int dict_get_helper(node_t *node, const char *key) {
  // if the current node is null, then there isn't such entry
  if (node == NULL) {
    return -1;
  } else {
    // compare the key and run this function on the appropriate subtree
    if (strcmp(key, node->key) == 0) {
      return node->value;
    } else if (strcmp(key, node->key) < 0) {
      return dict_get_helper(node->left, key);
    } else if (strcmp(key, node->key) > 0) {
      return dict_get_helper(node->right, key);
    } else {
    }
  }
  perror("Reaching invalid line in dict_get_helper");
  return -1;
}

/**
 * @brief  Function to get the value for a given key
 * @note   
 * @param  *dict: dictionary from which the value is retreived
 * @param  *key: key of the interested entry
 * @retval the value of the corresponding key, if not present -1
 */
int dict_get(my_dict_t *dict, const char *key) {
  // acquire the readlock, get the value for the desired key, and release the readlock
  rwlock_acquire_readlock(&dict->rwlock);
  int ret = dict_get_helper(dict->root, key);
  rwlock_release_readlock(&dict->rwlock);
  return ret;
}

/**
 * @brief  Find the next smallest node in the BST
 * @note Citation: https://www.geeksforgeeks.org/binary-search-tree-set-2-delete/
 * @param  *node: node from which to find the next smallest node
 * @retval pointer to the next smallest node 
 */
node_t *findMinValNode(node_t *node) {

  node_t *cur = node;
  while (cur && cur->left != NULL) {
    cur = cur->left;
  }
  return cur;
}

/**
 * @brief  Helper function to remove an entry from a dictionary
 * @note Citation: https://www.geeksforgeeks.org/binary-search-tree-set-2-delete/
 * @param  *node: root node of the dictionary from which to remove an entry
 * @param  *key: key of the entry to be removed
 * @retval new head node after removal
 */
node_t *dict_remove_helper(node_t *node, const char *key) {
  if (node == NULL) {
    return node;
  } else {
    // compare the key and the current key and run this function recursively on the appropriate subtree
    if (strcmp(key, node->key) < 0) {
      node->left = dict_remove_helper(node->left, key);
    } else if (strcmp(key, node->key) > 0) {
      node->right = dict_remove_helper(node->right, key);
    } else {
      // we are at the node to remove. if either left or right is null, remove the current node and replace it with its child
      if (node->left == NULL) {
        node_t *temp = node->right;
        free(node);
        return temp;
      } else if (node->right == NULL) {
        node_t *temp = node->left;
        free(node);
        return temp;
      } else {
        // If both children aren't null, then find the next smallest child, replace the entries and remove that node
        node_t *minValNode = findMinValNode(node->right);
        node->key = minValNode->key;
        node->value = minValNode->value;
        node->right = dict_remove_helper(node->right, minValNode->key);
      }
    }
  }
  return node;
}

/**
 * @brief  Function to remove an entry
 * @note   
 * @param  *dict: dictionary from which to remove an entry
 * @param  *key: key of the entry to remove
 * @retval None
 */
void dict_remove(my_dict_t *dict, const char *key) {
  // acquire the writelock, remove the entry, and release the key. 
  rwlock_acquire_writelock(&dict->rwlock);
  dict->root = dict_remove_helper(dict->root, key);
  rwlock_release_writelock(&dict->rwlock);
}
