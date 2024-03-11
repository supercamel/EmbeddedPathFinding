#include <stdio.h>
#include <math.h>
#include "visibility_graph.h"
#include "polygon.h"

// A small number for floating-point comparison
#define EPSILON 1e-6

bool points_equal(Point* p1, Point* p2);
static uint32_t point_hash_func(const cts_pointer key);


static uint32_t adjacency_node_hash_func(const cts_pointer key) {
    AdjacencyNode* adjacencyNode = (AdjacencyNode*) key;
    if (adjacencyNode->root == NULL) {
        // Handle the error
    }
    return point_hash_func(adjacencyNode->root);
}

static uint32_t point_hash_func(const cts_pointer key) {
    Point* point = (Point*) key;
    if (point == NULL) {
        // Handle the error
    }
    return (uint32_t)(point->x + point->y);
}


CTS_DEFINE_TYPE(CtsBase, cts_base, AdjacencyNode, adjacency_node)

bool adjacency_node_construct(AdjacencyNode* self) {
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*) self);
    self->root = NULL;
    self->polygon = NULL;
    self->adjacent_points = cts_slist_new(alloc);
    return true;
}

void adjacency_node_destruct(AdjacencyNode* self) {
    cts_slist_free(self->adjacent_points);
    cts_slist_unref(self->adjacent_points);
}

CTS_DEFINE_TYPE(CtsBase, cts_base, GraphNode, graph_node)

bool graph_node_construct(GraphNode* graph_node) {
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*) graph_node);
    graph_node->point = NULL;
    graph_node->parent = NULL;
    graph_node->g_cost = 0.0;
    graph_node->h_cost = 0.0;
    return true;
}

void graph_node_destruct(GraphNode* graph_node) {

}

int compare_graph_nodes(const GraphNode* a, const GraphNode* b) {
    double f_costA = a->g_cost + a->h_cost;
    double f_costB = b->g_cost + b->h_cost;
    if(f_costA < f_costB) return 1;
    if(f_costA > f_costB) return -1;
    return 0;
}

double heuristic(Point* point1, Point* point2) {
    double dx = point1->x - point2->x;
    double dy = point1->y - point2->y;
    return sqrt(dx*dx + dy*dy);
}

CTS_DEFINE_TYPE(CtsBase, cts_base, Graph, graph)

bool graph_construct(Graph* graph) {
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*) graph);
    graph->polygons = cts_array_new(alloc);
    graph->start_point = NULL;
    graph->end_point = NULL;
    if(graph->polygons == NULL) {
        return false;
    }

    graph->adjacency = cts_array_new(alloc);
    return true;
}

void graph_destruct(Graph* graph) {
    cts_array_free_full(graph->adjacency, NULL, (ArrayFreeFunc)cts_object_free);
    cts_array_unref(graph->adjacency);

    cts_array_free_full(graph->polygons, NULL, (ArrayFreeFunc)cts_object_free);
    cts_array_unref(graph->polygons);

    if(graph->start_point) {
        point_unref(graph->start_point);
    }
    if(graph->end_point) {
        point_unref(graph->end_point);
    }
}

static double max(double a, double b) {
    return (a > b)? a : b;
}

static double min(double a, double b) {
    return (a < b)? a : b;
}


void graph_add_polygon(Graph* graph, Polygon* polygon) {
    cts_array_append(graph->polygons, polygon);
    polygon_ref(polygon);
}

double orientation(Point* p, Point* q, Point* r) {
    double val = (q->y - p->y) * (r->x - q->x) - (q->x - p->x) * (r->y - q->y);
    if (val == 0) return 0;  // collinear
    return (val > 0) ? 1: 2; // clock or counterclock
}

bool onSegment(Point* p, Point* q, Point* r) {
    if (q->x <= max(p->x, r->x) && q->x >= min(p->x, r->x) &&
        q->y <= max(p->y, r->y) && q->y >= min(p->y, r->y))
       return true;
    return false;
}

