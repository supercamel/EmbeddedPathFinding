#include "rbtree.h"
#include "stack.h"

typedef enum { RED, BLACK } node_color;

typedef struct RbNode {
    void *key;                     // Node value, it's a pointer now
    void* value;
    node_color color;              // Node color (RED or BLACK)
    struct RbNode *left, *right, *parent;  // Pointers to left child, right child, and parent
} RbNode;

typedef struct CtsRbTreePrivate {
    RbNode* root;
    size_t size;
    RbTreeCompareFunc compare_func;
} CtsRbTreePrivate;

CTS_DEFINE_TYPE(CtsBase, cts_base, CtsRbTree, cts_rb_tree)

bool cts_rb_tree_construct(CtsRbTree *self) {
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*)self);
    self->priv = cts_allocator_alloc(alloc, sizeof(CtsRbTreePrivate));
    if(self->priv == NULL) {
        return false;
    }
    self->priv->root = NULL;
    self->priv->size = 0;
    return true;
}

void cts_rb_tree_destruct(CtsRbTree *self) {
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*)self);
    cts_rb_tree_clear(self);
    cts_allocator_free(alloc, self->priv);
}

CtsRbTree* cts_rb_tree_new_full(CtsAllocator* alloc, RbTreeCompareFunc compare_func)
{
    CtsRbTree* tree = cts_rb_tree_new(alloc);
    if(tree == NULL) {
        return NULL;
    }
    tree->priv->compare_func = compare_func;
    return tree;
}

static RbNode* minimum(RbNode* node) {
    while(node->left != NULL)
        node = node->left;
    return node;
}

static void leftRotate(CtsRbTree *tree, RbNode *x) {
    RbNode *y = x->right; // Set y
    x->right = y->left; // Turn y's left subtree into x's right subtree
    if (y->left != NULL) {
        y->left->parent = x;
    }
    y->parent = x->parent; // Link x's parent to y
    if (x->parent == NULL) { // x is root
        tree->priv->root = y;
    } else if (x == x->parent->left) { // x is left child
        x->parent->left = y;
    } else { // x is right child
        x->parent->right = y;
    }
    y->left = x; // Put x on y's left
    x->parent = y;
}

static void rightRotate(CtsRbTree *tree, RbNode *y) {
    RbNode *x = y->left; // Set x
    y->left = x->right; // Turn x's right subtree into y's left subtree
    if (x->right != NULL) {
        x->right->parent = y;
    }
    x->parent = y->parent; // Link y's parent to x
    if (y->parent == NULL) { // y is root
        tree->priv->root = x;
    } else if (y == y->parent->right) { // y is right child
        y->parent->right = x;
    } else { // y is left child
        y->parent->left = x;
    }
    x->right = y; // Put y on x's right
    y->parent = x;
}


static void insertFixUp(CtsRbTree *tree, RbNode *node) {
    while (node != tree->priv->root && node->parent->color == RED) {
        if (node->parent == node->parent->parent->left) {
            RbNode *uncle = node->parent->parent->right;
            if (uncle != NULL && uncle->color == RED) {
                // Case 1: Uncle is RED
                node->parent->color = BLACK;
                uncle->color = BLACK;
                node->parent->parent->color = RED;
                node = node->parent->parent;
            } else {
                if (node == node->parent->right) {
                    // Case 2: Uncle is BLACK and node is a right child
                    node = node->parent;
                    leftRotate(tree, node);
                }
                // Case 3: Uncle is BLACK and node is a left child
                node->parent->color = BLACK;
                node->parent->parent->color = RED;
                rightRotate(tree, node->parent->parent);
            }
        } else {
            // Same as above, but swapping "right" and "left"
            RbNode *uncle = node->parent->parent->left;
            if (uncle != NULL && uncle->color == RED) {
                node->parent->color = BLACK;
                uncle->color = BLACK;
                node->parent->parent->color = RED;
                node = node->parent->parent;
            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    rightRotate(tree, node);
                }
                node->parent->color = BLACK;
                node->parent->parent->color = RED;
                leftRotate(tree, node->parent->parent);
            }
        }
    }
    tree->priv->root->color = BLACK; // Root should always be black
}

