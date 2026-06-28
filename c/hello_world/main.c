/**
 * @file   main.c
 * @brief  Minimal C program — prints "Hello, World!" to stdout.
 *
 * The simplest complete C program. Demonstrates the mandatory structure
 * every C program must have: a preprocessor include, a main() entry point,
 * a standard library call, and a return value to the OS.
 *
 * @section concepts Concepts Demonstrated
 * - `#include` — pulls in declarations from the C standard library header.
 * - `main()` — the single required entry point; the OS calls this on launch.
 * - `printf()` — formatted print to stdout (declared in <stdio.h>).
 * - `return 0` — convention: 0 means success, non-zero means error.
 *
 * @section compile Compile
 * @code
 *   make
 *   # or manually:
 *   clang -Wall -std=c11 main.c -o hello
 * @endcode
 *
 * @section nextsteps Next Steps
 * - Accept command-line arguments via `argc` / `argv`.
 * - Read from stdin with `scanf()` or `fgets()`.
 * - Return a non-zero exit code and observe it with `echo $?`.
 */

#include <stdio.h>

/**
 * @brief  Program entry point.
 * @return 0 on success (always succeeds).
 */
int main(void) {
    printf("Hello, World!\n");
    return 0;
}
