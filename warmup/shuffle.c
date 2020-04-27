#include <stdio.h>
#include <stdlib.h>

// Here are some handy C preprocessor definitions for suits of cards
// Source: http://stackoverflow.com/questions/27133508/how-to-print-spades-hearts-diamonds-etc-in-c-and-linux
#if defined(_WIN32) || defined(__MSDOS__)
   #define SPADE   "\x06"
   #define CLUB    "\x05"
   #define HEART   "\x03"
   #define DIAMOND "\x04"
#else
   #define SPADE   "\xE2\x99\xA0"
   #define CLUB    "\xE2\x99\xA3"
   #define HEART   "\xE2\x99\xA5"
   #define DIAMOND "\xE2\x99\xA6"
#endif

int main(int argc, char** argv) {
  // TODO: Shuffle a deck of cards and print each card
  
  // These demo printouts do something horrible with the C preprocessor.
  // Adjacent strings in C are just combined into one.
  // You should remove these lines, but they may be helpful for setting up your own printing code
  printf("King of spades: K" SPADE "\n");
  printf("Two of clubs: 2" CLUB "\n");
  printf("Ace of hearts: A" HEART "\n");
  printf("Jack of diamonds: J" DIAMOND "\n");
  return 0;
}
