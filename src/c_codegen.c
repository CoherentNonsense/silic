#include "c_codegen.h"

#include <stdint.h>
#include <iso646.h>

// C- Primitives
typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;
typedef __uint128_t u128;
typedef __int128_t i128;
typedef float f32;
typedef double f64;
typedef void* ptr;

void c_codegen_generate(Module* module) {
    /*
    FILE* out_file = fopen("program.c", "wb");
    if (out_file == NULL) {
	printf("Could not write file\n");
	return;
    }

    char* content = "int add(int a) {\nreturn a + 123;\n}";

    fwrite(content, strlen(content), 1, out_file);
    fclose(out_file);

    system("gcc -S -O1 program.c -o program.s");
    */
}

