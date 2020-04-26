#include <gtest/gtest.h>

#include "queue.hh"

/****** Queue Invariants ******/

// Invariant 1
// For every value V that has been put onto the queue p times and returned by take q times, there must be p-q copies of this value on the queue. This only holds if p >= q.

// Invariant 2
// No value should ever be returned by taken if it was not first passed to put by some thread.

// Invariant 3
// If a thread puts value A and then puts value B, and no other thread puts these specific values, A must not be taken from the queue before taking B.

/****** Begin Tests ******/

// Basic queue functionality
TEST(QueueTest, BasicQueueOps) {
  my_queue_t q;
  queue_init(&q);
  
  // Make sure the queue is empty
  ASSERT_TRUE(queue_empty(&q));
  
  // Make sure taking from the queue returns -1
  ASSERT_EQ(-1, queue_take(&q));
  
  // Add some items to the queue
  queue_put(&q, 1);
  queue_put(&q, 2);
  queue_put(&q, 3);
  
  // Make sure the queue is not empty
  ASSERT_FALSE(queue_empty(&q));
  
  // Take the values from the queue and check them
  ASSERT_EQ(1, queue_take(&q));
  ASSERT_EQ(2, queue_take(&q));
  ASSERT_EQ(3, queue_take(&q));
  
  // Make sure the queue is empty
  ASSERT_TRUE(queue_empty(&q));
  
  // Clean up
  queue_destroy(&q);
}

TEST(QueueTest, EmptyQueue){

  my_queue_t q;
  queue_init(&q);

  // Queue should be empty
  ASSERT_TRUE(queue_empty(&q));

  // Popping an empty queue should return -1
  ASSERT_EQ(-1, queue_take(&q));

  // Put something on the queue
  queue_put(&q, 0);

  // Queue should not be empty
  ASSERT_FALSE(queue_empty(&q));

  // Should take 0
  ASSERT_EQ(0, queue_take(&q));

  // Queue should be empty
  ASSERT_TRUE(queue_empty(&q));

  // Clean up
  queue_destroy(&q);

}


// Invariants 1 and 2

// Structs for the thread arugument and return values
typedef struct thread_arguments
{
  int num1Put;
  int num2Put;
  my_queue_t *queue;
} thread_args_t;

typedef struct return_args
{
  int num1Taken;
  int num2Taken;
} thread_ret_t;

// Function for the thread to run
void *thread_run_v1_v2(void *thread_args)
{
  thread_args_t *arguments = (thread_args_t *)thread_args;
  int num1Put = arguments->num1Put;
  int num2Put = arguments->num2Put;

  // Push 1s and 2s as prescribed
  for (int i = 0; i < num1Put; i++)
  {
    queue_put(arguments->queue, 1);
  }
  for (int i = 0; i < num2Put; i++)
  {
    queue_put(arguments->queue, 2);
  }

  // Ready the return struct
  thread_ret_t *ret = (thread_ret_t *)malloc(sizeof(thread_ret_t));
  ret->num1Taken = 0;
  ret->num2Taken = 0;

  // Pop a random number of times from 0 to 14 and count the number of 1s and 2s that are taken and return that count
  for (int i = 0; i < (rand() % 15); i++)
  {
    int takenNum = queue_take(arguments->queue);
    if (takenNum == 1)
    {
      ret->num1Taken++;
    }
    else if (takenNum == 2)
    {
      ret->num2Taken++;
    }
    else if (takenNum == -1)
    {
      // do nothing, the queue is empty
    }
    else
    {
      perror("Took neither 1 nor 2");
    }
  }
  return (void *)ret;
}

TEST(queueTest, Invariant1_2)
{
  // Initialize the queue as well as thread argument and return structs
  my_queue_t q;
  queue_init(&q);

  srand(time(NULL));
  pthread_t threads[2];
  thread_args_t arguments[2];
  thread_ret_t *thread_ret[2];

  // Ready the number of 1s and 2s that each thread will push
  int num1PutT1 = rand() % 15;
  int num2PutT1 = rand() % 15;

  int num1PutT2 = rand() % 15;
  int num2PutT2 = rand() % 15;

  arguments[0].num1Put = num1PutT1;
  arguments[0].num2Put = num2PutT1;
  arguments[0].queue = &q;

  arguments[1].num1Put = num1PutT2;
  arguments[1].num2Put = num2PutT2;
  arguments[1].queue = &q;

  // Spawn the threads with the appropriate information
  for (int i = 0; i < 2; i++)
  {
    int ret = pthread_create(&threads[i], NULL, thread_run_v1_v2, &arguments[i]);
    if (ret != 0)
      perror("Error creating thread");
  }

  // Join the threads, storing the return structs in the array
  for (int i = 0; i < 2; i++)
  {
    int ret = pthread_join(threads[i], (void **)&thread_ret[i]);
    if (ret != 0)
      perror("Error joining thread");
  }

  // Calculate the total number of 1 and 2s pushed and taken
  int total1Put = arguments[0].num1Put + arguments[1].num1Put;
  int total2Put = arguments[0].num2Put + arguments[1].num2Put;
  int total1Taken = thread_ret[0]->num1Taken + thread_ret[1]->num1Taken;
  int total2Taken = thread_ret[0]->num2Taken + thread_ret[1]->num2Taken;

  int totalPut = total1Put + total2Put;
  int totalTaken = total1Taken + total2Taken;

  // If there are more values pushed than taken, pop all the remaining values.
  int total1Left = 0;
  int total2Left = 0;

  if (totalPut > totalTaken)
  {
    for (int i = 0; i < (totalPut - totalTaken); i++)
    {
      // If values are left, pop the values and keep track of the number of 1 and 2s that are left/taken.
      int taken = queue_take(&q);
      if (taken == 1)
      {
        total1Left++;
      }
      else if (taken == 2)
      {
        total2Left++;
      }
      else
      {
        perror("Took neither 1 nor 2 at end");
      }
    }
  }

  // Assert that the queue is empty and that the total number of 1s pushed is equal to the number of 1s taken. Do the same for 2s
  ASSERT_EQ(0, q.size);
  ASSERT_EQ(total1Put, total1Taken + total1Left);
  ASSERT_EQ(total2Put, total2Taken + total2Left);

  // Free malloc-ed memory and destroy the queue.ue
  free(thread_ret[0]);
  free(thread_ret[1]);
  queue_destroy(&q);
}




