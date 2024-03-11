#include <stdio.h>
#include "polygon.h"

CTS_DEFINE_TYPE(CtsBase, cts_base, Point, point)

bool point_construct(Point* point) { return true; }
void point_destruct(Point* point) { }

Point* point_new_with_coords(CtsAllocator* alloc, double x, double y) {
    Point* p = point_new(alloc);
    if(p == NULL) {
        return NULL;
    }
    p->x = x;
    p->y = y;
    return p;
}


CTS_DEFINE_TYPE(CtsBase, cts_base, Polygon, polygon)

bool polygon_construct(Polygon* polygon) {
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*) polygon);
    polygon->points = cts_array_new(alloc);
    if(polygon->points == NULL) {
        return false;
    }
    return true;
}

void polygon_destruct(Polygon* polygon) {
    cts_array_free_full(polygon->points, NULL, (ArrayFreeFunc)cts_object_free);
    cts_array_unref(polygon->points);
}

bool polygon_add_point(Polygon* polygon, double x, double y) {
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*) polygon);
    Point* p = point_new(alloc);
    p->x = x;
    p->y = y;
    cts_array_append(polygon->points, p);
    return true;
}

size_t polygon_size(Polygon* polygon) {
    return cts_array_get_length(polygon->points);
}

Point* polygon_get_point(Polygon* polygon, int index) {
    Point* p = (Point*)cts_array_get(polygon->points, index);
    return p;
}

static size_t left_most_point(Polygon* polygon) {
    double y;
    size_t n_points = cts_array_get_length(polygon->points);

    if(n_points <= 1) {
        return 0;
    }

    Point* p = (Point*)cts_array_get(polygon->points, 0);
    y = p->y;
    size_t r = 0;

    for(size_t i = 1; i < n_points; i++) {
        p = (Point*)cts_array_get(polygon->points, i);
        if(p->y <= y) {
            y = p->y;
            r = i;
        } 
    }
    return r;
}

static int orientation(Point* p, Point* q, Point* r) {
    double val = (q->y - p->y) * (r->x - q->x) - (q->x - p->x) * (r->y - q->y);

    if (val == 0) return 0;  // collinear
    return (val > 0) ? 1 : 2; // clock or counterclock wise
}

void polygon_giftwrap(Polygon* polygon) {
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*) polygon);
    size_t numPoints = cts_array_get_length(polygon->points);
    if(numPoints < 3) return; // Convex hull is not possible for less than 3 points

    // get the left most point
    size_t lmp = left_most_point(polygon);

    // gift wrapping algorithm
    // Create an array to store points of convex hull
    size_t* hull = (size_t*)cts_allocator_alloc(alloc, numPoints * sizeof(size_t));
    size_t m = 0;  // Initialize result (number of points in hull)

    size_t p = lmp, q;
    do {
        // add current point to result
        hull[m] = p;
        m++;

        // Search for a point 'q' such that orientation(p, x, q) is counterclockwise for all points 'x'
        q = (p + 1) % numPoints;
        for (size_t i = 0; i < numPoints; i++) {
            // If i is more counterclockwise than current q, then update q
            if (orientation((Point*)cts_array_get(polygon->points, p), (Point*)cts_array_get(polygon->points, i), (Point*)cts_array_get(polygon->points, q)) == 2)
                q = i;
        }

        // Now q is the most counterclockwise with respect to p
        // Set p as q for next iteration, so that q is added to result 'hull'
        p = q;

    } while (p != lmp);  // While we don't come to first point

    CtsArray* arr = cts_array_new(alloc);
    // Print Result
    for (size_t i = 0; i < m; i++) {
        Point* point = (Point*)cts_array_get(polygon->points, hull[i]);
        point_ref(point);
        cts_array_append(arr, point);
    }

    cts_array_free_full(polygon->points, NULL, (ArrayFreeFunc)cts_object_free);
    cts_array_unref(polygon->points);
    polygon->points = arr;

    // Free the allocated memory
    cts_allocator_free(alloc, hull);
}


