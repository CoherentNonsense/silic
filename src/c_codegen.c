#include "c_codegen.h"

#include <iso646.h>

static char prelude_text[] =
    "// ------------------- //\n"
    "// C(IR) PRELUDE START //\n"
    "// ------------------- //\n"
    "\n"
    "#include <stdint.h>\n"
    "\n"
    "// Primitives\n"
    "typedef uint8_t u8;\n"
    "typedef int8_t i8;\n"
    "typedef uint16_t u16;\n"
    "typedef int16_t i16;\n"
    "typedef uint32_t u32;\n"
    "typedef int32_t i32;\n"
    "typedef uint64_t u64;\n"
    "typedef int64_t i64;\n"
    "typedef float f32;\n"
    "typedef double f64;\n"
    "typedef void* ptr;\n"
    "\n"
    "// ----------------- //\n"
    "// C(IR) PRELUDE END //\n"
    "// ----------------- //\n";

static size_t prelude_text_length = sizeof(prelude_text) - 1;

typedef struct CodegenContext {
    FILE* out_file;
} CodegenContext;

static void generate_prelude(CodegenContext* context) {
    fwrite(prelude_text, prelude_text_length, 1, context->out_file);
}

void c_codegen_generate(Module* module) {
    CodegenContext context;
    context.out_file = fopen("build/module.c", "wb");
    if (context.out_file == NULL) {
	printf("Could not open file\n");
	return;
    }

    generate_prelude(&context);

    fclose(context.out_file);

    //system("gcc -S -O1 program.c -o program.s");
}

