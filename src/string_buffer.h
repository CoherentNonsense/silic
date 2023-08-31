#ifndef STRING_H
#define STRING_H

#include <stddef.h>

typedef struct String {
    char* data;
    int length;
} String;

typedef struct Token Token;

String string_from_literal(char* literal);
String string_from_buffer(char* start, const size_t length);
String string_from_token(char* buffer, Token* token);
String string_copy(String a);
void string_delete(String a);
int string_compare(const String a, const String b);
int string_compare_literal(const String a, const char* b);

#endif // !STRING_H
