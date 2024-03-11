#ifndef CTS_PIPELINE_H
#define CTS_PIPELINE_H

#include "object.h"

typedef union {
    void* ptr;
    double d;
    float f;
    int i;
} SourceValue;

CTS_BEGIN_DECLARE_TYPE(CtsBase, SourceElement, source_element)
SourceValue (*vfunc_push)(struct SourceElement *self);
CTS_END_DECLARE_TYPE(SourceElement, source_element)

SourceValue source_element_push(SourceElement* self);


CTS_BEGIN_DECLARE_TYPE(CtsBase, SinkElement, sink_element)
SourceElement* source;
void (*vfunc_on_pull)(struct SinkElement *self, SourceValue data);
CTS_END_DECLARE_TYPE(SinkElement, sink_element)

void sink_element_connect(SinkElement* self, SourceElement* source);
void sink_element_pull(SinkElement *self);


CTS_BEGIN_DECLARE_TYPE(SourceElement, TestSource, test_source)
double value;
CTS_END_DECLARE_TYPE(TestSource, test_source)

void test_source_set_value(TestSource* self, double v);

CTS_BEGIN_DECLARE_TYPE(SinkElement, TestSink, test_sink)
CTS_END_DECLARE_TYPE(TestSink, test_sink)



#endif
