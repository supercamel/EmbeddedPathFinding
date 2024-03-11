#include "pipeline.h"

CTS_DEFINE_TYPE(CtsBase, cts_base, SourceElement , source_element)

bool source_element_construct(SourceElement *self) {
    self->vfunc_push = NULL;
    return true;
}

void source_element_destruct(SourceElement *self) {
    (void)(self);
}

SourceValue source_element_push(SourceElement* self) {
    if(self->vfunc_push != NULL) {
        return self->vfunc_push(self);
    }
    SourceValue sv;
    sv.ptr = NULL;
    return sv;
}

CTS_DEFINE_TYPE(CtsBase, cts_base, SinkElement, sink_element)

bool sink_element_construct(SinkElement *self) {
    self->source = NULL;
    self->vfunc_on_pull = NULL;
    return true;
}

void sink_element_destruct(SinkElement *self) {
    if(self->source != NULL) {
        source_element_unref(self->source);
    }
}

void sink_element_connect(SinkElement* self, SourceElement* source) {
    self->source = source;
}

void sink_element_pull(SinkElement *self) {
    if(self->source != NULL) {
        SourceValue d = source_element_push(self->source);
        if(self->vfunc_on_pull != NULL) {
            self->vfunc_on_pull(self, d);
        }
    }
}

CTS_DEFINE_TYPE(SourceElement, source_element, TestSource, test_source)

SourceValue test_source_push(TestSource* self) {
    SourceValue sv;
    sv.d = self->value;
    return sv;
}

bool test_source_construct(TestSource *self) {
    SourceElement* source = (SourceElement*)(self);
    source->vfunc_push = (SourceValue(*)(SourceElement*))test_source_push; 
    return true;
}

void test_source_destruct(TestSource *self) {
    (void)(self);
}

void test_source_set_value(TestSource* self, double d) {
    self->value = d; 
}

CTS_DEFINE_TYPE(SinkElement, sink_element, TestSink, test_sink)

void test_sink_on_pull(TestSink* self, SourceValue data) {
    (void)(self);
    (void)(data);
    //printf("%f\n", data.f);
}

bool test_sink_construct(TestSink *self) {
    SinkElement* se = (SinkElement*)(self);
    se->vfunc_on_pull = (void(*)(SinkElement*, SourceValue))test_sink_on_pull;
    return true;
}

void test_sink_destruct(TestSink *self) {
    (void)(self);
}

