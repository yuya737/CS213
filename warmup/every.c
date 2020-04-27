#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

// This line defines a new type, fn_t, that is a function that takes no arguments.
// Function pointer syntax in C is scary, but the rest of it works a lot like Scheme.
typedef void (*fn_t)();

// Declaration of the every function, which calls `fn` every `interval` milliseconds `num_runs` times.
void every(size_t num_runs, size_t interval, fn_t fn);

// You may be a little alarmed to see the type `size_t`. This is just an integer of some sort, defined to be the "right" width (number of bits) to count things on the current machine. This type and many others like it are defined in stdint.h.

// This is a function that will be called at some regular interval
void some_action() {
  printf("Here I am!\n");
}

int main(int argc, char** argv) {
  // Use `every` to invoke `some_action` five times, once per second.
  every(5, 1000, some_action);
  return 0;
}

void every(size_t num_runs, size_t interval, fn_t fn) {
  // TODO: Invoke `fn` every `interval` milliseconds until it has run `num_runs` times
  // The next line calls the function passed in as `fn`
  fn();
}
