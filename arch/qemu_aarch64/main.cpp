#include <stdint.h>

#define UART0 ((volatile char*)0x09000000)

extern "C" void kmain(void) {
    const char* msg = "hello world\n";

    while (*msg) {
        *UART0 = *msg++;
    }

    while (1) {
    }
}
