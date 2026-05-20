.section .text.boot
.global _start
.type _start, %function
_start:
    /* Set up stack pointer */
    ldr x0, =__stack_top
    mov sp, x0

    /* Zero BSS */
    ldr x0, =__bss_start
    ldr x1, =__bss_end
bss_clear:
    cmp x0, x1
    b.ge bss_done
    str xzr, [x0], #8
    b bss_clear
bss_done:
    bl kmain

hang:
    wfe
    b hang
