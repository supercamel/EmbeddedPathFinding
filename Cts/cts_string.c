#include "cts_string.h"
#include <stdlib.h>
#include <stdio.h>

bool cts_isspace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

char* cts_strdup(CtsAllocator* alloc, const char* str) {
    size_t len = cts_strlen(str) + 1;
    char *new_str = (char*)cts_allocator_alloc(alloc, len);
    if (new_str == NULL)
        return NULL;
    return cts_strncpy(new_str, str, len);
}

char* cts_strcpy(char* dst, const char* src) {
    while ((*dst++ = *src++));
    return dst;
}

char* cts_strncpy(char* dst, const char* src, size_t n) {
    char* dst_orig = dst;
    while (n > 0 && *src != '\0') {
        *dst++ = *src++;
        --n;
    }
    while (n > 0) {
        *dst++ = '\0';
        --n;
    }
    return dst_orig;
}


char* cts_strcat(char* dst, const char* src) {
    while (*dst) dst++;
    while ((*dst++ = *src++));
    return dst;
}

size_t cts_strlen(const char* str) {
    const char *s;
    for (s = str; *s; ++s);
    return s - str;
}

char* cts_strtok(char* str, const char* delimiters) {
    static char* next_token = NULL;

    if (str != NULL) 
        next_token = str;

    if (next_token == NULL)
        return NULL;

    // Skip over leading delimiters
    while (*next_token) {
        const char* d = delimiters;
        do {
            if (*d == *next_token) {
                ++next_token;
                d = delimiters;
                continue;
            }
            ++d;
        } while (*d);
        break;
    }

    str = next_token;

    while (*next_token) {
        const char* d = delimiters;
        do {
            if (*d == *next_token) {
                *next_token = '\0';
                ++next_token;
                return str;
            }
            ++d;
        } while (*d);
        ++next_token;
    }

    next_token = NULL;
    return str;
}

int cts_sprintf(CtsAllocator* alloc, char** str, const char* format, ...) {
    va_list args;
    va_start(args, format);
    size_t size = vsnprintf(NULL, 0, format, args);
    va_end(args);

    *str = (char*)cts_allocator_alloc(alloc, size + 1);
    if (*str == NULL) 
        return -1;

    va_start(args, format);
    vsnprintf(*str, size + 1, format, args);
    va_end(args);

    return (int)size;
}

char* cts_strstr(CtsAllocator* alloc, const char* haystack, const char* needle) {
    char* found = cts_strstr(alloc, haystack, needle);
    if (found != NULL) {
        return cts_strdup(alloc, found);
    }
    return NULL;
}

char* cts_strchr(CtsAllocator* alloc, const char* s, int c) {
    char* found = cts_strchr(alloc, s, c);
    if (found != NULL) {
        return cts_strdup(alloc, found);
    }
    return NULL;
}

int cts_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }

    if (*s1 < *s2)
        return -1;
    else if (*s1 > *s2)
        return 1;
    else
        return 0;
}

int cts_strncmp(const char* s1, const char* s2, int n) {
    while (n-- && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }

    if (n == -1) {
        return 0;
    } else if (*s1 < *s2)
        return -1;
    else if (*s1 > *s2)
        return 1;
    else
        return 0;
}

char* cts_strchug(CtsAllocator* alloc, const char* str) {
    while (cts_isspace((unsigned char)*str)) {
        ++str;
    }

    return cts_strdup(alloc, str);
}

char* cts_strchomp(CtsAllocator* alloc, const char* str) {
    size_t len = cts_strlen(str);
    while (len > 0 && cts_isspace((unsigned char)str[len - 1])) {
        --len;
    }

    char* new_str = (char*)cts_allocator_alloc(alloc, len + 1);
    if (new_str == NULL) 
        return NULL;

    cts_strncpy(new_str, str, len);
    new_str[len] = '\0';
    return new_str;
}

char* cts_strstrip(CtsAllocator* alloc, const char* str) {
    const char* end;
    size_t len;

    while (cts_isspace((unsigned char)*str)) {
        ++str;
    }

    if (*str == 0) {
        return cts_strdup(alloc, str);
    }

    end = str + cts_strlen(str) - 1;
    while (end > str && cts_isspace((unsigned char)*end)) {
        --end;
    }

    end++;

    len = end - str;
    char* new_str = (char*)cts_allocator_alloc(alloc, len + 1);
    if (new_str == NULL) 
        return NULL;

    cts_strncpy(new_str, str, len);
    new_str[len] = '\0';
    return new_str;
}

