#include "slist.h"
#include <stdio.h>

typedef struct CtsSListNode
{
    cts_pointer obj;
    struct CtsSListNode *next;
} CtsSListNode;

typedef struct CtsSListPrivate
{
    struct CtsSListNode *head;
    struct CtsSListNode *end;
    size_t length;
} CtsSListPrivate;

CTS_DEFINE_TYPE(CtsBase, cts_base, CtsSList, cts_slist)

bool cts_slist_construct(CtsSList *self)
{
    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    self->private = cts_allocator_alloc(alloc, sizeof(CtsSListPrivate));
    if (self->private == NULL)
    {
        return false;
    }
    self->private->head = NULL;
    self->private->end = NULL;
    self->private->length = 0;
    return true;
}

void cts_slist_destruct(CtsSList *self)
{
    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    cts_slist_free(self);
    if (self->private != NULL)
    {
        cts_allocator_free(alloc, self->private);
    }
}

bool cts_slist_prepend(CtsSList *self, cts_pointer obj)
{
    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    CtsSListNode *node = cts_allocator_alloc(alloc, sizeof(CtsSListNode));
    if (node == NULL)
    {
        return false;
    }
    node->obj = obj;
    node->next = self->private->head;
    self->private->head = node;
    if (self->private->end == NULL)
    {
        self->private->end = node;
    }
    self->private->length++;
    return true;
}

bool cts_slist_append(CtsSList *self, cts_pointer obj)
{
    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    if (self->private == NULL)
    {
        return false;
    }

    CtsSListNode *node = cts_allocator_alloc(alloc, sizeof(CtsSListNode));
    if (node == NULL)
    {
        return false;
    }

    node->obj = obj;
    node->next = NULL;
    if (self->private->head == NULL)
    {
        self->private->head = node;
        self->private->end = node;
    }
    else
    {
        self->private->end->next = node;
        self->private->end = node;
    }
    self->private->length++;
    return true;
}

bool cts_slist_insert(CtsSList *self, size_t index, cts_pointer obj)
{
    if (self->private == NULL || self->private->length < index)
    {
        return false;
    }

    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    CtsSListNode *node = cts_allocator_alloc(alloc, sizeof(CtsSListNode));
    if (node == NULL)
    {
        return false;
    }
    node->obj = obj;

    if (index == 0)
    {
        node->next = self->private->head;
        self->private->head = node;
        if (self->private->end == NULL)
        {
            self->private->end = node;
        }
    }
    else
    {
        CtsSListNode *prev_node = self->private->head;
        for (size_t i = 0; i < index - 1; ++i)
        {
            prev_node = prev_node->next;
        }
        node->next = prev_node->next;
        prev_node->next = node;
    }
    self->private->length++;
    return true;
}

cts_pointer cts_slist_get(CtsSList *self, size_t index)
{
    // check size of list
    if (self->private != NULL && self->private->length >= index)
    {
        CtsSListNode *node = self->private->head;
        for (size_t i = 0; i < index; i++)
        {
            node = node->next;
        }
        return node->obj;
    }
    return NULL;
}

size_t slist_find(CtsSList *self, cts_pointer obj)
{
    CtsSListNode *node = self->private->head;
    size_t index = 0;
    while (node != NULL)
    {
        if (node->obj == obj)
        {
            return index;
        }
        node = node->next;
        ++index;
    }
    return (size_t)-1; // not found
}

cts_pointer cts_slist_remove(CtsSList *self, size_t index)
{
    if (self->private == NULL || self->private->length < index)
    {
        return NULL;
    }
    CtsSListNode *prev_node = NULL;
    CtsSListNode *node = self->private->head;
    for (size_t i = 0; i < index; ++i)
    {
        prev_node = node;
        node = node->next;
    }
    cts_pointer obj = node->obj;
    if (prev_node == NULL)
    {
        self->private->head = node->next;
    }
    else
    {
        prev_node->next = node->next;
    }
    if (node == self->private->end)
    {
        self->private->end = prev_node;
    }
    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    cts_allocator_free(alloc, node);
    self->private->length--;
    return obj;
}

