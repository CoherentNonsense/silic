#ifndef COMPILER_H
#define COMPILER_H

#include "module.h"

Module* compiler_compile_module(Span path, Span source, bool build, bool debug_info);

#endif
