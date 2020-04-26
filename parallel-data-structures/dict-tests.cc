#include <gtest/gtest.h>

#include "dict.hh"
#include <pthread.h>
#include <time.h>

/****** Dictionary Invariants ******/

// Invariant 1
// For every value V that has been put onto the dictionary p times and returned
// by take q times, there must be p-q copies of this value on the dictionary.
// This only holds if p >= q.

// Invariant 2
// Value for any key should not change, unless specified.

// Invariant 3
// There are no duplicate entires with the same key.

/****** Synchronization ******/

// Under what circumstances can accesses to your dictionary structure can
// proceed in parallel? Answer below.
//
// Any functions that read (contains, get) can run in parallel using a
// reader-writer lock
//

// When will two accesses to your dictionary structure be ordered by
// synchronization? Answer below.
//
// Any function that write(destroy, set, remove) will run in order since they
// cannot run in parallel using a reader-writer lock
//

/****** Begin Tests ******/

// A utility function to do inorder traversal of BST
void inorder(node_t *root) {
  if (root != NULL) {
    inorder(root->left);
    printf("Key: %s, value %d \n", root->key, root->value);
    inorder(root->right);
  }
}

// Basic functionality for the dictionary
TEST(DictionaryTest, BasicDictionaryOps) {
  my_dict_t d;
  dict_init(&d);

  // Make sure the dictionary does not contain keys A, B, and C
  ASSERT_FALSE(dict_contains(&d, "A"));
  ASSERT_FALSE(dict_contains(&d, "B"));
  ASSERT_FALSE(dict_contains(&d, "C"));

  // Add some values
  dict_set(&d, "A", 1);
  dict_set(&d, "B", 2);
  dict_set(&d, "C", 3);

  // Make sure these values are contained in the dictionary
  ASSERT_TRUE(dict_contains(&d, "A"));
  ASSERT_TRUE(dict_contains(&d, "B"));
  ASSERT_TRUE(dict_contains(&d, "C"));

  // Make sure these values are in the dictionary
  ASSERT_EQ(1, dict_get(&d, "A"));
  ASSERT_EQ(2, dict_get(&d, "B"));
  ASSERT_EQ(3, dict_get(&d, "C"));

  // Set some new values
  dict_set(&d, "A", 10);
  dict_set(&d, "B", 20);
  dict_set(&d, "C", 30);

  // Make sure these values are contained in the dictionary
  ASSERT_TRUE(dict_contains(&d, "A"));
  ASSERT_TRUE(dict_contains(&d, "B"));
  ASSERT_TRUE(dict_contains(&d, "C"));

  // Make sure the new values are in the dictionary
  ASSERT_EQ(10, dict_get(&d, "A"));
  ASSERT_EQ(20, dict_get(&d, "B"));
  ASSERT_EQ(30, dict_get(&d, "C"));

  // Remove the values
  dict_remove(&d, "A");
  dict_remove(&d, "B");
  dict_remove(&d, "C");

  // Make sure these values are not contained in the dictionary
  ASSERT_FALSE(dict_contains(&d, "A"));
  ASSERT_FALSE(dict_contains(&d, "B"));
  ASSERT_FALSE(dict_contains(&d, "C"));

  // Make sure we get -1 for each value
  ASSERT_EQ(-1, dict_get(&d, "A"));
  ASSERT_EQ(-1, dict_get(&d, "B"));
  ASSERT_EQ(-1, dict_get(&d, "C"));

  // Clean up
  dict_destroy(&d);
}