bool intersects(Edge* e1, Edge* e2) {
    Point* p1 = e1->from;
    Point* p2 = e1->to;
    Point* p3 = e2->from;
    Point* p4 = e2->to;

    if(p1 == p3 && p2 == p4) return false;
    if(p1 == p4 && p2 == p3) return false;

    // Find the four orientations needed for general and special cases
    double o1 = orientation(p1, p2, p3);
    double o2 = orientation(p1, p2, p4);
    double o3 = orientation(p3, p4, p1);
    double o4 = orientation(p3, p4, p2);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    // Special Cases
    if (o1 == 0 && onSegment(p1, p3, p2)) return true;
    if (o2 == 0 && onSegment(p1, p4, p2)) return true;
    if (o3 == 0 && onSegment(p3, p1, p4)) return true;
    if (o4 == 0 && onSegment(p3, p2, p4)) return true;

    return false; // Doesn't fall in any of the above cases
}


static bool is_visible(Graph* graph, AdjacencyNode* n1, AdjacencyNode* n2) {
    Edge edge = { n1->root, n2->root }; 

    size_t n_polygons = cts_array_get_length(graph->polygons);
    for(size_t i = 0; i < n_polygons; i++) {
        Polygon* polygon = (Polygon*)cts_array_get(graph->polygons, i);

        // iterate over each edge in the polygon
        for(size_t j = 0; j < polygon_size(polygon); j++) {
            Point* edge_start = polygon_get_point(polygon, j);
            Point* edge_end = polygon_get_point(polygon, (j + 1) % polygon_size(polygon));

            if((edge.from == edge_start) || (edge.from == edge_end)) {
                continue;
            }
            if((edge.to == edge_start) || (edge.to == edge_end)) {
                continue;
            }

            Edge poly_edge = { edge_start, edge_end };

            if(intersects(&edge, &poly_edge)) {
                return false;
            }
        }
    }

    return true;
}

void free_adjacency_node(CtsAllocator* alloc, AdjacencyNode* n) {
    (void)alloc;
    adjacency_node_unref(n);
}

bool graph_calculate_visibility(Graph* graph) {
    // Clear existing vertices and edges
    cts_array_free_full(graph->adjacency, NULL, (ArrayFreeFunc)free_adjacency_node);

    // create adjacency list stubs from polygons
    for(size_t i = 0; i < cts_array_get_length(graph->polygons); i++) {
        Polygon* polygon = (Polygon*)cts_array_get(graph->polygons, i);
        for(size_t j = 0; j < cts_array_get_length(polygon->points); j++) {
            Point* point = (Point*)cts_array_get(polygon->points, j);
            //point_ref(point);

            extern size_t n_allocs;
            AdjacencyNode* an = adjacency_node_new(cts_base_get_allocator((CtsBase*)graph));
            if(an == NULL) {
                cts_array_free_full(graph->adjacency, NULL, (ArrayFreeFunc)free_adjacency_node);
                return false;
            }

            an->root = point;
            an->polygon = polygon;

            Point* next_point = cts_array_get(polygon->points, (j+1) % cts_array_get_length(polygon->points));
            //point_ref(next_point);
            cts_slist_append(an->adjacent_points, next_point);

            Point* prev_point = cts_array_get(polygon->points, (j - 1 + cts_array_get_length(polygon->points)) % cts_array_get_length(polygon->points));
            //point_ref(prev_point);
            cts_slist_append(an->adjacent_points, prev_point);

            cts_array_append(graph->adjacency, an);
        }
    }

    // Add start and end points to adjacency list
    //point_ref(graph->start_point);
    //point_ref(graph->end_point);
    AdjacencyNode* an = adjacency_node_new(cts_base_get_allocator((CtsBase*)graph));
    if(an == NULL) {
        cts_array_free_full(graph->adjacency, NULL, (ArrayFreeFunc)free_adjacency_node);
        return false;
    }
    an->root = graph->start_point;
    cts_array_append(graph->adjacency, an);

    an = adjacency_node_new(cts_base_get_allocator((CtsBase*)graph));
    if(an == NULL) {
        cts_array_free_full(graph->adjacency, NULL, (ArrayFreeFunc)free_adjacency_node);
        return false;
    }
    an->root = graph->end_point;
    cts_array_append(graph->adjacency, an);

    // Calculate edges (visibility) between vertices
    size_t numVertices = cts_array_get_length(graph->adjacency);
    for(size_t i = 0; i < numVertices; i++) {
        AdjacencyNode* n = (AdjacencyNode*)cts_array_get(graph->adjacency, i);
        for(size_t j = 0; j < numVertices; j++) {
            AdjacencyNode* n2 = (AdjacencyNode*)cts_array_get(graph->adjacency, j); 
            if(n == n2) {
                continue;
            }

            if((n->polygon != NULL) && (n->polygon == n2->polygon)) {
                continue;
            }

            if(is_visible(graph, n, n2)) {
                //adjacency_node_ref(n2);
                cts_slist_append(n->adjacent_points, n2->root);
            }
        }
    }
}

