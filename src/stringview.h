#ifndef STRING_VIEW_H
#define STRING_VIEW_H

typedef struct StringView {
    char* buffer;
    int start;
    int length;
} StringView;

StringView string_from(StringView string);

#endif
