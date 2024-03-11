#ifndef CST_STRING_H
#define CST_STRING_H

#include <stdbool.h>
#include <stdarg.h>
#include "allocator.h"

bool cts_isspace(char c);
char* cts_strdup(CtsAllocator* alloc, const char* str);
char* cts_strcpy(char* dst, const char* src);
char* cts_strncpy(char* dst, const char* src, size_t n);
char* cts_strcat(char* dst, const char* src);
size_t cts_strlen(const char* str);
char* cts_strtok(char* str, const char* delimiters);
int cts_sprintf(CtsAllocator* alloc, char** str, const char* format, ...);
char* cts_strstr(CtsAllocator* alloc, const char* haystack, const char* needle);
char* cts_strchr(CtsAllocator* alloc, const char* s, int c);
int cts_strcmp(const char* s1, const char* s2);
int cts_strncmp(const char* s1, const char* s2, int n);
char* cts_strchug(CtsAllocator* alloc, const char* str);
char* cts_strchomp(CtsAllocator* alloc, const char* str);
char* cts_strstrip(CtsAllocator* alloc, const char* str);


#endif

