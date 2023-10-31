// ------------- //
// C(IR) PRELUDE //
// ------------- //

#include <stdint.h>

typedef uint8_t u8;
typedef int32_t i32;
typedef char c_char;

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


// ------------ //
// PROGRAM CODE //
// ------------ //

