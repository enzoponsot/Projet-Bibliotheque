#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include <gtk/gtk.h>

void clear_container_children(GtkWidget *container);
void add_view_in_scrolled(GtkWidget *container, GtkWidget *view);
GtkWidget *tree_view_with_columns(GtkListStore *store, const char *titles[], int ncols);

#endif /* UI_HELPERS_H */
