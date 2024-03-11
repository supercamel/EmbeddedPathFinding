#include "array.h"
#include <string.h>

typedef struct CtsArrayPrivate {
    cts_pointer* objs;
    size_t length;
    size_t reserved;
} CtsArrayPrivate;

CTS_DEFINE_TYPE(CtsBase, cts_base, CtsArray, cts_array)

bool cts_array_construct(CtsArray* self) {
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*)self);
    self->private = cts_allocator_alloc(alloc, sizeof(CtsArrayPrivate));
    if (self->private == NULL) {
        return false;
    }
    self->private->objs = NULL;
    self->private->length = 0;
    self->private->reserved = 0;
    return true;
}

void cts_array_destruct(CtsArray* self) {
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*)self);
    cts_array_free(self);
    if (self->private != NULL) {
        cts_allocator_free(alloc, self->private);
    }
}

bool cts_array_append(CtsArray* self, cts_pointer obj) 
{
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*)self);
    if(self->private == NULL) {
        return false;
    }
    if (self->private->length == self->private->reserved) {
        size_t new_reserved = self->private->reserved * 2;
        if (new_reserved == 0) {
            new_reserved = 1;
        }
        cts_pointer* new_objs = cts_allocator_alloc(alloc, sizeof(cts_pointer) * new_reserved);
        if (new_objs == NULL) {
            return false;
        }
        if (self->private->objs != NULL) {
            memcpy(new_objs, self->private->objs, sizeof(cts_pointer) * self->private->length);
            cts_allocator_free(alloc, self->private->objs);
        }
        self->private->objs = new_objs;
        self->private->reserved = new_reserved;
    }
    self->private->objs[self->private->length] = obj;
    self->private->length++;
    return true;
}

bool cts_array_insert(CtsArray* self, size_t index, cts_pointer obj)
{
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*)self);
    if (self->private == NULL) {
        return false;
    }
    if (index > self->private->length) {
        return false;
    }
    if (self->private->length == self->private->reserved) {
        size_t new_reserved = self->private->reserved * 2;
        if (new_reserved == 0) {
            new_reserved = 1;
        }
        cts_pointer* new_objs = cts_allocator_alloc(alloc, sizeof(cts_pointer) * new_reserved);
        if (new_objs == NULL) {
            return false;
        }
        if (self->private->objs != NULL) {
            memcpy(new_objs, self->private->objs, sizeof(cts_pointer) * self->private->length);
            cts_allocator_free(alloc, self->private->objs);
        }
        self->private->objs = new_objs;
        self->private->reserved = new_reserved;
    }
    if (index < self->private->length) {
        memmove(self->private->objs + index + 1, self->private->objs + index, sizeof(cts_pointer) * (self->private->length - index));
    }
    self->private->objs[index] = obj;
    self->private->length++;
    return true;
}

cts_pointer cts_array_remove_index(CtsArray* self, size_t index) 
{
    if (self->private == NULL) {
        return NULL;
    }
    if (index >= self->private->length) {
        return NULL;
    }
    cts_pointer obj = self->private->objs[index];
    if (index < self->private->length - 1) {
        memmove(self->private->objs + index, self->private->objs + index + 1, sizeof(cts_pointer) * (self->private->length - index - 1));
    }
    self->private->length--;
    return obj;
}

cts_pointer cts_array_get(CtsArray* self, size_t index) 
{ 
    if (self->private == NULL) {
        return NULL;
    }
    if (index >= self->private->length) {
        return NULL;
    }
    return self->private->objs[index];
}

cts_pointer cts_array_replace(CtsArray* self, size_t index, cts_pointer obj) 
{ 
    if (self->private == NULL) {
        return NULL;
    }
    if (index >= self->private->length) {
        return NULL;
    }
    cts_pointer old_obj = self->private->objs[index];
    self->private->objs[index] = obj;
    return old_obj;
}

void cts_array_reverse(CtsArray* self) 
{ 
    if (self->private == NULL) {
        return;
    }
    for (size_t i = 0; i < self->private->length / 2; i++) {
        cts_pointer tmp = self->private->objs[i];
        self->private->objs[i] = self->private->objs[self->private->length - i - 1];
        self->private->objs[self->private->length - i - 1] = tmp;
    }
}

size_t cts_array_get_length(CtsArray* self) 
{ 
    if (self->private == NULL) {
        return 0;
    }
    return self->private->length;
}

void cts_array_sort(CtsArray* self, ArrayCompareFunc func) 
{
    if (self->private == NULL) {
        return;
    }
    for (size_t i = 1; i < self->private->length; i++) {
        cts_pointer key = self->private->objs[i];
        int j = i - 1; 

        while (j >= 0 && func(self->private->objs[j], key) > 0) {
            self->private->objs[j + 1] = self->private->objs[j];
            j = j - 1;
        }
        self->private->objs[j + 1] = key;
    }
}

void cts_array_free(CtsArray* self)
{
    if (self->private == NULL) {
        return;
    }
    if (self->private->objs != NULL) {
        CtsAllocator* alloc = cts_base_get_allocator((CtsBase*)self);
        cts_allocator_free(alloc, self->private->objs);
        self->private->objs = NULL;
    }
    self->private->length = 0;
    self->private->reserved = 0;
}

void cts_array_free_full(CtsArray* self, CtsAllocator* alloc, ArrayFreeFunc func)
{
    if (self->private == NULL) {
        return;
    }
    if (func != NULL) {
        for (size_t i = 0; i < self->private->length; i++) {
            func(alloc, self->private->objs[i]);
        }
    }
    CtsAllocator* array_alloc = cts_base_get_allocator((CtsBase*)self);
    if (self->private->objs != NULL) {
        cts_allocator_free(array_alloc, self->private->objs);
        self->private->objs = NULL;
    }
    self->private->length = 0;
    self->private->reserved = 0;
}
