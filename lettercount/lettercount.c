#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <math.h>
#include <assert.h>

#define PAGE_SIZE 0x1000
#define ROUND_UP(x,y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

// Mutex lock to be used
pthread_mutex_t lock;

/// The number of times we've seen each letter in the input, initially zero
size_t letter_counts[26] = {0};

// The function the thread will run on, takes the string segment to be counted by this thread
void *thread_count(void* file_data){
    // for each letter in the string segment...
    for(size_t i = 0; i < strlen((char*) file_data); i++){
        char c = ((char*)file_data)[i];
        // if cases to make this case insensitive -  use the lock to access shared resources and increment counter
        if (c >= 'a' && c <= 'z'){
            int rc = pthread_mutex_lock(&lock);
            assert(rc == 0);
            letter_counts[c - 'a']++;
            rc = pthread_mutex_unlock(&lock);
            assert(rc == 0);
        } else if(c >= 'A' && c <= 'Z'){
            int rc = pthread_mutex_lock(&lock);
            assert(rc == 0);
            letter_counts[c - 'A']++;
            rc = pthread_mutex_unlock(&lock);
            assert(rc == 0);
        }
    }
    return NULL;
}
/**
 * This function should divide up the file_data between the specified number of
 * threads, then have each thread count the number of occurrences of each letter
 * in the input data. Counts should be written to the letter_counts array. Make
 * sure you count only the 26 different letters, treating upper- and lower-case
 * letters as the same. Skip all other characters.
 *
 * \param num_threads   The number of threads your program must use to count
 *                      letters. Divide work evenly between threads
 * \param file_data     A pointer to the beginning of the file data array
 * \param file_size     The number of bytes in the file_data array
 */
void count_letters(int num_threads, char* file_data, off_t file_size) {
    // get a rough count of characters each thread should process and initialize a count so far variable to keep track of how many characters we've read
    off_t roughCount = (off_t) file_size /  num_threads;
    off_t countSoFar = 0;

    // Initialize the mutex lock and the list of threads
    pthread_mutex_init(&lock, NULL);
    pthread_t threadList[num_threads];

    // For each thread, calculate the string segement to be counted and then pass it to the thread count function
    for (int i = 0; i < num_threads; i++){
        off_t threadReadSize;
        if (i == (num_threads - 1)){
            threadReadSize = file_size - countSoFar;
        } else {
            threadReadSize = roughCount;
        }
        char* temp  = file_data;
        temp = temp + countSoFar;
        char* file_data = strndup(temp, threadReadSize);

        // Create the thread with the string segment
        int rc = pthread_create(&threadList[i], NULL, thread_count, (void*) file_data);
        assert(rc == 0);
        countSoFar = countSoFar + threadReadSize;
    }
    // Join the thread at the end to ensure completion
    for (int i = 0; i < num_threads; i++){
        int rc = pthread_join(threadList[i], NULL);
        assert(rc == 0);
    }
}





/**
 * Show instructions on how to run the program.
 * \param program_name  The name of the command to print in the usage info
 */
void show_usage(char* program_name) {
    fprintf(stderr, "Usage: %s <N> <input file>\n", program_name);
    fprintf(stderr, "    where <N> is the number of threads (1, 2, 4, or 8)\n");
    fprintf(stderr, "    and <input file> is a path to an input text file.\n");
}

int main(int argc, char** argv) {
    // Check parameter count
    if(argc != 3) {
        show_usage(argv[0]);
        exit(1);
    }

    // Read thread count
    int num_threads = atoi(argv[1]);
    if(num_threads != 1 && num_threads != 2 && num_threads != 4 && num_threads != 8) {
        fprintf(stderr, "Invalid number of threads: %s\n", argv[1]);
        show_usage(argv[0]);
        exit(1);
    }

    // Open the input file
    int fd = open(argv[2], O_RDONLY);
    if(fd == -1) {
        fprintf(stderr, "Unable to open input file: %s\n", argv[2]);
        show_usage(argv[0]);
        exit(1);
    }

    // Get the file size
    off_t file_size = lseek(fd, 0, SEEK_END);
    if(file_size == -1) {
        fprintf(stderr, "Unable to seek to end of file\n");
        exit(2);
    }

    // Seek back to the start of the file
    if(lseek(fd, 0, SEEK_SET)) {
        fprintf(stderr, "Unable to seek to the beginning of the file\n");
        exit(2);
    }

    // Load the file with mmap
    char* file_data = mmap(NULL, ROUND_UP(file_size, PAGE_SIZE), PROT_READ, MAP_PRIVATE, fd, 0);
    if(file_data == MAP_FAILED) {
        fprintf(stderr, "Failed to map file\n");
        exit(2);
    }

    // Call the function to count letter frequencies
    count_letters(num_threads, file_data, file_size);

    // Print the letter counts
    for(int i=0; i<26; i++) {
        printf("%c: %lu\n", 'a' + i, letter_counts[i]);
    }

    return 0;
}
