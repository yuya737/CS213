#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
  // Make sure the program is run with an N parameter
  if(argc != 2) {
    fprintf(stderr, "Usage: %s N (N must be >= 1)\n", argv[0]);
    exit(1);
  }
  
  // Convert the N parameter to an integer
  int N = atoi(argv[1]);
  
  // Make sure N is >= 1
  if(N < 1) {
    fprintf(stderr, "Invalid N value %d\n", N);
    exit(1);
  }
  
  // TODO: read from standard input and print out ngrams until we reach the end of the input

  char string[N]; // char array to store current string to print
  char nextString[N]; // char array to store the manipulated string
  char nextChar; // the next char to be read


  if (fgets(string, N+1, stdin) != NULL){ 
      if (N > strlen(string)){ // if N is less than the length of input string return 
          return 0;
      }
      
      while ((nextChar = fgetc(stdin)) != EOF){ // while loop until the end of file
          puts(string); // print the current string
          memmove(nextString, string+1, N); // move all characters but the first to nextString
          memmove(nextString+N-1, &nextChar, 1); // append the current character to the end of nextString
          strcpy(string, nextString); // copy nextString to string
      }
      puts(string); // print the final string before exiting
  }
  return 0;
}
