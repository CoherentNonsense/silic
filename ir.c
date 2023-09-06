// ------------------- //
// C(IR) PRELUDE START //
// ------------------- //

#include <stdint.h>

typedef char u8;
typedef int32_t i32;

i32 addi32(i32 a, i32 b) {
    return a + b;
}

i32 subi32(i32 a, i32 b) {
    return a - b;
}

i32 muli32(i32 a, i32 b) {
    return a * b;
}

i32 divi32(i32 a, i32 b) {
    return a / b;
}

// ----------------- //
// C(IR) PRELUDE END //
// ----------------- //

i32 puts(u8* message);
void exit(i32 status);
i32 main();

i32 main() {
    puts("Hello, world!");
    exit(0);
}