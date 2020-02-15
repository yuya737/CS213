#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_contents(uint8_t* data, size_t size);

int main(int argc, char** argv) {
    // Make sure we have a file input
    if(argc != 2) {
        fprintf(stderr, "Please specify an input filename.\n");
        exit(1);
    }

    // Try to open the file
    FILE* input = fopen(argv[1], "r");
    if(input == NULL) {
        perror("Unable to open input file");
        exit(1);
    }

    // Seek to the end of the file so we can get its size
    if(fseek(input, 0, SEEK_END) != 0) {
        perror("Unable to seek to end of file");
        exit(2);
    }

    // Get the size of the file
    size_t size = ftell(input);

    // Seek back to the beginning of the file
    if(fseek(input, 0, SEEK_SET) != 0) {
        perror("Unable to seek to beginning of file");
        exit(2);
    }

    // Allocate a buffer to hold the file contents. We know the size in bytes, so
    // there's no need to multiply to get the size we pass to malloc in this case.
    uint8_t* data = malloc(size);

    // Read the file contents
    if(fread(data, 1, size, input) != size) {
        fprintf(stderr, "Failed to read entire file\n");
        exit(2);
    }

    // Make sure the file starts with the .ar file signature
    if(memcmp(data, "!<arch>\n", 8) != 0) {
        fprintf(stderr, "Input file is not in valid .ar format\n");
        exit(1);
    }

    // Call the code to print the archive contents
    print_contents(data, size);

    // Clean up
    free(data);
    fclose(input);

    return 0;
}

/**
 * This function should print the name of each file in the archive followed by its contents.
 *
 * \param data This is a pointer to the first byte in the file.
 * \param size This is the number of bytes in the file.
 */
void print_contents(uint8_t* data, size_t size) {
    //char identifier[16];
    //int timestamp;
    //int ownerID;
    //int groupID;
    //int fileSize;

    //struct __attribute__((packed)) dataStruct{
    //  char identifier[16];
    // char fileContent[fileSize];
    //};

    // Skip the first 8 characters
    data = data + 8;
    /*
    int count = 0;
    while(*data !=0 && count<500){
        printf("count: %d ascii: %d, data:%c\n",count,*data, (char) *data);
        count++;
        data++;
    }

    printf("count: %d ascii: %d, data:%c\n",count,*data, (char) *data);
    return;
    */ 

    while (*data != 0){
        // Allocate space for file identifier
        char identifier[17];
        identifier[16] = '\0';
        memmove(identifier, data, 16);
        if (strlen(identifier) != 16) break; 
        // Find the backslash and change to null terminating character
        char* endChar = strchr(identifier, '/');
        *endChar = '\0';
        printf("%s\n", identifier);

        // Move to File size, we don't care about the other stuff
        data = data + 48;

        // File size is at most 10 bytes
        char fileSize[11];
        fileSize[10] = '\0';
        memmove(fileSize, data, 10);
        int sizeOfFile = atoi(fileSize);

        // Move to File data
        data = data + 12;
        char file[sizeOfFile+1];
        file[sizeOfFile] = '\0';
        memmove(file, data, sizeOfFile);
        printf("%s\n", file);

        //printf("SizeofFile: %d\n", sizeOfFile);
        data = data + sizeOfFile;
        if (sizeOfFile % 2 == 1) data++;
        //printf("%s\n", data);

        //printf("count: %d data:%d\n",count, *data);
    }
    return;
}