void graph_print(Graph* graph) {
    size_t n_nodes = cts_array_get_length(graph->adjacency);
    for(size_t i = 0; i < n_nodes; i++) {
        AdjacencyNode* n = (AdjacencyNode*)cts_array_get(graph->adjacency, i);
        printf("(%f, %f): ", n->root->x, n->root->y);
        CtsSListIterator* iter = cts_slist_iterator_new_from_list(cts_base_get_allocator((CtsBase*)(graph)), n->adjacent_points);
        while(cts_slist_iterator_has_next(iter)) {
            Point* point = (Point*)cts_slist_iterator_next(iter);

            printf("(%f, %f)", point->x, point->y);
        }
        printf("\n");

        cts_slist_iterator_unref(iter);
    }
}

void graph_set_start_point(Graph* graph, Point* point) 
{
    if(graph->start_point) {
        point_unref(graph->start_point);
    }
    graph->start_point = point;
}

void graph_set_end_point(Graph* graph, Point* point)
{
    if(graph->end_point) {
        point_unref(graph->end_point);
    }
    graph->end_point = point;
}


bool points_equal(Point* p1, Point* p2) {
    // compare floats to episilon
    return fabs(p1->x - p2->x) < EPSILON && fabs(p1->y - p2->y) < EPSILON;
}

bool adjacency_nodes_equal(cts_pointer pn1, cts_pointer pn2) {
    AdjacencyNode* n1 = (AdjacencyNode*)pn1;
    AdjacencyNode* n2 = (AdjacencyNode*)pn2;
    return points_equal(n1->root, n2->root);
}

GraphNode* find_in_map(CtsHashMap* map, Point* node) {
    // Use the hashmap to find the node
    return (GraphNode*) cts_hash_map_get(map, node);
}

AdjacencyNode* get_adjacency_node(Graph* graph, Point* point) {
    // Assuming that there is a hash map in the Graph structure that 
    // maps each point to the corresponding AdjacencyNode
    return (AdjacencyNode*) cts_hash_map_get(graph->point_to_adjacency_map, point);
}

void release_graph_nodes(CtsArray* nodes) {
    for(size_t i = 0; i < cts_array_get_length(nodes); i++) {
        GraphNode* node = (GraphNode*)cts_array_get(nodes, i);
        graph_node_unref(node);
    }
}

