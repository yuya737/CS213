#define _GNU_SOURCE
#include <openssl/md5.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_USERNAME_LENGTH 64
#define PASSWORD_LENGTH 6

/************************* Part A *************************/
/********************* Parts B & C ************************/

/**
 * Find a six character lower-case alphabetic password that hashes
 * to the given hash value. Complete this function for part A of the lab.
 *
 * \param input_hash  An array of MD5_DIGEST_LENGTH bytes that holds the hash of a password
 * \param output      A pointer to memory with space for a six character password + '\0'
 * \returns           0 if the password was cracked. -1 otherwise.
 */
int crack_single_password(uint8_t* input_hash, char* output) {
    // Take our candidate password and hash it using MD5

    char candidate_passwd[7] = "aaaaaa"; //< This variable holds the password we are trying
    for (int first = 0; first < 26; first++){
        for (int second = 0; second < 26; second++){
            for (int third = 0; third < 26; third++){
                for (int fourth = 0; fourth < 26; fourth++){
                    for (int fifth = 0; fifth < 26; fifth++){
                        for (int sixth = 0; sixth < 26; sixth++){
                            uint8_t candidate_hash[MD5_DIGEST_LENGTH]; //< This will hold the hash of the candidate password
                            MD5((unsigned char*)candidate_passwd, strlen(candidate_passwd), candidate_hash); //< Do the hash

                            // Now check if the hash of the candidate password matches the input hash
                            if(memcmp(input_hash, candidate_hash, MD5_DIGEST_LENGTH) == 0) {
                                // Match! Copy the password to the output and return 0 (success)
                                strncpy(output, candidate_passwd, PASSWORD_LENGTH+1);
                                return 0;
                            }
                            candidate_passwd[5]++;
                        }
                        candidate_passwd[5] = 'a';
                        candidate_passwd[4]++;

                    }
                    candidate_passwd[4] = 'a';
                    candidate_passwd[3]++;

                }
                candidate_passwd[3] = 'a';
                candidate_passwd[2]++;

            }
            candidate_passwd[2] = 'a';
            candidate_passwd[1]++;

        }
        candidate_passwd[1] = 'a';
        candidate_passwd[0]++;
    }
    candidate_passwd[0] = 'a';
    return -1;
}

/********************* Parts B & C ************************/



/**
 * This struct is the root of the data structure that will hold users and hashed passwords.
 * This could be any type of data structure you choose: list, array, tree, hash table, etc.
 * Implement this data structure for part B of the lab.
 */
typedef struct password_set {
    char username[MAX_USERNAME_LENGTH];
    uint8_t input_hash[MD5_DIGEST_LENGTH];
    struct password_set* next;
} password_set_t;

/**
 * Initialize a password set.
 * Complete this implementation for part B of the lab.
 *
 * \param passwords  A pointer to allocated memory that will hold a password set
 */
void init_password_set(password_set_t* passwords) {
    strcpy(passwords->username, "");
    passwords->next = NULL;
}

/**
 * Add a password to a password set
 * Complete this implementation for part B of the lab.
 *
 * \param passwords   A pointer to a password set initialized with the function above.
 * \param username    The name of the user being added. The memory that holds this string's
 *                    characters will be reused, so if you keep a copy you must duplicate the
 *                    string. I recommend calling strdup().
 * \param password_hash   An array of MD5_DIGEST_LENGTH bytes that holds the hash of this user's
 *                        password. The memory that holds this array will be reused, so you must
 *                        make a copy of this value if you retain it in your data structure.
 */
void add_password(password_set_t* passwords, char* username, uint8_t* password_hash) {
    if (strcmp(passwords->username, "") == 0  && passwords->next == NULL){
        memcpy(passwords->username, username, MAX_USERNAME_LENGTH);
        memcpy(passwords->input_hash, password_hash, MD5_DIGEST_LENGTH);
        passwords->next = NULL;
    } else {
        password_set_t* cur = passwords;
        while (cur->next != NULL){
            cur = cur->next;
        }
        password_set_t* newNode = (password_set_t*) malloc(sizeof(password_set_t));
        memcpy(newNode->username, username, MAX_USERNAME_LENGTH);
        memcpy(newNode->input_hash, password_hash, MD5_DIGEST_LENGTH);
        newNode->next = NULL;
        cur->next = newNode;
    }
}

