#ifndef UTIL_H
#define UTIL_H

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

typedef struct Result {
    enum {
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


// Maybe
#define Maybe(T) \
    struct { \
	enum { \
	    Yes, \
	    No, \
	} type; \
	T value; \
    }

#endif // !UTIL_H


