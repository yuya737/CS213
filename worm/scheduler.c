#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include "scheduler.h"

#include <assert.h>
#include <curses.h>
#include <ucontext.h>

#include "util.h"

// Kohei Kotani, Yuya Kawakami
// This is an upper limit on the number of tasks we can create.
#define MAX_TASKS 128

// This is the size of each task's stack memory
#define STACK_SIZE 65536

#define MAIN_TASK 0

#define RUNNING 0
#define READY 1
#define ASLEEP 2
#define BLOCKED_ON_INPUT 3
#define BLOCKED_ON_WAIT 4
#define EXITED 5

void scheduler();

// This struct will hold the all the necessary information for each task
typedef struct task_info {
    // This field stores all the state required to switch back to this task
    ucontext_t context;

    // This field stores another context. This one is only used when the task
    // is exiting.
    ucontext_t exit_context;

    // State of task
    int state;
    // Wake up time
    size_t wakeUpTime;
    // Task to wait for
    task_t taskToWaitFor;
    // Saved input
    int input;

    // TODO: Add fields here so you can:
    //   a. Keep track of this task's state.
    //   b. If the task is sleeping, when should it wake up?
    //   c. If the task is waiting for another task, which task is it waiting for?
    //   d. Was the task blocked waiting for user input? Once you successfully
    //      read input, you will need to save it here so it can be returned.
} task_info_t;

int current_task = 0; //< The handle of the currently-executing task
int num_tasks = 1;    //< The number of tasks created so far
task_info_t tasks[MAX_TASKS]; //< Information for every task

/**
 * Initialize the scheduler. Programs should call this before calling any other
 * functiosn in this file.
 */
void scheduler_init() {
    // intialize the first task a running
    tasks[current_task].state = RUNNING;
}


/**
 * This function will execute when a task's function returns. This allows you
 * to update scheduler states and start another task. This function is run
 * because of how the contexts are set up in the task_create function.
 */
void task_exit() {
    // when tasks exit change state to exited and run the scheduler
    tasks[current_task].state = EXITED;
    scheduler();
}

/**
 * Create a new task and add it to the scheduler.
 *
 * \param handle  The handle for this task will be written to this location.
 * \param fn      The new task will run this function.
 */
void task_create(task_t* handle, task_fn_t fn) {
    // Claim an index for the new task
    int index = num_tasks;
    num_tasks++;

    // Set the task handle to this index, since task_t is just an int
    *handle = index;
    tasks[index].state = READY;

    // We're going to make two contexts: one to run the task, and one that runs at the end of the task so we can clean up. Start with the second
    // First, duplicate the current context as a starting point
    getcontext(&tasks[index].exit_context);

    // Set up a stack for the exit context
    tasks[index].exit_context.uc_stack.ss_sp = malloc(STACK_SIZE);
    tasks[index].exit_context.uc_stack.ss_size = STACK_SIZE;

    // Set up a context to run when the task function returns. This should call task_exit.
    makecontext(&tasks[index].exit_context, task_exit, 0);

    // Now we start with the task's actual running context
    getcontext(&tasks[index].context);

    // Allocate a stack for the new task and add it to the context
    tasks[index].context.uc_stack.ss_sp = malloc(STACK_SIZE);
    tasks[index].context.uc_stack.ss_size = STACK_SIZE;

    // Now set the uc_link field, which sets things up so our task will go to the exit context when the task function finishes
    tasks[index].context.uc_link = &tasks[index].exit_context;

    // And finally, set up the context to execute the task function
    makecontext(&tasks[index].context, fn, 0);


}

/**current_task
 * Wait for a task to finish. If the task has not yet finished, the scheduler should
 * suspend this task and wake it up later when the task specified by handle has exited.
 *
 * \param handle  This is the handle produced by task_create
 */
