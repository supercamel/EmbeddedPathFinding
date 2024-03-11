#ifndef CTS_RB_TREE_H
#define CTS_RB_TREE_H 

#include "object.h"

// key compare function pointer definition
typedef int (*RbTreeCompareFunc)(const void*, const void*);

CTS_BEGIN_DECLARE_TYPE(CtsBase, CtsRbTree, cts_rb_tree)
struct CtsRbTreePrivate* priv;
CTS_END_DECLARE_TYPE(CtsRbTree, cts_rb_tree)

CtsRbTree* cts_rb_tree_new_full(CtsAllocator* alloc, RbTreeCompareFunc compare_func);
bool cts_rb_tree_insert(CtsRbTree* tree, cts_pointer key, cts_pointer value);
cts_pointer cts_rb_tree_remove(CtsRbTree* tree, cts_pointer key);
cts_pointer cts_rb_tree_get(CtsRbTree* tree, cts_pointer key);
bool cts_rb_tree_contains(CtsRbTree* tree, cts_pointer key);
size_t cts_rb_tree_size(CtsRbTree* tree);
void cts_rb_tree_clear(CtsRbTree* tree);
void cts_rb_tree_clear_full(CtsRbTree* tree, CtsFreeFunc key_free_func, cts_pointer key_user_pointer, CtsFreeFunc value_free_func, cts_pointer value_user_pointer);
cts_pointer cts_rb_tree_minimum(CtsRbTree* tree);
cts_pointer cts_rb_tree_maximum(CtsRbTree* tree);

CTS_BEGIN_DECLARE_TYPE(CtsBase, CtsRbTreeIterator, cts_rb_tree_iterator)
CtsRbTree* tree;
struct RbNode* current;
CTS_END_DECLARE_TYPE(CtsRbTreeIterator, cts_rb_tree_iterator)

CtsRbTreeIterator* cts_rb_tree_iterator_new_from_tree(CtsAllocator* alloc, CtsRbTree* tree);
cts_pointer cts_rb_tree_iterator_next(CtsRbTreeIterator* self);
bool cts_rb_tree_iterator_has_next(CtsRbTreeIterator* self);

CtsRbTreeIterator* cts_rb_tree_reverse_iterator_new_from_tree(CtsAllocator* alloc, CtsRbTree* tree);
cts_pointer cts_rb_tree_reverse_iterator_next(CtsRbTreeIterator* self);
bool cts_rb_tree_reverse_iterator_has_next(CtsRbTreeIterator* self);

#endif
