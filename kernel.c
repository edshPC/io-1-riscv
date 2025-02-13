#include "kernel.h"

extern char __bss[], __bss_end[], __stack_top[];

struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long fid, long eid) {
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    register long a7 __asm__("a7") = eid;

    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                           "r"(a6), "r"(a7)
                         : "memory");
    return (struct sbiret){.error = a0, .value = a1};
}

void putchar(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1 /* Console Putchar */);
}

char getchar(void) {
    char c = -1;
    while (c == (char)-1) {
        c = sbi_call(0, 0, 0, 0, 0, 0, 0, 2).error;
    }
    putchar(c);
    return c;
}

void getstring(char* buf) {
    int i = 0;
    char c = getchar();
    while (c != 13) {
        buf[i++] = c;
        c = getchar();
    }
    buf[i] = 0;
    putchar('\n');
}

void putstring(const char* s) {
    for (int i = 0; s[i] != '\0'; i++) {
        putchar(s[i]);
    }
}

void putnumber(long num) {
    if (num < 0) {
        putchar('-');
        num = -num;
    } else if (num == 0) return putchar('0');
    char buf[64];
    int i = 0;
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    while (i > 0) {
        putchar(buf[--i]);
    }
}

long getnumber(void) {
    char buf[16];
    getstring(buf);
    long res = 0;
    for (int i = 0; buf[i] != '\0'; i++) {
        res = res * 10 + (buf[i] - '0');
    }
    return res;
}

void kernel_main(void) {
    putstring("\n\nPick function:\n"
"1. Get SBI specification version\n"
"2. Get number of counters\n"
"3. Get details of a counter\n"
"4. System Shutdown\n");
    long res, minor, major;
    
    for (;;) {
        putstring("\n> ");
        char c = getchar();
        putchar('\n');
        switch (c) {
        case '1':
            res = sbi_call(0, 0, 0, 0, 0, 0, 0, 0x10).value;
            minor = res & 0xFFFFFF;
            major = (res & 0x7F000000) >> 24;
            putstring("version: ");
            putnumber(major);
            putchar('.');
            putnumber(minor);
            break;
        case '2':
            res = sbi_call(0, 0, 0, 0, 0, 0, 0, 0x504D55).value;
            putstring("number of counters: ");
            putnumber(res);
            break;
        case '3':
            res = getnumber();
            break;
        case '4':
            putstring("exiting...\n");
            __asm__ __volatile__("wfi");
        default:
            break;
        }
        //__asm__ __volatile__("wfi");
    }
}

__attribute__((section(".text.boot")))
__attribute__((naked))
void boot(void) {
    __asm__ __volatile__(
        "mv sp, %[stack_top]\n" // Устанавливаем указатель стека
        "j kernel_main\n"       // Переходим к функции main ядра
        :
        : [stack_top] "r" (__stack_top) // Передаём верхний адрес стека в виде %[stack_top]
    );
}
