#include "ui_helpers.h"
#include <gtk/gtk.h>

void clear_container_children(GtkWidget *container) {
    if (!container) return;
    GList *children = gtk_container_get_children(GTK_CONTAINER(container));
    for (GList *l = children; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(children);
}

void add_view_in_scrolled(GtkWidget *container, GtkWidget *view) {
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled), view);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_container_add(GTK_CONTAINER(container), scrolled);
    gtk_widget_show_all(container);
}

GtkWidget *tree_view_with_columns(GtkListStore *store, const char *titles[], int ncols) {
    GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    for (int i = 0; i < ncols; ++i) {
        GtkCellRenderer *r = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *c = gtk_tree_view_column_new_with_attributes(titles[i], r, "text", i, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    }
    return view;
}