static void deleteFixUp(CtsRbTree *tree, RbNode *x, RbNode *xParent) {
    while (x != tree->priv->root && (x == NULL || x->color == BLACK)) {
        if (x == xParent->left) {
            RbNode *w = xParent->right;
            if (w->color == RED) {
                w->color = BLACK;
                xParent->color = RED;
                leftRotate(tree, xParent);
                w = xParent->right;
            }
            if ((w->left == NULL || w->left->color == BLACK) && 
                (w->right == NULL || w->right->color == BLACK)) {
                w->color = RED;
                x = xParent;
                xParent = x->parent;
            } else {
                if (w->right == NULL || w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rightRotate(tree, w);
                    w = xParent->right;
                }
                w->color = xParent->color;
                xParent->color = BLACK;
                if (w->right) w->right->color = BLACK;
                leftRotate(tree, xParent);
                x = tree->priv->root;
            }
        } else {
            RbNode *w = xParent->left;
            if (w->color == RED) {
                w->color = BLACK;
                xParent->color = RED;
                rightRotate(tree, xParent);
                w = xParent->left;
            }
            if ((w->right == NULL || w->right->color == BLACK) && 
                (w->left == NULL || w->left->color == BLACK)) {
                w->color = RED;
                x = xParent;
                xParent = x->parent;
            } else {
                if (w->left == NULL || w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    leftRotate(tree, w);
                    w = xParent->left;
                }
                w->color = xParent->color;
                xParent->color = BLACK;
                if (w->left) w->left->color = BLACK;
                rightRotate(tree, xParent);
                x = tree->priv->root;
            }
        }
    }
    if (x) x->color = BLACK;
}