/**
 * Crack all of the passwords in a set of passwords. The function should print the username
 * and cracked password for each user listed in passwords, separated by a space character.
 * Complete this implementation for part B
 *
 * \returns The number of passwords cracked in the list
 */
int crack_password_list_not_parallel(password_set_t* passwords) {
    int count = 0;
    char candidate_passwd[7] = "aaaaaa"; //< This variable holds the password we are trying
    for (int first = 0; first < 26; first++){
        for (int second = 0; second < 26; second++){
            for (int third = 0; third < 26; third++){
                for (int fourth = 0; fourth < 26; fourth++){
                    for (int fifth = 0; fifth < 26; fifth++){
                        for (int sixth = 0; sixth < 26; sixth++){
                            uint8_t candidate_hash[MD5_DIGEST_LENGTH]; //< This will hold the hash of the candidate password
                            MD5((unsigned char*)candidate_passwd, strlen(candidate_passwd), candidate_hash); //< Do the hash

                            password_set_t* cur = passwords;
                            while (cur != NULL){
                                // Now check if the hash of the candidate password matches the input hash
                                if(memcmp(cur->input_hash, candidate_hash, MD5_DIGEST_LENGTH) == 0) {
                                    // Match! Copy the password to the output and return 0 (success)
                                    printf("%s %s\n", cur->username, candidate_passwd);
                                    count++;
                                }
                                cur = cur->next;
                            }

                            candidate_passwd[5]++;
                        }
                        candidate_passwd[5] = 'a';

                        candidate_passwd[4]++;

                    }
                    candidate_passwd[4] = 'a';
                    candidate_passwd[3]++;

                }
                candidate_passwd[3] = 'a';
                candidate_passwd[2]++;

            }
            candidate_passwd[2] = 'a';
            candidate_passwd[1]++;

        }
        candidate_passwd[1] = 'a';
        candidate_passwd[0]++;
    }
    candidate_passwd[0] = 'a';
    return count;
}

typedef struct arg_struct {
    int thread_num;
    password_set_t* passwords;
} arg_t;

/**
 * Crack all of the passwords in a set of passwords. The function should print the username
 * and cracked password for each user listed in passwords, separated by a space character.
 * Complete this implementation for part B
 *
 * \returns The number of passwords cracked in the list
 */


/**
 * function to be called on the thread. The implementation is the same as the non parallel version, except this
 * takes void* arguments that holds the username/password dictionary and the thread num. 
 */

void* thread_crack(void* arguments) {
    int num = ((arg_t*)arguments)->thread_num;
    password_set_t* passwords = ((arg_t*)arguments)->passwords;
    int count = 0;
    char candidate_passwd[7] = "aaaaaa"; //< This variable holds the password we are trying
    // Move the first character forward according to the thread number/
    candidate_passwd[0] += num*6;
    // Iterate the same way as before, except only go through a subset of the password options
    // firstCharNum holds the number of first characters the given thread needs to look through. The first 
    // three threads will look through 6, the last thread will look through 8, summing to 26. 
    int firstCharNum;
    if (num == 3) {
        firstCharNum = 8;
    } else{
        firstCharNum = 6;
    }
    // Iterate only through the alloted number of first characters
    for (int first = 0; first < firstCharNum; first++){
        for (int second = 0; second < 26; second++){
            for (int third = 0; third < 26; third++){
                for (int fourth = 0; fourth < 26; fourth++){
                    for (int fifth = 0; fifth < 26; fifth++){
                        for (int sixth = 0; sixth < 26; sixth++){
                            uint8_t candidate_hash[MD5_DIGEST_LENGTH]; //< This will hold the hash of the candidate password
                            MD5((unsigned char*)candidate_passwd, strlen(candidate_passwd), candidate_hash); //< Do the hash

                            password_set_t* cur = passwords;
                            while (cur != NULL){
                                // Now check if the hash of the candidate password matches the input hash
                                if(memcmp(cur->input_hash, candidate_hash, MD5_DIGEST_LENGTH) == 0) {
                                    // Match! Copy the password to the output and return 0 (success)
                                    printf("%s %s\n", cur->username, candidate_passwd);
                                    count++;
                                }
                                cur = cur->next;
                            }

                            candidate_passwd[5]++;
                        }
                        candidate_passwd[5] = 'a';

                        candidate_passwd[4]++;

                    }
                    candidate_passwd[4] = 'a';
                    candidate_passwd[3]++;

                }
                candidate_passwd[3] = 'a';
                candidate_passwd[2]++;

            }
            candidate_passwd[2] = 'a';
            candidate_passwd[1]++;

        }
        candidate_passwd[1] = 'a';
        candidate_passwd[0]++;
    }
    candidate_passwd[0] = 'a';
    return (void *) (size_t)count;
}

