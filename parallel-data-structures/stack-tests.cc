#include <gtest/gtest.h>

#include "stack.hh"
#include <pthread.h>
#include <stdio.h>
#include <time.h>

/****** Stack Invariants ******/

// Invariant 1
// For every value V that has been pushed onto the stack p times and returned by
// pop q times, there must be p-q copies of this value on the stack. This only
// holds if p >= q.

// Invariant 2
// No value should ever be returned by pop if it was not first passed to push by
// some thread.

// Invariant 3
// If a thread pushes value A and then pushes value B, and no other thread
// pushes these specific values, A must not be popped from the stack before
// popping B.

/****** Begin Tests ******/

// A simple test of basic stack functionality
TEST(StackTest, BasicStackOps) {
  // Create a stack
  my_stack_t s;
  stack_init(&s);

  // Push some values onto the stack
  stack_push(&s, 1);
  stack_push(&s, 2);
  stack_push(&s, 3);

  // Make sure the elements come off the stack in the right order
  ASSERT_EQ(3, stack_pop(&s));
  ASSERT_EQ(2, stack_pop(&s));
  ASSERT_EQ(1, stack_pop(&s));

  // Clean up
  stack_destroy(&s);
}

// Another test case
TEST(StackTest, EmptyStack) {
  // Create a stack
  my_stack_t s;
  stack_init(&s);

  // The stack should be empty
  ASSERT_TRUE(stack_empty(&s));

  // Popping an empty stack should return -1
  ASSERT_EQ(-1, stack_pop(&s));

  // Put something on the stack
  stack_push(&s, 0);

  // The stack should not be empty
  ASSERT_FALSE(stack_empty(&s));

  // Pop the element off the stack.
  // We're just testing empty stack behavior, so there's no need to check the
  // resulting value
  stack_pop(&s);

  // The stack should be empty now
  ASSERT_TRUE(stack_empty(&s));

  // Clean up
  stack_destroy(&s);
}

// Invariants 1 and 2

// Structs for the thread arugument and return values
typedef struct thread_arguments {
  int num1Pushed;
  int num2Pushed;
  my_stack_t *stack;
} thread_args_t;

typedef struct return_args {
  int num1Popped;
  int num2Popped;
} thread_ret_t;

// Function for the thread to run
void *thread_run_v1_v2(void *thread_args) {
  thread_args_t *arguments = (thread_args_t *)thread_args;
  int num1Pushed = arguments->num1Pushed;
  int num2Pushed = arguments->num2Pushed;

  // Push 1s and 2s as prescribed
  for (int i = 0; i < num1Pushed; i++) {
    stack_push(arguments->stack, 1);
  }
  for (int i = 0; i < num2Pushed; i++) {
    stack_push(arguments->stack, 2);
  }

  // Ready the return struct
  thread_ret_t *ret = (thread_ret_t *)malloc(sizeof(thread_ret_t));
  ret->num1Popped = 0;
  ret->num2Popped = 0;

  // Pop a random number of times from 0 to 14 and count the number of 1s and 2s
  // that are popped and return that count
  for (int i = 0; i < (rand() % 15); i++) {
    int poppedNum = stack_pop(arguments->stack);
    if (poppedNum == 1) {
      ret->num1Popped++;
    } else if (poppedNum == 2) {
      ret->num2Popped++;
    } else if (poppedNum == -1) {
      // do nothing, the stack is empty
    } else {
      perror("Popped neither 1 nor 2");
    }
  }
  return (void *)ret;
}

TEST(StackTest, Invariant1_2) {
  // Initialize the stack as well as thread argument and return structs
  my_stack_t s;
  stack_init(&s);

  srand(time(NULL));
  pthread_t threads[2];
  thread_args_t arguments[2];
  thread_ret_t *thread_ret[2];

  // Ready the number of 1s and 2s that each thread will push
  int num1PushedT1 = rand() % 15;
  int num2PushedT1 = rand() % 15;

  int num1PushedT2 = rand() % 15;
  int num2PushedT2 = rand() % 15;

  arguments[0].num1Pushed = num1PushedT1;
  arguments[0].num2Pushed = num2PushedT1;
  arguments[0].stack = &s;

  arguments[1].num1Pushed = num1PushedT2;
  arguments[1].num2Pushed = num2PushedT2;
  arguments[1].stack = &s;

  // Spawn the threads with the appropriate information
  for (int i = 0; i < 2; i++) {
    int ret =
        pthread_create(&threads[i], NULL, thread_run_v1_v2, &arguments[i]);
    if (ret != 0)
      perror("Error creating thread");
  }

  // Join the threads, storing the return structs in the array
  for (int i = 0; i < 2; i++) {
    int ret = pthread_join(threads[i], (void **)&thread_ret[i]);
    if (ret != 0)
      perror("Error joining thread");
  }

  // Calculate the total number of 1 and 2s pushed and popped
  int total1Pushed = arguments[0].num1Pushed + arguments[1].num1Pushed;
  int total2Pushed = arguments[0].num2Pushed + arguments[1].num2Pushed;
  int total1Popped = thread_ret[0]->num1Popped + thread_ret[1]->num1Popped;
  int total2Popped = thread_ret[0]->num2Popped + thread_ret[1]->num2Popped;

  int totalPushed = total1Pushed + total2Pushed;
  int totalPopped = total1Popped + total2Popped;

  // If there are more values pushed than popped, pop all the remaining values.
  int total1Left = 0;
  int total2Left = 0;

  if (totalPushed > totalPopped) {
    for (int i = 0; i < (totalPushed - totalPopped); i++) {
      // If values are left, pop the values and keep track of the number of 1
      // and 2s that are left/popped.
      int popped = stack_pop(&s);
      if (popped == 1) {
        total1Left++;
      } else if (popped == 2) {
        total2Left++;
      } else {
        perror("Popped neither 1 nor 2 at end");
      }
    }
  }

  // Assert that the stack is empty and that the total number of 1s pushed is
  // equal to the number of 1s popped. Do the same for 2s
  ASSERT_EQ(0, s.size);
  ASSERT_EQ(total1Pushed, total1Popped + total1Left);
  ASSERT_EQ(total2Pushed, total2Popped + total2Left);

  // Free malloc-ed memory and destroy the stack.ue
  free(thread_ret[0]);
  free(thread_ret[1]);
  stack_destroy(&s);
}

