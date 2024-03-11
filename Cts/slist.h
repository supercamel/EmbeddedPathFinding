/**
 * \file
 * \brief Header file for SList.
 *
 * SList is a singly linked list that provides functions to add, get, find and remove items.
 * SList supports appending elements at the end and prepending at the beginning of the list.
 * It allows for element access via index, finding the index of an element, and removing elements by index.
 * The list can also be sorted and reversed.
 *
 * The SList struct needs to be allocated with a CtsAllocator, which is used for allocating the list's internal structure.
 * The SList struct should be deallocated by calling slist_unref when it's no longer needed.
 *
 * Here's an example of how to use SList:
 * \code
 * cts_allocator_init_default();
 * CtsAllocator* alloc = cts_allocator_get_default();
 * SList* list = slist_new(alloc);
 *
 * // Append elements
 * int* num = cts_allocator_alloc(alloc, sizeof(int));
 * *num = 10;
 * slist_append(list, num);
 *
 * // Get an element
 * int *element = (int *)slist_get(list, 0);
 * printf("Element at index 0: %d\n", *element);
 *
 * // Clean up
 * slist_free_full(list, alloc, cts_allocator_free);
 * slist_unref(list);
 * \endcode
 */

#ifndef CTS_SLIST_H
#define CTS_SLIST_H

#include "object.h"

CTS_BEGIN_DECLARE_TYPE(CtsBase, CtsSList, cts_slist)
struct CtsSListPrivate* private;
CTS_END_DECLARE_TYPE(CtsSList, cts_slist)


CTS_BEGIN_DECLARE_TYPE(CtsBase, CtsSListIterator, cts_slist_iterator)
CtsSList* list;
struct CtsSListNode* current;
struct CtsSListNode* prev;
CTS_END_DECLARE_TYPE(CtsSListIterator, cts_slist_iterator)


typedef void (*SListFreeFunc)(cts_pointer alloc, cts_pointer data);
typedef int (*SListCompareFunc)(cts_pointer a, cts_pointer b);


bool cts_slist_prepend(CtsSList* self, cts_pointer obj);
bool cts_slist_append(CtsSList* self, cts_pointer obj);
bool cts_slist_insert(CtsSList* self, size_t index, cts_pointer obj);
cts_pointer cts_slist_get(CtsSList* self, size_t index);
size_t cts_slist_find(CtsSList* self, cts_pointer obj);
cts_pointer cts_slist_remove(CtsSList* self, size_t index);
cts_pointer cts_slist_remove_iter(CtsSList* self, CtsSListIterator* iter);
size_t cts_slist_get_length(CtsSList* self);
void cts_slist_sort(CtsSList* self, SListCompareFunc func);
void cts_slist_reverse(CtsSList* self);
void cts_slist_free(CtsSList* self);
void cts_slist_free_full(CtsSList* self, cts_pointer alloc, SListFreeFunc func);

/**
 * @brief Create a new SListIterator for the given SList.
 *
 * This function creates a new SListIterator that can be used to iterate over the elements of the given SList.
 * The SListIterator should be freed by calling slist_iterator_unref when it's no longer needed.
 * The SList will be kept alive as long as the SListIterator is alive.
 *
 * @param alloc The allocator to use for creating the iterator.
 * @param list The SList to create an iterator for.
 * @return A new SListIterator instance or NULL if an error occurred.
 */
CtsSListIterator* cts_slist_iterator_new_from_list(CtsAllocator* alloc, struct CtsSList* list);


cts_pointer cts_slist_iterator_peek(CtsSListIterator* self);
/**
 * @brief Get the next element from the SListIterator.
 *
 * This function returns the next element in the SList that the SListIterator was created for. If there are no more
 * elements, this function returns NULL.
 *
 * @param self The SListIterator to get the next element from.
 * @return The next element in the SList or NULL if there are no more elements.
 */
cts_pointer cts_slist_iterator_next(CtsSListIterator* self);

/**
 * @brief Check if the SListIterator has more elements.
 *
 * This function returns true if there are more elements to iterate over in the SList that the SListIterator was created
 * for, and false otherwise.
 *
 * @param self The SListIterator to check.
 * @return true if there are more elements in the SList, false otherwise.
 */
bool cts_slist_iterator_has_next(CtsSListIterator* self);

bool cts_slist_iterator_equals(CtsSListIterator* self, CtsSListIterator* other);


#endif