/* function to crack passwords in parallel
 */


int crack_password_list(password_set_t* passwords){
    // The final count to return; the number of passwords cracked
    int count = 0;
    pthread_t threadList[4];
    arg_t argList[4];
    for (int i=0; i<4; i++){
        argList[i].thread_num = i;
        argList[i].passwords = passwords;
        // Create the arguments to pass through the thread and call the thread
        pthread_create(&threadList[i],NULL, thread_crack, &argList[i]);
    }
    for (int j = 0; j<4; j++){
        void* ret;
        pthread_join(threadList[j], (void**) &ret);
        count += (int)ret;
    }
    return count;
}


/******************** Provided Code ***********************/

/**
 * Convert a string representation of an MD5 hash to a sequence
 * of bytes. The input md5_string must be 32 characters long, and
 * the output buffer bytes must have room for MD5_DIGEST_LENGTH
 * bytes.
 *
 * \param md5_string  The md5 string representation
 * \param bytes       The destination buffer for the converted md5 hash
 * \returns           0 on success, -1 otherwise
 */
int md5_string_to_bytes(const char* md5_string, uint8_t* bytes) {
    // Check for a valid MD5 string
    if(strlen(md5_string) != 2 * MD5_DIGEST_LENGTH) return -1;

    // Start our "cursor" at the start of the string
    const char* pos = md5_string;

    // Loop until we've read enough bytes
    for(size_t i=0; i<MD5_DIGEST_LENGTH; i++) {
        // Read one byte (two characters)
        int rc = sscanf(pos, "%2hhx", &bytes[i]);
        if(rc != 1) return -1;

        // Move the "cursor" to the next hexadecimal byte
        pos += 2;
    }

    return 0;
}

void print_usage(const char* exec_name) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s single <MD5 hash>\n", exec_name);
    fprintf(stderr, "  %s list <password file name>\n", exec_name);
}

int main(int argc, char** argv) {
    if(argc != 3) {
        print_usage(argv[0]);
        exit(1);
    }

    if(strcmp(argv[1], "single") == 0) {
        // The input MD5 hash is a string in hexadecimal. Convert it to bytes.
        uint8_t input_hash[MD5_DIGEST_LENGTH];
        if(md5_string_to_bytes(argv[2], input_hash)) {
            fprintf(stderr, "Input has value %s is not a valid MD5 hash.\n", argv[2]);
            exit(1);
        }

        // Now call the crack_single_password function
        char result[7];
        if(crack_single_password(input_hash, result)) {
            printf("No matching password found.\n");
        } else {
            printf("%s\n", result);
        }

    } else if(strcmp(argv[1], "list") == 0) {
        // Make and initialize a password set
        password_set_t passwords;
        init_password_set(&passwords);

        // Open the password file
        FILE* password_file = fopen(argv[2], "r");
        if(password_file == NULL) {
            perror("opening password file");
            exit(2);
        }

        int password_count = 0;

        // Read until we hit the end of the file
        while(!feof(password_file)) {
            // Make space to hold the username
            char username[MAX_USERNAME_LENGTH];

            // Make space to hold the MD5 string
            char md5_string[MD5_DIGEST_LENGTH * 2 + 1];

            // Make space to hold the MD5 bytes
            uint8_t password_hash[MD5_DIGEST_LENGTH];

            // Try to read. The space in the format string is required to eat the newline
            if(fscanf(password_file, "%s %s ", username, md5_string) != 2) {
                fprintf(stderr, "Error reading password file: malformed line\n");
                exit(2);
            }

            // Convert the MD5 string to MD5 bytes in our new node
            if(md5_string_to_bytes(md5_string, password_hash) != 0) {
                fprintf(stderr, "Error reading MD5\n");
                exit(2);
            }

            // Add the password to the password set
            add_password(&passwords, username, password_hash);
            password_count++;
        }
        // Now run the password list cracker
        int cracked = crack_password_list(&passwords);

        printf("Cracked %d of %d passwords.\n", cracked, password_count);

    } else {
        print_usage(argv[0]);
        exit(1);
    }

    return 0;
}
