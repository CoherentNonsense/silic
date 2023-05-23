#ifndef UTIL_H
#define UTIL_H

typedef struct Result {
    const enum {
        Ok,
        Error,
    } type;
    const char* msg;
} Result;

void result_print(Result result);

#define RESULT_OK (Result){ Ok, 0 }
#define RESULT_ERR(m) (Result){ Error, m }

void sil_panic(const char* format, ...)
    __attribute__((cold))
    __attribute__((format(printf, 1, 2)))
    __attribute__((noreturn));

#endif // !UTIL_H