CtsArray* graph_get_path(Graph* graph) {
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase*) graph);
    graph->point_to_adjacency_map = cts_hash_map_new_full(alloc, point_hash_func, points_equal, NULL, NULL, NULL, NULL);
    
    for (size_t i = 0; i < cts_array_get_length(graph->adjacency); i++) {
        AdjacencyNode* adj_node = (AdjacencyNode*) cts_array_get(graph->adjacency, i);
        cts_hash_map_set(graph->point_to_adjacency_map, adj_node->root, adj_node);
    }

    CtsPriorityQueue* openSet = cts_priority_queue_new_full(alloc, (HeapCompareFunc) compare_graph_nodes, NULL, NULL);
    CtsHashMap* openSetMap = cts_hash_map_new_full(alloc, point_hash_func, points_equal, NULL, NULL, NULL, NULL);
    CtsHashMap* closedSet = cts_hash_map_new_full(alloc, point_hash_func, points_equal, NULL, NULL, NULL, NULL);
    CtsArray* graph_nodes = cts_array_new(alloc);

    CtsArray* path = cts_array_new(alloc);

    // Create a GraphNode for the start point and add it to the open set
    GraphNode* start_node = graph_node_new(alloc);
    if(start_node == NULL) {
        goto cleanup;
    }

    if(!cts_array_append(graph_nodes, start_node)) {
        goto cleanup;
    }

    size_t n_points = cts_array_get_length(graph->adjacency);
    start_node->point = cts_array_get(graph->adjacency, n_points-2);
    start_node->g_cost = 0.0;
    start_node->h_cost = heuristic(start_node->point->root, graph->end_point);
    if((cts_priority_queue_push(openSet, start_node) == false) ||
        (cts_hash_map_set(openSetMap, start_node->point->root, start_node) == false)) {
        goto cleanup;
    }

    while (!cts_priority_queue_is_empty(openSet)) {
        GraphNode* current_node = (GraphNode*) cts_priority_queue_pop(openSet);
        cts_hash_map_remove(openSetMap, current_node->point->root);

        // If the current node is the end point, construct the path and return
        if (points_equal(current_node->point->root, graph->end_point)) {
            while (current_node != NULL) {
                if(cts_array_append(path, current_node->point->root) == false) {
                    goto cleanup;
                }
                //point_ref(current_node->point->root);
                GraphNode* tmp = current_node;
                current_node = current_node->parent;
                //graph_node_unref(tmp);
            }
            cts_array_reverse(path);
            goto cleanup;
        }

        CtsSListIterator* iter = cts_slist_iterator_new_from_list(alloc, current_node->point->adjacent_points);
        if(iter == NULL) {
            goto cleanup;
        }
        while(cts_slist_iterator_has_next(iter)) {
            Point* neighbor_point = (Point*)cts_slist_iterator_next(iter);
            if (cts_hash_map_contains(closedSet, neighbor_point)) continue;  // Ignore neighbors in the closed set

            double tentative_g_cost = current_node->g_cost + heuristic(current_node->point->root, neighbor_point);
            GraphNode* graph_node_neighbor;

            // If the neighbor is in the open set and the tentative g cost is less than the neighbor's current g cost, update the neighbor's cost and parent
            if (cts_hash_map_contains(openSetMap, neighbor_point)) {
                graph_node_neighbor = (GraphNode*) cts_hash_map_get(openSetMap, neighbor_point);
                if (tentative_g_cost < graph_node_neighbor->g_cost) {
                    graph_node_neighbor->g_cost = tentative_g_cost;
                    graph_node_neighbor->parent = current_node;
                }
            } else {
                // If the neighbor is not in the open set, create a new graph node for the neighbor and add it to the open set
                graph_node_neighbor = graph_node_new(alloc);
                if(graph_node_neighbor == NULL) {
                    goto cleanup;
                }
                cts_array_append(graph_nodes, graph_node_neighbor);
                graph_node_neighbor->point = (AdjacencyNode*) cts_hash_map_get(graph->point_to_adjacency_map, neighbor_point);
                graph_node_neighbor->g_cost = tentative_g_cost;
                graph_node_neighbor->h_cost = heuristic(neighbor_point, graph->end_point);
                graph_node_neighbor->parent = current_node;
                if(cts_hash_map_set(openSetMap, neighbor_point, graph_node_neighbor) == false) {
                    goto cleanup;
                }
                if(cts_priority_queue_push(openSet, graph_node_neighbor) == false) {
                    goto cleanup;
                }
            }
        }

        // Only after all neighbors have been considered, add the current node to the closed set
        if(cts_hash_map_set(closedSet, current_node->point->root, current_node) == false) {
            goto cleanup;
        }
        cts_slist_iterator_unref(iter);
    }

    cleanup:

    cts_priority_queue_clear(openSet);
    cts_priority_queue_unref(openSet);
    cts_hash_map_clear(openSetMap);
    cts_hash_map_unref(openSetMap);
    cts_hash_map_clear(closedSet);
    cts_hash_map_unref(closedSet);
    cts_hash_map_clear(graph->point_to_adjacency_map);
    cts_hash_map_unref(graph->point_to_adjacency_map);
    release_graph_nodes(graph_nodes);
    cts_array_unref(graph_nodes);
            
    return path;
}


CtsArray* find_path(CtsAllocator* alloc, Point* start, Point* end, Polygon** poly_list, size_t n_polygons)
{
    Graph* graph = graph_new(alloc);

    for(size_t i = 0; i < n_polygons; i++) {
        graph_add_polygon(graph, poly_list[i]);
    }

    graph_set_start_point(graph, start);
    graph_set_end_point(graph, end);
    graph_calculate_visibility(graph);
    CtsArray* path = graph_get_path(graph);
    graph_unref(graph);
}