void task_wait(task_t handle) {
    // Change state and assign the task to wait on
    tasks[current_task].state = BLOCKED_ON_WAIT;
    tasks[current_task].taskToWaitFor = handle;
    scheduler();
    printf("exiting task_wait\n");
}

/**
 * The currently-executing task should sleep for a specified time. If that time is larger
 * than zero, the scheduler should suspend this task and run a different task until at least
 * ms milliseconds have elapsed.
 *
 * \param ms  The number of milliseconds the task should sleep.
 */
void task_sleep(size_t ms) {
    // Change state and calculate the wakeupTime, run the scheduler
    tasks[current_task].state = ASLEEP;
    size_t wakeUpTime  = time_ms() + ms;
    tasks[current_task].wakeUpTime = wakeUpTime;
    scheduler();
}

/**
 * Read a character from user input. If no input is available, the task should
 * block until input becomes available. The scheduler should run a different
 * task while this task is blocked.
 *
 * \returns The read character code
 */
int task_readchar() {
    // To check for input, call getch(). If it returns ERR, no input was available.
    // Otherwise, getch() will returns the character code that was read.
    tasks[current_task].state = BLOCKED_ON_INPUT;
    scheduler();
    return tasks[current_task].input;
}

void scheduler() {
    // c: character from stdin, i: for loop counter, executable: flag to test if a process is executable, exit_count: number of exited processes 
    int c, i, executable = 0, exit_count = 0;

AGAIN:
    exit_count = 0;
    executable = 0;
    if (tasks[MAIN_TASK].state == BLOCKED_ON_INPUT){
        if((c = getch()) != ERR){
            tasks[MAIN_TASK].input = c;
            tasks[MAIN_TASK].state = READY;
        }
    }
    // loop from the next task
    for(i = current_task+1; i < num_tasks + current_task; i++) {
        switch(tasks[i % num_tasks].state) {
            // if the task is ready, schedule to execute this task
            case READY:
                executable = 1;
                if (tasks[current_task].state == RUNNING) tasks[current_task].state = READY;
                tasks[i % num_tasks].state = RUNNING;
                break;
            // if this task is asleep and the current time is after its wake up time, schedule to execute this task
            case ASLEEP:
                if(time_ms() > tasks[i % num_tasks].wakeUpTime) {
                    executable = 1;
                    if (tasks[current_task].state == RUNNING) tasks[current_task].state = READY;
                    tasks[i % num_tasks].state = RUNNING;
                }
                break;
            // if this task is blocked on input, get input and schedule to execute this task
            case BLOCKED_ON_INPUT:
                if((c = getch()) != ERR) {
                    executable = 1;
                    tasks[i % num_tasks].input = c;
                    if (tasks[current_task].state == RUNNING) tasks[current_task].state = READY;
                    tasks[i % num_tasks].state = RUNNING;
                }
                break;
            // if this task is blocked on wait, and the task it is waiting for is complete, schedule to execute this task
            case BLOCKED_ON_WAIT:
                if(tasks[tasks[i % num_tasks].taskToWaitFor].state == EXITED) {
                    executable = 1;
                    if (tasks[current_task].state == RUNNING) tasks[current_task].state = READY;
                    tasks[i % num_tasks].state = RUNNING;
                }
                break;
            // if this task has exited, increment exit_count counter
            case EXITED:
                exit_count++;
                break;
            default:
                printf("hi index is %d state is %d\n",i%num_tasks,tasks[i % num_tasks].state);
                perror("undefined");
        }
        // if something is executable, exit from loop
        if(executable) break;
    }
    // if not all tasks are exited, run this again
    if((!executable && exit_count != num_tasks - 1) || tasks[MAIN_TASK].state == BLOCKED_ON_INPUT) goto AGAIN;

    if(exit_count == num_tasks - 1) {
        return;
    }
    // change current_task to the new task to be run and swap the context to new task
    task_t temp = current_task;
    current_task = i % num_tasks;
    swapcontext(&tasks[temp].context, &tasks[i % num_tasks].context);
    return;
}