// Basic functionality for the dictionary
TEST(DictionaryTest, BasicDictionaryOps2) {
  my_dict_t d;
  dict_init(&d);

  // Make sure the dictionary does not contain keys A, B, and C
  ASSERT_FALSE(dict_contains(&d, "A"));
  ASSERT_FALSE(dict_contains(&d, "B"));
  ASSERT_FALSE(dict_contains(&d, "C"));

  // Add some values
  dict_set(&d, "A", 0);
  dict_set(&d, "B", 0);
  dict_set(&d, "C", 0);

  // Make sure these values are contained in the dictionary
  ASSERT_TRUE(dict_contains(&d, "A"));
  ASSERT_TRUE(dict_contains(&d, "B"));
  ASSERT_TRUE(dict_contains(&d, "C"));

  // Make sure these values are in the dictionary
  ASSERT_EQ(0, dict_get(&d, "A"));
  ASSERT_EQ(0, dict_get(&d, "B"));
  ASSERT_EQ(0, dict_get(&d, "C"));

  // Set some new values
  dict_set(&d, "A", 0);
  dict_set(&d, "B", 0);
  dict_set(&d, "C", 0);

  // Make sure these values are contained in the dictionary
  ASSERT_TRUE(dict_contains(&d, "A"));
  ASSERT_TRUE(dict_contains(&d, "B"));
  ASSERT_TRUE(dict_contains(&d, "C"));

  // Make sure the new values are in the dictionary
  ASSERT_EQ(0, dict_get(&d, "A"));
  ASSERT_EQ(0, dict_get(&d, "B"));
  ASSERT_EQ(0, dict_get(&d, "C"));

  // Remove the values
  dict_remove(&d, "A");
  dict_remove(&d, "B");
  dict_remove(&d, "C");

  // Make sure these values are not contained in the dictionary
  ASSERT_FALSE(dict_contains(&d, "A"));
  ASSERT_FALSE(dict_contains(&d, "B"));
  ASSERT_FALSE(dict_contains(&d, "C"));

  // Make sure we get -1 for each value
  ASSERT_EQ(-1, dict_get(&d, "A"));
  ASSERT_EQ(-1, dict_get(&d, "B"));
  ASSERT_EQ(-1, dict_get(&d, "C"));

  // Clean up
  dict_destroy(&d);
}

typedef struct thread_args {
  bool firstThread;
  my_dict_t *d;
} thread_args_t;

/**
 * @brief  Function the threads will run to check the first two invariants
 * @note
 * @param  *thread_args: struct to hold address of dictionary and a boolean of
 * whether it's the first thread or not
 * @retval None
 */
void *thread_run_12(void *thread_args) {
  thread_args_t *arguments = (thread_args_t *)thread_args;
  bool firstThread = arguments->firstThread;
  my_dict_t *d = arguments->d;

  // Set and remove entries
  if (firstThread) {
    dict_set(d, "A", 0);
    dict_set(d, "B", 0);
    dict_set(d, "C", 0);
    dict_set(d, "D", 0);
    dict_set(d, "E", 0);
    dict_set(d, "F", 0);
    dict_set(d, "G", 0);
    dict_set(d, "H", 0);
    dict_set(d, "I", 0);

    dict_remove(d, "A");
    dict_remove(d, "B");
    dict_remove(d, "C");
    dict_remove(d, "D");
    dict_remove(d, "E");

  } else {
    dict_set(d, "a", 0);
    dict_set(d, "b", 0);
    dict_set(d, "c", 0);
    dict_set(d, "d", 0);
    dict_set(d, "e", 0);
    dict_set(d, "f", 0);
    dict_set(d, "g", 0);
    dict_set(d, "h", 0);
    dict_set(d, "i", 0);

    dict_remove(d, "a");
    dict_remove(d, "b");
    dict_remove(d, "c");
  }
  return NULL;
}

/**
 * @brief  Function to check the first two invariants
 * @note
 * @retval
 */