bool cts_rb_tree_insert(CtsRbTree* tree, cts_pointer key, cts_pointer value) {
    // Create new node
    RbNode *newNode = cts_allocator_alloc(cts_base_get_allocator((CtsBase*)tree), sizeof(RbNode));
    if (newNode == NULL) {
        return false;  // Allocation failed
    }

    newNode->key = key;
    newNode->value = value;
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->color = RED;  // New nodes are always red

    // Find the correct location to insert the new node
    RbNode *current = tree->priv->root;
    RbNode *parent = NULL;
    while (current != NULL) {
        parent = current;
        if (tree->priv->compare_func(key, current->key) < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    // Set the parent of the new node
    newNode->parent = parent;

    // Insert the new node
    if (parent == NULL) {
        // The tree was empty, so newNode becomes the root
        tree->priv->root = newNode;
    } else if (tree->priv->compare_func(key, parent->key) < 0) {
        parent->left = newNode;
    } else {
        parent->right = newNode;
    }

    // Update size
    tree->priv->size++;

    insertFixUp(tree, newNode);

    return true;
}

cts_pointer cts_rb_tree_remove(CtsRbTree* tree, cts_pointer key) {
    RbNode *node_to_remove = tree->priv->root;
    while(node_to_remove != NULL) {
        int cmp = tree->priv->compare_func(key, node_to_remove->key);
        if(cmp < 0)
            node_to_remove = node_to_remove->left;
        else if(cmp > 0)
            node_to_remove = node_to_remove->right;
        else
            break;
    }

    if(node_to_remove == NULL)
        return NULL;  // Key not found in the tree

    // Identify the node that will replace the node being removed
    RbNode *replacement_node;
    if(node_to_remove->left != NULL && node_to_remove->right != NULL) {
        replacement_node = minimum(node_to_remove->right);
    } else if(node_to_remove->left != NULL) {
        replacement_node = node_to_remove->left;
    } else if(node_to_remove->right != NULL) {
        replacement_node = node_to_remove->right;
    } else {
        replacement_node = NULL;
    }

    void* old_key = node_to_remove->key;
    RbNode *x, *xParent;
    node_color original_color = node_to_remove->color;

    if (node_to_remove->left == NULL || node_to_remove->right == NULL) {
        // Node to remove has zero or one children
        x = node_to_remove->left ? node_to_remove->left : node_to_remove->right;
        if (node_to_remove == tree->priv->root) {
            // Special case: root
            tree->priv->root = x;
            if (x) x->color = BLACK;
        } else {
            if (node_to_remove == node_to_remove->parent->left) {
                node_to_remove->parent->left = x;
            } else {
                node_to_remove->parent->right = x;
            }
            if (x) x->parent = node_to_remove->parent;
            if (original_color == BLACK) {
                xParent = x ? x->parent : node_to_remove->parent;
                deleteFixUp(tree, x, xParent);
            }
        }
    } else {
        // Node to remove has two children
        RbNode *successor = replacement_node;
        original_color = successor->color;
        x = successor->right;
        if (successor->parent == node_to_remove) {
            if (x) x->parent = successor;
            xParent = successor;
        } else {
            xParent = successor->parent;
            if (x) x->parent = successor->parent;
            successor->parent->left = x;
            successor->right = node_to_remove->right;
            node_to_remove->right->parent = successor;
        }
        if (node_to_remove == tree->priv->root) {
            tree->priv->root = successor;
        } else {
            if (node_to_remove == node_to_remove->parent->left) {
                node_to_remove->parent->left = successor;
            } else {
                node_to_remove->parent->right = successor;
            }
        }
        successor->parent = node_to_remove->parent;
        successor->color = node_to_remove->color;
        successor->left = node_to_remove->left;
        node_to_remove->left->parent = successor;
        if (original_color == BLACK) {
            deleteFixUp(tree, x, xParent);
        }
    }
    // Finally, delete the node_to_remove and reduce the size of the tree.
    cts_allocator_free(cts_base_get_allocator((CtsBase*)tree), node_to_remove);
    tree->priv->size--;
    return old_key;
}

cts_pointer cts_rb_tree_get(CtsRbTree* tree, cts_pointer key) {
    RbNode* current = tree->priv->root;
    while (current != NULL) {
        int cmp = tree->priv->compare_func(key, current->key);
        if (cmp < 0) {
            current = current->left;
        } else if (cmp > 0) {
            current = current->right;
        } else {
            // We've found the key
            return current->value;
        }
    }
    // The key was not found in the tree
    return NULL;
}

bool cts_rb_tree_contains(CtsRbTree* tree, cts_pointer key) {
    return cts_rb_tree_get(tree, key) != NULL;
}

size_t cts_rb_tree_size(CtsRbTree* tree)
{
    return tree->priv->size;
}

void cts_rb_tree_clear(CtsRbTree* tree) {
    cts_rb_tree_clear_full(tree, NULL, NULL, NULL, NULL); 
}

void cts_rb_tree_clear_full(CtsRbTree* tree, CtsFreeFunc key_free_func, cts_pointer key_ptr, CtsFreeFunc value_free_func, cts_pointer val_ptr)
{
    if(tree->priv->root == NULL)
        return;

    // Create a new stack to keep track of nodes to be visited
    CtsStack* stack = cts_stack_new(cts_base_get_allocator((CtsBase*)tree));

    // Start with the root
    cts_stack_push(stack, tree->priv->root);

    while(!cts_stack_is_empty(stack)) {
        RbNode* current_node = (RbNode*)cts_stack_pop(stack);
        if(current_node->left != NULL) {
            cts_stack_push(stack, current_node->left);
        }
        if(current_node->right != NULL) {
            cts_stack_push(stack, current_node->right);
        }
        if(key_free_func != NULL) {
            key_free_func(key_ptr, current_node->key);
        }
        if(value_free_func != NULL) {
            value_free_func(val_ptr, current_node->value);
        }
        // Deallocate the current node
        cts_allocator_free(cts_base_get_allocator((CtsBase*)tree), current_node);
    }
    
    // Deallocate the stack
    cts_stack_unref(stack);

    // Set the root to NULL and reset the size
    tree->priv->root = NULL;
    tree->priv->size = 0; 
}

cts_pointer cts_rb_tree_minimum(CtsRbTree* tree) {
    RbNode* node = tree->priv->root;
    if (node == NULL) {
        return NULL;
    }

    while (node->left != NULL) {
        node = node->left;
    }

    return node->value;
}

cts_pointer cts_rb_tree_maximum(CtsRbTree* tree) {
    RbNode* node = tree->priv->root;
    if (node == NULL) {
        return NULL;
    }

    while (node->right != NULL) {
        node = node->right;
    }

    return node->value;
}

// Find the node with the minimum key in the subtree rooted at 'node'
RbNode* cts_rb_node_minimum(RbNode* node) {
    if (node == NULL) {
        return NULL;
    }

    while (node->left != NULL) {
        node = node->left;
    }

    return node;
}

// Find the node with the maximum key in the subtree rooted at 'node'
RbNode* cts_rb_node_maximum(RbNode* node) {
    if (node == NULL) {
        return NULL;
    }

    while (node->right != NULL) {
        node = node->right;
    }

    return node;
}

// Find the in-order successor of 'node'
RbNode* cts_rb_node_successor(RbNode* node) {
    if (node == NULL) {
        return NULL;
    }

    // If node has a right child, the successor is the minimum node in the right subtree
    if (node->right != NULL) {
        return cts_rb_node_minimum(node->right);
    }

    // If node doesn't have a right child, then the successor is the lowest ancestor of node whose left child is also an ancestor of node
    RbNode* parent = node->parent;
    while (parent != NULL && node == parent->right) {
        node = parent;
        parent = parent->parent;
    }

    return parent;
}

// Find the in-order predecessor of 'node'
RbNode* cts_rb_node_predecessor(RbNode* node) {
    if (node == NULL) {
        return NULL;
    }

    // If node has a left child, the predecessor is the maximum node in the left subtree
    if (node->left != NULL) {
        return cts_rb_node_maximum(node->left);
    }

    // If node doesn't have a left child, then the predecessor is the lowest ancestor of node whose right child is also an ancestor of node
    RbNode* parent = node->parent;
    while (parent != NULL && node == parent->left) {
        node = parent;
        parent = parent->parent;
    }

    return parent;
}

CTS_DEFINE_TYPE(CtsBase, cts_base, CtsRbTreeIterator, cts_rb_tree_iterator)

bool cts_rb_tree_iterator_construct(CtsRbTreeIterator *self)
{
    self->tree = NULL;
    self->current = NULL;
    return true;
}

void cts_rb_tree_iterator_destruct(CtsRbTreeIterator *self)
{
    if(self->tree != NULL)
    {
        cts_rb_tree_unref(self->tree);
    }
    self->tree = NULL;
    self->current = NULL;
}

CtsRbTreeIterator* cts_rb_tree_iterator_new_from_tree(CtsAllocator* alloc, CtsRbTree* tree) {
    CtsRbTreeIterator* self = cts_rb_tree_iterator_new(alloc);
    if (self == NULL)
    {
        return NULL;
    }
    cts_rb_tree_ref(tree);
    self->tree = tree;
    self->current = cts_rb_node_minimum(tree->priv->root);  // We start from the minimum
    return self;
}

cts_pointer cts_rb_tree_iterator_next(CtsRbTreeIterator* self) {
    if (self->current == NULL)
    {
        return NULL;
    }
    cts_pointer key = self->current->key;
    self->current = cts_rb_node_successor(self->current);
    return key;
}

bool cts_rb_tree_iterator_has_next(CtsRbTreeIterator* self) {
    return self->current != NULL;
}

CtsRbTreeIterator* cts_rb_tree_reverse_iterator_new_from_tree(CtsAllocator* alloc, CtsRbTree* tree) {
    CtsRbTreeIterator* self = cts_rb_tree_iterator_new(alloc);
    if (self == NULL)
    {
        return NULL;
    }
    cts_rb_tree_ref(tree);
    self->tree = tree;
    self->current = cts_rb_node_maximum(tree->priv->root);  // We start from the maximum
    return self;
}

cts_pointer cts_rb_tree_reverse_iterator_next(CtsRbTreeIterator* self) {
    if (self->current == NULL)
    {
        return NULL;
    }
    cts_pointer key = self->current->key;
    self->current = cts_rb_node_predecessor(self->current);
    return key;
}

bool cts_rb_tree_reverse_iterator_has_next(CtsRbTreeIterator* self) {
    return self->current != NULL;
}

