#include <gtk/gtk.h>
#include "polygon.h"
#include "visibility_graph.h"

#define POOL_SIZE 1024 * 24 

uint8_t blob[POOL_SIZE];
CtsAllocator *alloc;
Graph *graph = NULL;
GtkWidget *drawing_area;
extern size_t n_allocs;

static void create_graph()
{
    Polygon *p = polygon_new(alloc);

    polygon_add_point(p, 100.0, 100.0);
    polygon_add_point(p, 100.0, 200.0);
    polygon_add_point(p, 150, 150);
    polygon_add_point(p, 200, 200);
    polygon_add_point(p, 200, 100.0);

//    polygon_giftwrap(p);

    Polygon *p2 = polygon_new(alloc);
    polygon_add_point(p2, 250.0, 100);
    polygon_add_point(p2, 250.0, 200);
    polygon_add_point(p2, 280, 200);
    polygon_add_point(p2, 380, 150);
    polygon_add_point(p2, 280, 100.0);

    graph = graph_new(alloc);
    graph_set_start_point(graph, point_new_with_coords(alloc, 80.0, 80.0));
    graph_set_end_point(graph, point_new_with_coords(alloc, 350, 300));
    graph_add_polygon(graph, p);
    graph_add_polygon(graph, p2);

    polygon_unref(p);
    polygon_unref(p2);
}

// Function to be called when the DrawingArea widget needs to be redrawn
static gboolean on_draw_event(GtkDrawingArea *area, cairo_t *cr, gpointer data)
{
    if (graph == NULL)
        create_graph();

    graph_calculate_visibility(graph);
    graph_print(graph);

    cairo_set_source_rgb(cr, 0, 0, 1); // Set color to blue
    size_t v_len = cts_array_get_length(graph->adjacency);
    for (size_t i = 0; i < v_len; i++)
    {
        AdjacencyNode *n = (AdjacencyNode *)cts_array_get(graph->adjacency, i);

        CtsSListIterator *iter = cts_slist_iterator_new_from_list(alloc, n->adjacent_points);
        while (cts_slist_iterator_has_next(iter))
        {
            Point *point = (Point *)cts_slist_iterator_next(iter);
            Point *root = n->root;
            cairo_move_to(cr, root->x, root->y);
            cairo_line_to(cr, point->x, point->y);
            cairo_stroke(cr);
        }
        cts_slist_iterator_unref(iter);
    }

    CtsArray *path = graph_get_path(graph);
    size_t path_len = cts_array_get_length(path);
    printf("Path len: %d\n", path_len);
    printf("Path:\n");
    cairo_set_source_rgb(cr, 1, 0, 0); // Set color to red
    cairo_move_to(cr, 80, 80);
    for (size_t i = 0; i < path_len; i++)
    {
        Point *p = (Point *)cts_array_get(path, i);
        printf("(%f, %f)\n", p->x, p->y);
        cairo_line_to(cr, p->x, p->y);
    }
    cairo_stroke(cr);

    cts_array_unref(path);

    printf("n_allocs: %d\n", n_allocs);
    return FALSE;
}

static void on_button_pressed(GtkGestureClick *gesture, int n_press, double x, double y, gpointer data)
{
    if (graph != NULL)
    {
        //alloc = cts_allocator_from_pool(blob, POOL_SIZE);
        //create_graph();
        graph_set_end_point(graph, point_new_with_coords(alloc, x, y));
        // graph_calculate_visibility(graph);
        gtk_widget_queue_draw(drawing_area);
    }
}

// Function to force a redraw of the DrawingArea
void force_redraw(GtkWidget *drawing_area)
{
    gtk_widget_queue_draw(drawing_area);
}

static void activate(GtkApplication *app)
{
    GtkWidget *window;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "My Gtk-4.0 App");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    drawing_area = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), on_draw_event, NULL, NULL);

    GtkGesture *gesture = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture), GDK_BUTTON_PRIMARY);
    g_signal_connect(gesture, "pressed", G_CALLBACK(on_button_pressed), NULL);
    gtk_widget_add_controller(drawing_area, GTK_EVENT_CONTROLLER(gesture));

    gtk_window_set_child(GTK_WINDOW(window), drawing_area);

    gtk_widget_show(window);
}

int main(int argc, char **argv)
{
    cts_allocator_init_default();

    alloc = cts_allocator_from_pool(blob, POOL_SIZE);
    //alloc = cts_allocator_get_default();

    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.GtkApplication", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

