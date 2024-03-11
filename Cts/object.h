// TODO actually read and check this gpt generated description

/**
 * CTS (Camel's Type System) - A simple type system for C programming language.
 *
 * CTS provides a basic object-oriented programming model for C with features such as inheritance 
 * and virtual functions. It doesn't directly support inheritance as in languages like C++ or Java,
 * but you can achieve something similar by including an instance of the base type as the first 
 * member of the derived type. This is essentially what CTS is doing.
 *
 * # Inheritance:
 * To define a new object that inherits from a base object:
 * 
 * ```c
 * CTS_BEGIN_DECLARE_TYPE(BaseType, DerivedType, derived_type)
 * // Additional members for DerivedType go here...
 * CTS_END_DECLARE_TYPE(DerivedType, derived_type)
 * ```
 *
 * In this case, `DerivedType` includes an instance of `BaseType` as its first member. This allows 
 * a `DerivedType` pointer to be used wherever a `BaseType` pointer is expected, enabling polymorphism.
 * 
 * # Overriding Functions:
 * In CTS, you can override a function by providing a new definition in the derived type's function table.
 *
 * ```c
 * CTS_DEFINE_TYPE(BaseType, base_type, DerivedType, derived_type)
 * void derived_type_destroy(DerivedType* self) {
 *     // New implementation goes here...
 *     // Then call parent's destroy function
 *     base_type_destroy((BaseType*)self);
 * }
 * void derived_type_class_init(DerivedType* self) {
 *     base_type_class_init((BaseType*)self);
 *     // Override the destroy function
 *     ((BaseType*)self)->func_table.destroy = (void (*)(void*))derived_type_destroy;
 * }
 * ```
 *
 * # Calling Parent Class Functions:
 * To call a function from the base type, you can access the function through the base type's function table.
 * If you have a `DerivedType` instance, you can cast it to `BaseType` with `(BaseType*)self`.
 * 
 * # Virtual Functions:
 * In CTS, functions are "virtual" in the sense that they can be overridden by derived types. When a 
 * function is called, the implementation that gets used is the one associated with the type of the object 
 * that the function is being called on. This is determined at runtime by looking up the function in the object's 
 * function table. If the function has been overridden by a derived type, the overriding implementation will be used.
 *
 * ```c
 * DerivedType* obj = derived_type_new();
 * // This will call derived_type_destroy, if it has been overridden
 * cts_base_unref((CtsBase*)obj);
 * ```
 */


#ifndef CTS_OBJECT_H
#define CTS_OBJECT_H

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "allocator.h"



typedef struct CtsBaseFunctionTable {
    void (*destroy)(void* self);
} CtsBaseFunctionTable;

typedef struct CtsBase {
    size_t ref_count;
    CtsAllocator* allocator;
    CtsBaseFunctionTable func_table;
} CtsBase;


CtsBase cts_base_class_init();
CtsBase* cts_base_new(CtsAllocator* alloc);
CtsAllocator* cts_base_get_allocator(CtsBase* self);
void cts_base_construct(CtsBase *self);
void cts_base_destroy(CtsBase *self);
void cts_base_ref(CtsBase *self);
void cts_base_unref(CtsBase *self);

typedef void* cts_pointer;

typedef void (*CtsFreeFunc)(cts_pointer alloc, cts_pointer value);

static inline void cts_object_free(cts_pointer alloc, CtsBase* object) {
    (void)(alloc);
    cts_base_unref((CtsBase*)object);
}

#define CTS_BEGIN_DECLARE_TYPE(BaseType, Type, type) \
    typedef struct Type { \
        BaseType __parent; 

#define CTS_END_DECLARE_TYPE(Type, type) \
        CtsBaseFunctionTable func_table; \
    } Type; \
    bool type##_construct(Type *self); \
    void type##_destroy(Type *self); \
    void type##_destruct(Type *self); \
    void type##_ref(Type* self); \
    void type##_unref(Type *self); \
    Type* type##_new(CtsAllocator* alloc); \
    void type##_class_init(Type* self); 

#define CTS_DEFINE_TYPE(BaseType, base_type, Type, type) \
    void type##_class_init(Type* self) { \
        base_type##_class_init(&self->__parent); \
        self->__parent.func_table.destroy = (void (*)(void*))type##_destroy; \
        base_type##_construct((BaseType*)&self->__parent); \
        self->func_table.destroy = NULL; \
    } \
    void type##_ref(Type* self) { \
        CtsBase* obj = (CtsBase*)self; \
        cts_base_ref(obj); \
    } \
    \
    void type##_unref(Type* self) { \
        CtsBase* obj = (CtsBase*)self; \
        cts_base_unref(obj); \
    }\
    void type##_destroy(Type* self) {\
        type##_destruct((Type*)self); \
        if(self->func_table.destroy != NULL) { \
            self->func_table.destroy(self); \
        } \
    }\
    \
    Type* type##_new(CtsAllocator* alloc) { \
        Type *self = cts_allocator_alloc(alloc, sizeof(Type)); \
        if(self == NULL) { return NULL; } \
        CtsBase* base = (CtsBase*)self; \
        base->allocator = alloc; \
        type##_class_init(self); \
        if(type##_construct(self) == false) {  \
            cts_allocator_free(alloc, self); \
            return NULL; \
        }; \
        return (Type*)self; \
    }

#endif
