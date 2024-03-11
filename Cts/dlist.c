#include <stdint.h>
#include "dlist.h"

typedef struct CtsDListNode 
{
    cts_pointer obj;
    struct CtsDListNode *next;
    struct CtsDListNode* prev;
} CtsDListNode;

typedef struct CtsDListPrivate
{
    struct CtsDListNode *head;
    struct CtsDListNode* end;
    size_t length;
} CtsDListPrivate;



CTS_DEFINE_TYPE(CtsBase, cts_base, CtsDList, cts_dlist)

bool cts_dlist_construct(CtsDList *self)
{
    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    self->private = cts_allocator_alloc(alloc, sizeof(CtsDListPrivate));
    if (self->private == NULL)
    {
        return false;
    }
    self->private->head = NULL;
    self->private->end = NULL;
    self->private->length = 0;
    return true;
}

void cts_dlist_destruct(CtsDList *self)
{
    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    cts_dlist_free(self);
    if (self->private != NULL)
    {
        alloc->free(alloc, self->private);
    }
}

bool cts_dlist_prepend(CtsDList* self, cts_pointer obj) {
    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    CtsDListNode* node = cts_allocator_alloc(alloc, sizeof(CtsDListNode));
    if (node == NULL)
    {
        return false;
    }
    node->obj = obj;
    node->next = self->private->head;
    node->prev = NULL;

    if (self->private->head != NULL)
    {
        self->private->head->prev = node;
    }
    self->private->head = node;

    if (self->private->end == NULL)
    {
        self->private->end = node;
    }
    self->private->length++;
    return true;
}

bool cts_dlist_append(CtsDList* self, cts_pointer obj) {
    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    if (self->private == NULL)
    {
        return false;
    }

    CtsDListNode* node = cts_allocator_alloc(alloc, sizeof(CtsDListNode));
    if (node == NULL)
    {
        return false;
    }

    node->obj = obj;
    node->next = NULL;
    node->prev = self->private->end;

    if (self->private->head == NULL)
    {
        self->private->head = node;
    }
    else 
    {
        self->private->end->next = node;
    }
    self->private->end = node;
    self->private->length++;
    return true;
}

bool cts_dlist_insert(CtsDList* self, size_t index, cts_pointer obj) {
    if(self->private == NULL || self->private->length < index) {
        return false;
    }

    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    CtsDListNode* node = cts_allocator_alloc(alloc, sizeof(CtsDListNode));
    if (node == NULL)
    {
        return false;
    }
    node->obj = obj;

    if(index == 0){
        node->next = self->private->head;
        node->prev = NULL;
        if (self->private->head != NULL)
        {
            self->private->head->prev = node;
        }
        self->private->head = node;
        if (self->private->end == NULL)
        {
            self->private->end = node;
        }
    } else {
        CtsDListNode* prev_node = self->private->head;
        for(size_t i = 0; i < index - 1; ++i){
            prev_node = prev_node->next;
        }
        node->next = prev_node->next;
        node->prev = prev_node;
        if (prev_node->next != NULL)
        {
            prev_node->next->prev = node;
        }
        prev_node->next = node;
    }
    self->private->length++;
    return true;
}


cts_pointer cts_dlist_get(CtsDList* self, size_t index) {
    if(self->private == NULL || self->private->length <= index) {
        return NULL;
    }

    CtsDListNode* node = self->private->head;
    for(size_t i = 0; i < index; ++i){
        node = node->next;
    }
    return node->obj;
}

size_t cts_dlist_find(CtsDList* self, cts_pointer obj) {
    CtsDListNode* node = self->private->head;
    size_t index = 0;
    while (node != NULL) {
        if(node->obj == obj) {
            return index;
        }
        node = node->next;
        index++;
    }
    return SIZE_MAX; // Return maximum size_t value if object not found
}

cts_pointer cts_dlist_remove(CtsDList* self, size_t index) {
    if(self->private == NULL || self->private->length <= index) {
        return NULL;
    }

    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    CtsDListNode* node = self->private->head;
    for(size_t i = 0; i < index; ++i){
        node = node->next;
    }

    cts_pointer obj = node->obj;

    if(node->prev != NULL){
        node->prev->next = node->next;
    } else {
        self->private->head = node->next;
    }

    if(node->next != NULL){
        node->next->prev = node->prev;
    } else {
        self->private->end = node->prev;
    }

    alloc->free(alloc, node);
    self->private->length--;

    return obj;
}

size_t cts_dlist_get_length(CtsDList* self) {
    return self->private->length;
}

void cts_dlist_sort(CtsDList* self, DListCompareFunc func)
{
    if(self->private == NULL) {
        return;
    }
    if(self->private->length < 2){
        return;
    }
    // Insertion sort
    CtsDListNode* sorted_end = self->private->head;
    while(sorted_end != self->private->end){
        CtsDListNode* node = sorted_end->next;
        CtsDListNode* insert_after = NULL;
        CtsDListNode* sorted_node = self->private->head;
        while(sorted_node != sorted_end){
            if(func(sorted_node->obj, node->obj) > 0){
                break;
            }
            insert_after = sorted_node;
            sorted_node = sorted_node->next;
        }
        sorted_end->next = node->next;
        if (node->next != NULL) {
            node->next->prev = sorted_end;
        }
        node->next = insert_after == NULL ? self->private->head : insert_after->next;
        node->prev = insert_after;

        if(node->next != NULL){
            node->next->prev = node;
        }

        if(insert_after == NULL){
            self->private->head = node;
        }else{
            insert_after->next = node;
        }
        if(sorted_end->next == NULL){
            self->private->end = sorted_end;
        }
    }
}

void cts_dlist_reverse(CtsDList* self) {
    CtsDListNode* node = self->private->head;
    CtsDListNode* tmp = NULL;

    while(node != NULL){
        tmp = node->prev;
        node->prev = node->next;
        node->next = tmp;
        node = node->prev;
    }

    if(tmp != NULL){
        self->private->head = tmp->prev;
    }
}

void cts_dlist_free(CtsDList* self) {
    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    CtsDListNode* node = self->private->head;
    CtsDListNode* next = NULL;

    while(node != NULL){
        next = node->next;
        alloc->free(alloc, node);
        node = next;
    }

    self->private->head = NULL;
    self->private->end = NULL;
    self->private->length = 0;
}

void cts_dlist_free_full(CtsDList* self, cts_pointer p_alloc, DListFreeFunc func) {
    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    CtsDListNode* node = self->private->head;
    CtsDListNode* next = NULL;

    while(node != NULL){
        next = node->next;
        func(p_alloc, node->obj);
        cts_allocator_free(alloc, node);
        node = next;
    }

    self->private->head = NULL;
    self->private->end = NULL;
    self->private->length = 0;
}
