/*
 * CST_DLIST_H
 *
 * CtsDList is a doubly-linked list implementation. It provides operations to perform actions
 * such as adding elements to the front or back of the list, inserting elements at a specific
 * position, getting elements, finding elements, removing elements, sorting and reversing the list.
 * In addition to these, the CtsDList library includes an iterator, CtsDListIterator, which can be 
 * used to traverse the list in both directions.
 *
 * Both CtsDList and CtsDListIterator are objects inherited from CtsBase. Therefore, they can be managed 
 * using the generic cts_base_unref() function as well, although specific functions are provided for better clarity.
 *
 * It is important to note that the cts_dlist_remove() function passes ownership of the removed item to the caller. 
 * This means that it is the responsibility of the caller to properly free the item.
 *
 * Example usage:
 * 
 * // Initialize default allocator
 * cts_allocator_init_default();
 * CtsAllocator* alloc = cts_allocator_get_default();
 * 
 * // Create a new double-linked list
 * CtsDList* dlist = cts_dlist_new(alloc);
 * 
 * // Append some elements
 * int* num = cts_allocator_alloc(alloc, sizeof(int));
 * *num = 10;
 * cts_dlist_append(dlist, num);
 * 
 * // Traverse using an iterator
 * CtsDListIterator* iter = cts_dlist_iterator_new_from_list_head(alloc, dlist);
 * while (cts_dlist_iterator_has_next(iter)) {
 *     int* val = (int*)cts_dlist_iterator_next(iter);
 *     printf("Value: %d\n", *val);
 * }
 * 
 * // Clean up
 * cts_dlist_iterator_unref(iter);
 * cts_dlist_free_full(dlist, alloc, (DListFreeFunc)cts_allocator_free);
 * cts_dlist_unref(dlist);
 *
 */

#ifndef CST_DLIST_H
#define CST_DLIST_H


#include "object.h"

CTS_BEGIN_DECLARE_TYPE(CtsBase, CtsDList, cts_dlist)
struct CtsDListPrivate* private;
CTS_END_DECLARE_TYPE(CtsDList, cts_dlist)

typedef void (*DListFreeFunc)(cts_pointer alloc, cts_pointer data);
typedef int (*DListCompareFunc)(cts_pointer a, cts_pointer b);

bool cts_dlist_prepend(CtsDList* self, cts_pointer obj);
bool cts_dlist_append(CtsDList* self, cts_pointer obj);
bool cts_dlist_insert(CtsDList* self, size_t index, cts_pointer obj);
cts_pointer cts_dlist_get(CtsDList* self, size_t index);
size_t cts_dlist_find(CtsDList* self, cts_pointer obj);
cts_pointer cts_dlist_remove(CtsDList* self, size_t index);
size_t cts_dlist_get_length(CtsDList* self);
void cts_dlist_sort(CtsDList* self, DListCompareFunc func);
void cts_dlist_reverse(CtsDList* self);
void cts_dlist_free(CtsDList* self);
void cts_dlist_free_full(CtsDList* self, cts_pointer alloc, DListFreeFunc func);



CTS_BEGIN_DECLARE_TYPE(CtsBase, CtsDListIterator, cts_d_list_iterator)
CtsDList* list;
struct CtsDListNode* current;
CTS_END_DECLARE_TYPE(CtsDListIterator, cts_d_list_iterator)

CtsDListIterator* cts_dlist_iterator_new_from_list_head(CtsAllocator* alloc, struct CtsDList* list);
CtsDListIterator* cts_dlist_iterator_new_from_list_tail(CtsAllocator* alloc, struct CtsDList* list);
cts_pointer cts_dlist_iterator_next(CtsDListIterator* self);
cts_pointer cts_dlist_iterator_prev(CtsDListIterator* self);
bool cts_dlist_iterator_has_next(CtsDListIterator* self);
bool cts_dlist_iterator_has_prev(CtsDListIterator* self);

#endif //CST_DLIST_H