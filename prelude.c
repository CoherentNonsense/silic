// ------------- //
// C(IR) PRELUDE //
// ------------- //
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

typedef uint8_t u8;
typedef int32_t i32;
typedef int64_t i64;
typedef char c_char;
typedef size_t usize;
typedef ssize_t isize;

// instructions
static void wri32(i32* a, i32 b) {
    *a = b;
}

static i32 addi32(i32 a, i32 b) {
    return a + b;
}

static i32 subi32(i32 a, i32 b) {
    return a - b;
}

static i32 muli32(i32 a, i32 b) {
    return a * b;
}

static i32 divi32(i32 a, i32 b) {
    return a / b;
}

static bool eqi32(i32 a, i32 b) {
    return a == b;
}

static bool neqi32(i32 a, i32 b) {
    return a != b;
}

static bool gti32(i32 a, i32 b) {
    return a > b;
}

static bool lti32(i32 a, i32 b) {
    return a < b;
}

static bool and(bool a, bool b) {
    return a && b;
}

static bool or(bool a, bool b) {
    return a || b;
}


// ------------ //
// PROGRAM CODE //
// ------------ //