cts_pointer cts_slist_remove_iter(CtsSList *self, CtsSListIterator *iter)
{
    CtsSListNode *prev_node = iter->prev;
    CtsSListNode *node = iter->current;
    CtsSListNode *next_node = node->next;

    cts_pointer obj = node->obj;
    if (prev_node == NULL)
    {
        self->private->head = node->next;
    }
    else
    {
        prev_node->next = node->next;
    }
    if (node == self->private->end)
    {
        self->private->end = prev_node;
    }

    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
    cts_allocator_free(alloc, node);

    self->private->length--;
    iter->current = next_node;

    return obj;
}

size_t cts_slist_get_length(CtsSList *self)
{
    return self->private->length;
}

void cts_slist_sort(CtsSList *self, SListCompareFunc func)
{
    if (self->private == NULL)
    {
        return;
    }
    if (self->private->length < 2)
    {
        return;
    }
    // Insertion sort
    CtsSListNode *sorted_end = self->private->head;
    while (sorted_end != self->private->end)
    {
        CtsSListNode *node = sorted_end->next;
        CtsSListNode *insert_after = NULL;
        CtsSListNode *sorted_node = self->private->head;
        while (sorted_node != sorted_end)
        {
            if (func(sorted_node->obj, node->obj) > 0)
            {
                break;
            }
            insert_after = sorted_node;
            sorted_node = sorted_node->next;
        }
        sorted_end->next = node->next;
        node->next = insert_after == NULL ? self->private->head : insert_after->next;
        if (insert_after == NULL)
        {
            self->private->head = node;
        }
        else
        {
            insert_after->next = node;
        }
        if (sorted_end->next == NULL)
        {
            self->private->end = sorted_end;
        }
    }
}

void cts_slist_reverse(CtsSList *self)
{
    CtsSListNode *node = self->private->head;
    CtsSListNode *prev_node = NULL;
    while (node != NULL)
    {
        CtsSListNode *next_node = node->next;
        node->next = prev_node;
        prev_node = node;
        node = next_node;
    }
    CtsSListNode *old_end = self->private->end;
    self->private->end = self->private->head;
    self->private->head = old_end;
}

void cts_slist_free(CtsSList *self)
{
    CtsAllocator *list_alloc = cts_base_get_allocator((CtsBase *)self);
    if (self->private != NULL)
    {
        CtsSListNode *node = self->private->head;
        while (node != NULL)
        {
            CtsSListNode *next = node->next;
            cts_allocator_free(list_alloc, node);
            node = next;
        }
        self->private->head = NULL;
        self->private->end = NULL;
    }
}

void cts_slist_free_full(CtsSList *self, cts_pointer alloc, SListFreeFunc func)
{
    CtsAllocator *list_alloc = cts_base_get_allocator((CtsBase *)self);
    if (self->private != NULL)
    {
        CtsSListNode *node = self->private->head;
        while (node != NULL)
        {
            CtsSListNode *next = node->next;
            if(func != NULL)
                func(alloc, node->obj);
            cts_allocator_free(list_alloc, node);
            node = next;
        }
        self->private->head = NULL;
        self->private->end = NULL;
    }
}

CTS_DEFINE_TYPE(CtsBase, cts_base, CtsSListIterator, cts_slist_iterator)

bool cts_slist_iterator_construct(CtsSListIterator *self)
{
    self->list = NULL;
    self->current = NULL;
    self->prev = NULL;
    return true;
}

void cts_slist_iterator_destruct(CtsSListIterator *self)
{
    if (self->list != NULL)
    {
        cts_slist_unref(self->list);
    }
    self->list = NULL;
    self->current = NULL;
}

CtsSListIterator *cts_slist_iterator_new_from_list(CtsAllocator *alloc, CtsSList *list)
{
    CtsSListIterator *self = cts_slist_iterator_new(alloc);
    if (self == NULL)
    {
        return NULL;
    }
    cts_slist_ref(list);
    self->list = list;
    self->current = list->private->head;
    return self;
}

cts_pointer cts_slist_iterator_peek(CtsSListIterator *self)
{
    return self->current->obj;
}

cts_pointer cts_slist_iterator_next(CtsSListIterator *self)
{
    if (self->current == NULL)
    {
        return NULL;
    }
    cts_pointer obj = self->current->obj;
    self->prev = self->current;
    self->current = self->current->next;
    return obj;
}

bool cts_slist_iterator_has_next(CtsSListIterator *self)
{
    return self->current != NULL;
}

bool cts_slist_iterator_equals(CtsSListIterator *self, CtsSListIterator *other)
{
    if((self->current == other->current) && (self->prev == other->prev)) {
        return true;
    }
    return false;
}
