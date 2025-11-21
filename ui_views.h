#ifndef UI_VIEWS_H
#define UI_VIEWS_H

#include <gtk/gtk.h>
#include "structure.h"

GtkWidget* create_books_view(void);
GtkWidget* create_users_view(void);
GtkWidget* create_emprunts_view(void);
GtkWidget* create_search_results_view(const char *titre, const char *auteur, const char *categorie);
GtkWidget* create_user_history_view(int id_utilisateur);
GtkWidget* create_current_loans_view(int id_utilisateur);
void refresh_books_container(GtkWidget *container);
void refresh_users_container(GtkWidget *container);
void refresh_emprunts_container(GtkWidget *container);
void refresh_current_container(GtkWidget *container, int id_utilisateur);
void build_stats_into_container(GtkWidget *container);

#endif /* UI_VIEWS_H */
