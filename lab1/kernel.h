#pragma once

struct sbiret {
    long error;
    union {
        long value;
        unsigned long uvalue;
    };
};

struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long fid, long eid);

void putchar(char ch);
char getchar(void);
void getstring(char* buf);
void putstring(const char* str);
void putnumber(long num);
long getnumber(void);