// struct for argument to thread in third invariant
typedef struct thread_arguments3 {
  bool even;
  my_stack_t *stack;
} thread_args3Push_t;

// function to run on the Push test in the third invariant
void *thread_run_v3_Push(void *thread_args) {
  thread_args3Push_t *arguments = (thread_args3Push_t *)thread_args;
  // Push either even or odd numbers to the stack
  for (int i = 0; i < 6; i++) {
    int toPush = arguments->even ? 2 * i : (2 * i) + 1;
    stack_push(arguments->stack, toPush);
  }
  return NULL;
}

// Checks if values A and B are pushed in a thread, A is popped before B in main
// thread
TEST(StackTest, Invariant3_Push) {
  // Prepare the threads, initialize the stack as well as the thread argument
  // structs
  pthread_t threads[2];
  thread_args3Push_t arguments[2];

  my_stack_t s;
  stack_init(&s);

  // The first thread will push even numbers
  arguments[0].even = true;
  arguments[0].stack = &s;

  // The second thread will push odd numbers
  arguments[1].even = false;
  arguments[1].stack = &s;

  // Launch the threads and join the threads
  for (int i = 0; i < 2; i++) {
    int ret =
        pthread_create(&threads[i], NULL, thread_run_v3_Push, &arguments[i]);
    if (ret != 0)
      perror("Error creating thread");
  }

  for (int i = 0; i < 2; i++) {
    int ret = pthread_join(threads[i], NULL);
    if (ret != 0)
      perror("Error joining threads");
  }

  // Make sure that each odd and even number that gets popped is less that the
  // last. The largest even and odd number in the stack in 12 and 13.
  int lastEven = 12;
  int lastOdd = 13;

  // Pop each number, check if even or odd and compare to the last even or odd
  // number to make sure the order is being kept
  for (int i = 0; i < s.size; i++) {
    int popped = stack_pop(&s);
    if (popped % 2 == 0) {
      ASSERT_FALSE(popped > lastEven);
      lastEven = popped;
    } else {
      ASSERT_FALSE(popped > lastOdd);
      lastOdd = popped;
    }
  }
  // Clean up
  stack_destroy(&s);
}

// structs for the pop in the third invariant
typedef struct thread_arguments3_Pop {
  my_stack_t *stack;
} thread_args3Pop_t;

// struct for the return value. Holds an int array of length 7 to keep track of
// the order of the 7 numbers that the thread will pop
typedef struct return_args3 {
  int ret[7];
} thread_ret3_t;

// Function to run for the pop version in the third invariant
void *thread_run_v3_Pop(void *thread_args) {
  thread_args3Pop_t *arguments = (thread_args3Pop_t *)thread_args;
  my_stack_t *s = arguments->stack;
  // Allocate space for the return value of the thread
  thread_ret3_t *ret = (thread_ret3_t *)malloc(sizeof(thread_ret3_t));

  // Pop 7 numbers and record the order in the return struct
  for (int i = 0; i < 7; i++) {
    int popped = stack_pop(s);
    ret->ret[i] = popped;
  }
  return (void *)ret;
}

// Checks to make sure that if A and B are added in the main thread, when the
// values are popped in a thread, A comes before B
TEST(StackTest, Invariant3_Pop) {
  // Declare the threads and neccessary structs  for the threads.
  pthread_t threads[2];
  thread_args3Pop_t arguments[2];
  thread_ret3_t *thread_ret[2];

  // Initialize the stack
  my_stack_t s;
  stack_init(&s);

  arguments[0].stack = &s;
  arguments[1].stack = &s;

  // Push the first 14 numbers to the stack
  for (int i = 0; i < 14; i++) {
    stack_push(&s, i);
  }

  // Launch the threads
  for (int i = 0; i < 2; i++) {
    int ret =
        pthread_create(&threads[i], NULL, thread_run_v3_Pop, &arguments[i]);
    if (ret != 0)
      perror("Error creating thread");
  }
  // Join the threads and store the order in which each thread popped values.
  for (int i = 0; i < 2; i++) {
    int ret = pthread_join(threads[i], (void **)&thread_ret[i]);
    if (ret != 0)
      perror("Error joining threads");
  }
  // Make sure that for every odd or even number, it is smaller than the last
  // even or odd number that was popped.
  for (int i = 0; i < 2; i++) {
    // We pushed the first 14 numbers (including 0), so the last even and odd
    // numbers are 12 and 13.
    int lastEven = 12;
    int lastOdd = 13;
    for (int j = 0; j < 6; j++) {
      // For every number that each thread popped, make sure that its smaller
      // than the last even or odd number
      int popped = thread_ret[i]->ret[j];
      if (popped % 2 == 0) {
        ASSERT_FALSE(popped > lastEven);
      } else {
        ASSERT_FALSE(popped > lastOdd);
      }
    }
  }
  // Free malloc-ed memory and clean up
  free(thread_ret[0]);
  free(thread_ret[1]);
  stack_destroy(&s);
}