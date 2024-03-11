#ifndef VISIBILITY_GRAPH_H
#define VISIBILITY_GRAPH_H

#include <Cts/cts.h>
#include "polygon.h"

CTS_BEGIN_DECLARE_TYPE(CtsBase, AdjacencyNode, adjacency_node)
Point* root;
CtsSList* adjacent_points;
Polygon* polygon;
CTS_END_DECLARE_TYPE(AdjacencyNode, adjacency_node)


CTS_BEGIN_DECLARE_TYPE(CtsBase, Graph, graph)
CtsArray* polygons; // array of Polygon*
Point* start_point;
Point* end_point;
CtsArray* adjacency; // adjacency list
CtsHashMap* point_to_adjacency_map;
CTS_END_DECLARE_TYPE(Graph, graph) 

void graph_add_polygon(Graph* graph, Polygon* polygon);
bool graph_calculate_visibility(Graph* graph);
void graph_print(Graph* graph);
void graph_set_start_point(Graph* graph, Point* point);
void graph_set_end_point(Graph* graph, Point* point);
CtsArray* graph_get_path(Graph* graph);


CTS_BEGIN_DECLARE_TYPE(CtsBase, GraphNode, graph_node) 
AdjacencyNode* point;
struct GraphNode* parent;
double g_cost;
double h_cost;
CTS_END_DECLARE_TYPE(GraphNode, graph_node)

typedef struct Edge {
    Point* from;
    Point* to;
} Edge;


CtsArray* find_path(CtsAllocator* alloc, Point* start, Point* end, Polygon** poly_list, size_t n_polygons);


#endif 
