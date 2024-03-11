#ifndef POLYGON_H_INCLUDED
#define POLYGON_H_INCLUDED

#include <Cts/cts.h>

CTS_BEGIN_DECLARE_TYPE(CtsBase, Point, point)
double x;
double y;
CTS_END_DECLARE_TYPE(Point, point)

Point* point_new_with_coords(CtsAllocator* alloc, double x, double y);

CTS_BEGIN_DECLARE_TYPE(CtsBase, Polygon, polygon) 
CtsArray* points;
CTS_END_DECLARE_TYPE(Polygon, polygon)

bool polygon_add_point(Polygon* polygon, double x, double y);
size_t polygon_size(Polygon* polygon);
Point* polygon_get_point(Polygon* polygon, int index);
void polygon_giftwrap(Polygon* polygon);

#endif 