TEST(DictionaryTest, Invariant1_2) {
  // initialize the dictionary and the arguments
  my_dict_t d;
  dict_init(&d);

  pthread_t threads[2];
  thread_args_t arguments[2];

  arguments[0].d = &d;
  arguments[0].firstThread = true;
  arguments[1].d = &d;
  arguments[1].firstThread = false;

  // Spawn the threads with the appropriate information
  for (int i = 0; i < 2; i++) {
    int ret = pthread_create(&threads[i], NULL, thread_run_12, &arguments[i]);
    if (ret != 0)
      perror("Error creating thread");
  }

  // Join the threads
  for (int i = 0; i < 2; i++) {
    int ret = pthread_join(threads[i], NULL);
    if (ret != 0)
      perror("Error joining thread");
  }

  // Make sure that no value that wasn't explicitly removed in the threads were
  // removed and remove the entry
  ASSERT_EQ(dict_get(&d, "F"), 0);
  dict_remove(&d, "F");
  ASSERT_EQ(dict_get(&d, "G"), 0);
  dict_remove(&d, "G");
  ASSERT_EQ(dict_get(&d, "H"), 0);
  dict_remove(&d, "H");
  ASSERT_EQ(dict_get(&d, "I"), 0);
  dict_remove(&d, "I");
  ASSERT_EQ(dict_get(&d, "d"), 0);
  dict_remove(&d, "d");
  ASSERT_EQ(dict_get(&d, "e"), 0);
  dict_remove(&d, "e");
  ASSERT_EQ(dict_get(&d, "f"), 0);
  dict_remove(&d, "f");
  ASSERT_EQ(dict_get(&d, "g"), 0);
  dict_remove(&d, "g");
  ASSERT_EQ(dict_get(&d, "h"), 0);
  dict_remove(&d, "h");
  ASSERT_EQ(dict_get(&d, "i"), 0);
  dict_remove(&d, "i");

  // Make sure all values that were added and removed aren't contained - i.e. no
  // imaginary entries were added
  ASSERT_FALSE(dict_contains(&d, "A"));
  ASSERT_FALSE(dict_contains(&d, "B"));
  ASSERT_FALSE(dict_contains(&d, "C"));
  ASSERT_FALSE(dict_contains(&d, "D"));
  ASSERT_FALSE(dict_contains(&d, "E"));
  ASSERT_FALSE(dict_contains(&d, "F"));
  ASSERT_FALSE(dict_contains(&d, "G"));
  ASSERT_FALSE(dict_contains(&d, "H"));
  ASSERT_FALSE(dict_contains(&d, "I"));
  ASSERT_FALSE(dict_contains(&d, "a"));
  ASSERT_FALSE(dict_contains(&d, "b"));
  ASSERT_FALSE(dict_contains(&d, "c"));
  ASSERT_FALSE(dict_contains(&d, "d"));
  ASSERT_FALSE(dict_contains(&d, "e"));
  ASSERT_FALSE(dict_contains(&d, "f"));
  ASSERT_FALSE(dict_contains(&d, "g"));
  ASSERT_FALSE(dict_contains(&d, "h"));
  ASSERT_FALSE(dict_contains(&d, "i"));

  // clean up
  dict_destroy(&d);
}

typedef struct thread_args3 {
  my_dict_t *d;
} thread_args3_t;
/**
 * @brief  Function to the run the thread for the third invariant - that there
 * aren't any entries with duplicate keys
 * @note
 * @param  thread_args:
 * @retval None
 */
void *thread_run_Invariant3(void *thread_args) {
  thread_args3_t *arguments = (thread_args3_t *)thread_args;
  my_dict_t *d = arguments->d;

  // Set 15 entries with "A"
  for (int i = 0; i < 15; i++) {
    dict_set(d, "A", i);
  }

  return NULL;
}
/**
 * @brief  Test for unique key - first to check that if multiple entries with
 * the same key is added and removed in parallel, only one entry is that key is
 * left
 * @note
 * @retval
 */
TEST(DictionaryTest, Invariant_3) {
  // initialize the dictionary
  my_dict_t d;
  dict_init(&d);

  pthread_t threads[2];
  thread_args3_t arguments[2];

  arguments[0].d = &d;
  arguments[1].d = &d;

  // Spawn the threads with the appropriate information
  for (int i = 0; i < 2; i++) {
    int ret =
        pthread_create(&threads[i], NULL, thread_run_Invariant3, &arguments[i]);
    if (ret != 0)
      perror("Error creating thread");
  }

  // Join the threads
  for (int i = 0; i < 2; i++) {
    int ret = pthread_join(threads[i], NULL);
    if (ret != 0)
      perror("Error joining thread");
  }

  // make sure that "A" is in the dictionary
  ASSERT_TRUE(dict_contains(&d, "A"));
  // remove one entry with "A" and make sure that there aren't any more left
  dict_remove(&d, "A");
  ASSERT_FALSE(dict_contains(&d, "A"));

  // clean up
  dict_destroy(&d);
}
