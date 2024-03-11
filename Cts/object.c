#include "object.h"

CtsAllocator* cts_base_get_allocator(CtsBase *self) 
{
    return self->allocator;
}

void cts_base_ref(CtsBase *self)
{
    self->ref_count++;
}

void cts_base_unref(CtsBase *self)
{
    if(--(self->ref_count) == 0) {
        self->func_table.destroy(self);
        cts_allocator_free(self->allocator, self);
    }
}

void cts_base_construct(CtsBase *self) { 
    self->ref_count = 1;
}

CtsBase cts_base_class_init() 
{
    CtsBase self = {
        .ref_count = 1
    };
    return self;
}

CtsBase* cts_base_new(CtsAllocator* alloc) {
    CtsBase *self = cts_allocator_alloc(alloc, sizeof(CtsBase)); 
    if(self == NULL ) { return NULL; }
    self->allocator = alloc;
    *self = cts_base_class_init();
    cts_base_construct((CtsBase*)self); 
    return (CtsBase*)self; 
}
