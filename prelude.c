// ------------- //
// C(IR) PRELUDE //
// ------------- //

#include <stdint.h>

typedef char u8;
typedef int32_t i32;

inline i32 SILICPRELUDE_addi32(i32 a, i32 b) {
    return a + b;
}

inline i32 SILICPRELUDE_subi32(i32 a, i32 b) {
    return a - b;
}

inline i32 SILICPRELUDE_muli32(i32 a, i32 b) {
    return a * b;
}

inline i32 SILICPRELUDE_divi32(i32 a, i32 b) {
    return a / b;
}
