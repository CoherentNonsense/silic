#ifndef COMPILER_H
#define COMPILER_H

#include "module.h"

Module* compiler_compile_module(String path, String source, bool build, bool debug_info);

#endif