// struct for argument to thread in third invariant
typedef struct thread_arguments3
{
  bool even;
  my_queue_t *queue;
} thread_args3Put_t;

// function to run on the Push test in the third invariant
void *thread_run_v3_Push(void *thread_args)
{
  thread_args3Put_t *arguments = (thread_args3Put_t *)thread_args;
  // Push either even or odd numbers to the queue
  for (int i = 0; i < 6; i++)
  {
    int toPut = arguments->even ? 2 * i : (2 * i) + 1;
    queue_put(arguments->queue, toPut);
  }
  return NULL;
}

// Checks if values A and B are pushed in a thread, A is taken before B in main thread
TEST(queueTest, Invariant3_Push)
{
  // Prepare the threads, initialize the queue as well as the thread argument structs
  pthread_t threads[2];
  thread_args3Put_t arguments[2];

  my_queue_t q;
  queue_init(&q);

  // The first thread will push even numbers
  arguments[0].even = true;
  arguments[0].queue = &q;

  // The second thread will push odd numbers
  arguments[1].even = false;
  arguments[1].queue = &q;
  
  // Launch the threads and join the threads
  for (int i = 0; i < 2; i++)
  {
    int ret = pthread_create(&threads[i], NULL, thread_run_v3_Push, &arguments[i]);
    if (ret != 0)
      perror("Error creating thread");
  }

  for (int i = 0; i < 2; i++)
  {
    int ret = pthread_join(threads[i], NULL);
    if (ret != 0)
      perror("Error joining threads");
  }

  // Make sure that each odd and even number that gets taken is greater that the last. The smallest even and odd number 
  // in the queue in 12 and 13.
  int lastEven = 0;
  int lastOdd = 1;

  // Pop each number, check if even or odd and compare to the last even or odd number to make sure the order is being kept
  for (int i = 0; i < q.size; i++)
  {
    int taken = queue_take(&q);
    if (taken % 2 == 0)
    {
      ASSERT_FALSE(taken < lastEven);
      lastEven = taken;
    }
    else
    {
      ASSERT_FALSE(taken < lastOdd);
      lastOdd = taken;
    }
  }
  // Clean up
  queue_destroy(&q);
}

// structs for the take in the third invariant
typedef struct thread_arguments3_Take
{
  my_queue_t *queue;
} thread_args3_t;

// struct for the return value. Holds an int array of length 7 to keep track of the order of the 7 numbers that the thread will pop
typedef struct return_args3
{
  int ret[7];
} thread_ret3_t;

// Function to run for the take version in the third invariant
void *thread_run_v3_Take(void *thread_args)
{
  thread_args3_t *arguments = (thread_args3_t *)thread_args;
  my_queue_t *q = arguments->queue;
  // Allocate space for the return value of the thread
  thread_ret3_t *ret = (thread_ret3_t *)malloc(sizeof(thread_ret3_t));

  // Take 7 numbers and record the order in the return struct
  for (int i = 0; i < 7; i++)
  {
    int taken = queue_take(q);
    ret->ret[i] = taken;
  }
  return (void *)ret;
}

// Checks to make sure that if A and B are added in the main thread, when the values are taken in a thread, A comes before B
TEST(queueTest, Invariant3_Pop)
{
  // Declare the threads and neccessary structs  for the threads.
  pthread_t threads[2];
  thread_args3_t arguments[2];
  thread_ret3_t *thread_ret[2];

  // Initialize the queue
  my_queue_t q;
  queue_init(&q);

  arguments[0].queue = &q;
  arguments[1].queue = &q;

  // Push the first 14 numbers to the queue, including 0.
  for (int i = 0; i < 14; i++)
  {
    queue_put(&q, i);
  }

  // Launch the threads
  for (int i = 0; i < 2; i++)
  {
    int ret = pthread_create(&threads[i], NULL, thread_run_v3_Take, &arguments[i]);
    if (ret != 0)
      perror("Error creating thread");
  }
  // Join the threads and store the order in which each thread taken values.
  for (int i = 0; i < 2; i++)
  {
    int ret = pthread_join(threads[i], (void **)&thread_ret[i]);
    if (ret != 0)
      perror("Error joining threads");
  }
  // Make sure that for every odd or even number, it is greater than the last even or odd number that was taken.
  for (int i = 0; i < 2; i++)
  {
    // We pushed the first 14 numbers (including 0), so the first even and odd numbers are 0 and 1.
    int lastEven = 0;
    int lastOdd = 1;
    for (int j = 0; j < 6; j++)
    {
      // For every number that each thread taken, make sure that its smaller than the last even or odd number
      int taken = thread_ret[i]->ret[j];
      if (taken % 2 == 0)
      {
        ASSERT_FALSE(taken < lastEven);
      }
      else
      {
        ASSERT_FALSE(taken < lastOdd);
      }
    }
  }
  // Free malloc-ed memory and clean up
  free(thread_ret[0]);
  free(thread_ret[1]);
  queue_destroy(&q);
}
