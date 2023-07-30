#ifndef CODEGEN_ERROR_MESSAGE_H
#define CODEGEN_ERROR_MESSAGE_H

#include "lexer/lexer.h"
#include "string_buffer.h"

typedef struct ErrorMessage {
    String message;
    TextPosition position;
} ErrorMessage;

#endif
