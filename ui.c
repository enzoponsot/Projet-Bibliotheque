// ui.c - GTK3 UI implementation (single UI file)
#include "ui.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "authentification.h"
#include "livres.h"
#include "fichiers.h"
#include "utilisateurs.h"
#include "emprunts.h"
#include "statistiques.h"

// Keep a pointer to the main data
static Bibliotheque *g_bib = NULL;
static Utilisateur *g_user = NULL;
/* keep last logged user in RAM for auto-login during this process lifetime */
static Utilisateur *g_last_user = NULL;

// Persist last logged user id across runs (simple file in repo root)
static void save_last_user_id(int id) {
    FILE *f = fopen(".last_user", "w");
    if (!f) return;
    fprintf(f, "%d\n", id);
    fclose(f);
}
static int read_last_user_id(void) {
    FILE *f = fopen(".last_user", "r");
    int id = -1;
    if (!f) return -1;
    if (fscanf(f, "%d", &id) != 1) id = -1;
    fclose(f);
    return id;
}
static void clear_last_user_id(void) {
    remove(".last_user");
}

// Helper struct and callback for switching notebook pages
typedef struct { GtkNotebook *nb; gint idx; } PageSwitch;
static void switch_page_cb(GtkButton *b, gpointer user_data) {
    PageSwitch *p = user_data;
    gtk_notebook_set_current_page(p->nb, p->idx);
}

// Notebook switch handler: update a header label with the active page name
static void on_notebook_switch_page(GtkNotebook *notebook, GtkWidget *page, gint page_num, gpointer user_data) {
    (void)notebook; (void)page;
    GtkLabel *lbl = GTK_LABEL(user_data);
    const char *titles[] = { "Catalogue", "Recherche", "Historique", "Mes emprunts", "Statistiques" };
    if (page_num >= 0 && page_num < (gint)(sizeof(titles)/sizeof(titles[0]))) {
        gtk_label_set_text(lbl, titles[page_num]);
    }
}

// Convenience: show message dialog
static void show_info(GtkWindow *parent, const char *msg) {
    GtkWidget *dlg = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}

// Books list model: create and fill a GtkListStore
static GtkWidget* create_books_view() {
    GtkListStore *store = gtk_list_store_new(5, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
    for (int i = 0; i < g_bib->nombre_livres; ++i) {
        Livre *L = &g_bib->livres[i];
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, L->id,
                           1, L->titre,
                           2, L->auteur,
                           3, L->categorie,
                           4, L->annee,
                           -1);
    }

    GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    GtkCellRenderer *r;
    GtkTreeViewColumn *c;

    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("ID", r, "text", 0, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Titre", r, "text", 1, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Auteur", r, "text", 2, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Catégorie", r, "text", 3, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Année", r, "text", 4, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);

    return view;
}

// Users list model
static GtkWidget* create_users_view() {
    GtkListStore *store = gtk_list_store_new(7, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_DOUBLE);
    for (int i = 0; i < g_bib->nombre_utilisateurs; ++i) {
        Utilisateur *U = &g_bib->utilisateurs[i];
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, U->id,
                           1, U->nom,
                           2, U->prenom,
                           3, U->id_etudiant,
                           4, U->email,
                           5, U->nombre_emprunts_actifs,
                           6, U->penalites,
                           -1);
    }

    GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    GtkCellRenderer *r; GtkTreeViewColumn *c;
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("ID", r, "text", 0, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Nom", r, "text", 1, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Prénom", r, "text", 2, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("ID étudiant", r, "text", 3, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Email", r, "text", 4, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Emprunts actifs", r, "text", 5, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Pénalités (€)", r, "text", 6, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);

    return view;
}

static void refresh_users_container(GtkWidget *container) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(container));
    for (GList *l = children; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(children);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *view = create_users_view();
    gtk_container_add(GTK_CONTAINER(scrolled), view);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_container_add(GTK_CONTAINER(container), scrolled);
    gtk_widget_show_all(container);
}

// Emprunts view (active emprunts)
static GtkWidget* create_emprunts_view() {
    GtkListStore *store = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE);
    for (int i = 0; i < g_bib->nombre_emprunts; ++i) {
        Emprunt *E = &g_bib->emprunts[i];
        if (E->date_retour_effectif != 0) continue; // skip returned
        Livre *L = rechercher_livre_par_id(g_bib, E->id_livre);
        Utilisateur *U = rechercher_utilisateur_par_id(g_bib, E->id_utilisateur);
        char buf_date[64];
        struct tm tmv;
        localtime_r(&E->date_emprunt, &tmv);
        strftime(buf_date, sizeof(buf_date), "%Y-%m-%d", &tmv);
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, E->id,
                           1, L ? L->titre : "(livre supprimé)",
                           2, U ? U->prenom : "(utilisateur)",
                           3, U ? U->nom : "",
                           4, buf_date,
                           5, E->penalite,
                           -1);
    }

    GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    GtkCellRenderer *r; GtkTreeViewColumn *c;
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("ID", r, "text", 0, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Livre", r, "text", 1, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Prénom", r, "text", 2, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Nom", r, "text", 3, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Date emprunt", r, "text", 4, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Pénalité (€)", r, "text", 5, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);

    return view;
}

static void refresh_emprunts_container(GtkWidget *container) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(container));
    for (GList *l = children; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(children);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *view = create_emprunts_view();
    gtk_container_add(GTK_CONTAINER(scrolled), view);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_container_add(GTK_CONTAINER(container), scrolled);
    gtk_widget_show_all(container);
}
static GtkWidget* create_current_loans_view(int id_utilisateur); /* forward */

// Refresh current user's loans container
static void refresh_current_container(GtkWidget *container, int id_utilisateur) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(container));
    for (GList *l = children; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(children);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *view = create_current_loans_view(id_utilisateur);
    gtk_container_add(GTK_CONTAINER(scrolled), view);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_container_add(GTK_CONTAINER(container), scrolled);
    gtk_widget_show_all(container);
}

// Statistics simple summary (uses data in Bibliotheque)
static void build_stats_into_container(GtkWidget *container) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(container));
    for (GList *l = children; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(children);

    char buf[256];
    GtkWidget *v = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    snprintf(buf, sizeof(buf), "Livres: %d", g_bib->nombre_livres);
    gtk_box_pack_start(GTK_BOX(v), gtk_label_new(buf), FALSE, FALSE, 4);
    snprintf(buf, sizeof(buf), "Utilisateurs: %d", g_bib->nombre_utilisateurs);
    gtk_box_pack_start(GTK_BOX(v), gtk_label_new(buf), FALSE, FALSE, 4);
    snprintf(buf, sizeof(buf), "Emprunts (total): %d", g_bib->nombre_emprunts);
    gtk_box_pack_start(GTK_BOX(v), gtk_label_new(buf), FALSE, FALSE, 4);

    // Count active emprunts and en retard
    int active = 0, en_retard = 0;
    time_t now = time(NULL);
    for (int i = 0; i < g_bib->nombre_emprunts; ++i) {
        Emprunt *E = &g_bib->emprunts[i];
        if (E->date_retour_effectif == 0) {
            active++;
            if (difftime(now, E->date_retour_prevue) > 0) en_retard++;
        }
    }
    snprintf(buf, sizeof(buf), "Emprunts actifs: %d", active);
    gtk_box_pack_start(GTK_BOX(v), gtk_label_new(buf), FALSE, FALSE, 4);
    snprintf(buf, sizeof(buf), "Emprunts en retard: %d", en_retard);
    gtk_box_pack_start(GTK_BOX(v), gtk_label_new(buf), FALSE, FALSE, 4);

    gtk_container_add(GTK_CONTAINER(container), v);
    gtk_widget_show_all(container);
}

// Search results view
static GtkWidget* create_search_results_view(const char *titre, const char *auteur, const char *categorie) {
    Livre *resultats[MAX_LIVRES];
    int n = rechercher_livres_multi_criteres(g_bib, titre && *titre ? titre : NULL, auteur && *auteur ? auteur : NULL, categorie && *categorie ? categorie : NULL, resultats, MAX_LIVRES);
    GtkListStore *store = gtk_list_store_new(5, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
    for (int i = 0; i < n; ++i) {
        Livre *L = resultats[i];
        GtkTreeIter iter; gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, L->id, 1, L->titre, 2, L->auteur, 3, L->categorie, 4, L->annee, -1);
    }
    GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    GtkCellRenderer *r; GtkTreeViewColumn *c;
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("ID", r, "text", 0, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Titre", r, "text", 1, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Auteur", r, "text", 2, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Catégorie", r, "text", 3, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Année", r, "text", 4, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    return view;
}

// User history view (all emprunts for a user)
static GtkWidget* create_user_history_view(int id_utilisateur) {
    GtkListStore *store = gtk_list_store_new(7, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE);
    for (int i = 0; i < g_bib->nombre_emprunts; ++i) {
        Emprunt *E = &g_bib->emprunts[i];
        if (E->id_utilisateur != id_utilisateur) continue;
        Livre *L = rechercher_livre_par_id(g_bib, E->id_livre);
        char date_emp[32] = ""; char date_ret[32] = "(non rendu)";
        struct tm tmv;
        localtime_r(&E->date_emprunt, &tmv); strftime(date_emp, sizeof(date_emp), "%Y-%m-%d", &tmv);
        if (E->date_retour_effectif != 0) { localtime_r(&E->date_retour_effectif, &tmv); strftime(date_ret, sizeof(date_ret), "%Y-%m-%d", &tmv); }
        GtkTreeIter iter; gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, E->id,
                           1, L ? L->titre : "(supprimé)",
                           2, date_emp,
                           3, date_ret,
                           4, E->est_en_retard ? "Oui" : "Non",
                           5, "",
                           6, E->penalite,
                           -1);
    }
    GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    GtkCellRenderer *r; GtkTreeViewColumn *c;
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("ID", r, "text", 0, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Livre", r, "text", 1, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Date emprunt", r, "text", 2, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Date retour", r, "text", 3, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("En retard", r, "text", 4, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("", r, "text", 5, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Pénalité (€)", r, "text", 6, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    return view;
}

// Current loans for a user
static GtkWidget* create_current_loans_view(int id_utilisateur) {
    GtkListStore *store = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE);
    for (int i = 0; i < g_bib->nombre_emprunts; ++i) {
        Emprunt *E = &g_bib->emprunts[i];
        if (E->id_utilisateur != id_utilisateur) continue;
        if (E->date_retour_effectif != 0) continue;
        Livre *L = rechercher_livre_par_id(g_bib, E->id_livre);
        char date_emp[32] = ""; struct tm tmv; localtime_r(&E->date_emprunt, &tmv); strftime(date_emp, sizeof(date_emp), "%Y-%m-%d", &tmv);
        GtkTreeIter iter; gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, E->id, 1, L ? L->titre : "(supprimé)", 2, date_emp, 3, "", 4, "", 5, E->penalite, -1);
    }
    GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    GtkCellRenderer *r; GtkTreeViewColumn *c;
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("ID", r, "text", 0, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Livre", r, "text", 1, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Date emprunt", r, "text", 2, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("", r, "text", 3, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("", r, "text", 4, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    r = gtk_cell_renderer_text_new(); c = gtk_tree_view_column_new_with_attributes("Pénalité (€)", r, "text", 5, NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(view), c);
    return view;
}

// Change password dialog
static void on_change_password(GtkButton *btn, gpointer data) {
    (void)btn;
    GtkWindow *parent = GTK_WINDOW(data);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Modifier mot de passe", parent, GTK_DIALOG_MODAL, "Enregistrer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog)); GtkWidget *grid = gtk_grid_new(); gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *e_old = gtk_entry_new(); gtk_entry_set_visibility(GTK_ENTRY(e_old), FALSE);
    GtkWidget *e_new = gtk_entry_new(); gtk_entry_set_visibility(GTK_ENTRY(e_new), FALSE);
    GtkWidget *e_new2 = gtk_entry_new(); gtk_entry_set_visibility(GTK_ENTRY(e_new2), FALSE);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Ancien mot de passe:"), 0,0,1,1); gtk_grid_attach(GTK_GRID(grid), e_old, 1,0,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Nouveau mot de passe:"), 0,1,1,1); gtk_grid_attach(GTK_GRID(grid), e_new, 1,1,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Confirmer:"), 0,2,1,1); gtk_grid_attach(GTK_GRID(grid), e_new2, 1,2,1,1);
    gtk_widget_show_all(dialog);
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        const char *old = gtk_entry_get_text(GTK_ENTRY(e_old));
        const char *n1 = gtk_entry_get_text(GTK_ENTRY(e_new));
        const char *n2 = gtk_entry_get_text(GTK_ENTRY(e_new2));
        if (strcmp(n1, n2) != 0) { show_info(parent, "Les mots de passe ne correspondent pas."); }
        else if (modifier_password_utilisateur(g_bib, g_user->id, old, n1)) { sauvegarder_tout(g_bib); show_info(parent, "Mot de passe modifié."); }
        else show_info(parent, "Échec: ancien mot de passe incorrect.");
    }
    gtk_widget_destroy(dialog);
}

// Pay penalties dialog
static void on_pay_penalties(GtkButton *btn, gpointer data) {
    (void)btn;
    GtkWindow *parent = GTK_WINDOW(data);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Payer pénalités", parent, GTK_DIALOG_MODAL, "Payer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog)); GtkWidget *grid = gtk_grid_new(); gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *e_amount = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(e_amount), "0.00");
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Montant (€):"), 0,0,1,1); gtk_grid_attach(GTK_GRID(grid), e_amount, 1,0,1,1);
    gtk_widget_show_all(dialog);
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        double amt = atof(gtk_entry_get_text(GTK_ENTRY(e_amount)));
        if (amt <= 0) show_info(parent, "Montant invalide.");
        else if (payer_penalites(g_bib, g_user->id, amt)) { sauvegarder_tout(g_bib); show_info(parent, "Paiement enregistré."); }
        else show_info(parent, "Erreur lors du paiement.");
    }
    gtk_widget_destroy(dialog);
}

// Create account from login
static void on_create_account(GtkButton *btn, gpointer data) {
    (void)btn;
    GtkWindow *parent = GTK_WINDOW(data);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Créer un compte", parent, GTK_DIALOG_MODAL, "Créer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog)); GtkWidget *grid = gtk_grid_new(); gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *e_nom = gtk_entry_new(); GtkWidget *e_prenom = gtk_entry_new(); GtkWidget *e_id = gtk_entry_new(); GtkWidget *e_email = gtk_entry_new(); GtkWidget *e_pass = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Nom:"), 0,0,1,1); gtk_grid_attach(GTK_GRID(grid), e_nom, 1,0,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Prénom:"), 0,1,1,1); gtk_grid_attach(GTK_GRID(grid), e_prenom, 1,1,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ID étudiant:"), 0,2,1,1); gtk_grid_attach(GTK_GRID(grid), e_id, 1,2,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Email:"), 0,3,1,1); gtk_grid_attach(GTK_GRID(grid), e_email, 1,3,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Mot de passe:"), 0,4,1,1); gtk_grid_attach(GTK_GRID(grid), e_pass, 1,4,1,1);
    gtk_widget_show_all(dialog);
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        const char *nom = gtk_entry_get_text(GTK_ENTRY(e_nom));
        const char *prenom = gtk_entry_get_text(GTK_ENTRY(e_prenom));
        const char *id_et = gtk_entry_get_text(GTK_ENTRY(e_id));
        const char *email = gtk_entry_get_text(GTK_ENTRY(e_email));
        const char *pass = gtk_entry_get_text(GTK_ENTRY(e_pass));
        if (ajouter_utilisateur(g_bib, nom, prenom, id_et, email, pass, UTILISATEUR_SIMPLE)) { sauvegarder_tout(g_bib); show_info(parent, "Compte créé."); }
        else show_info(parent, "Erreur création compte (email ou id déjà utilisé).");
    }
    gtk_widget_destroy(dialog);
}


// Refresh children of a container that holds the books view
static void refresh_books_container(GtkWidget *container) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(container));
    for (GList *l = children; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(children);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *view = create_books_view();
    gtk_container_add(GTK_CONTAINER(scrolled), view);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_container_add(GTK_CONTAINER(container), scrolled);
    gtk_widget_show_all(container);
}

// Dialog to add a book
static void on_add_book(GtkButton *btn, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Ajouter un livre", parent, GTK_DIALOG_MODAL, "Ajouter", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new(); gtk_container_add(GTK_CONTAINER(content), grid);

    GtkWidget *e_titre = gtk_entry_new(); GtkWidget *e_auteur = gtk_entry_new(); GtkWidget *e_isbn = gtk_entry_new(); GtkWidget *e_cat = gtk_entry_new(); GtkWidget *e_annee = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Titre:"), 0,0,1,1); gtk_grid_attach(GTK_GRID(grid), e_titre, 1,0,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Auteur:"), 0,1,1,1); gtk_grid_attach(GTK_GRID(grid), e_auteur, 1,1,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ISBN:"), 0,2,1,1); gtk_grid_attach(GTK_GRID(grid), e_isbn, 1,2,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Catégorie:"), 0,3,1,1); gtk_grid_attach(GTK_GRID(grid), e_cat, 1,3,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Année:"), 0,4,1,1); gtk_grid_attach(GTK_GRID(grid), e_annee, 1,4,1,1);

    gtk_widget_show_all(dialog);
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        const char *titre = gtk_entry_get_text(GTK_ENTRY(e_titre));
        const char *auteur = gtk_entry_get_text(GTK_ENTRY(e_auteur));
        const char *isbn = gtk_entry_get_text(GTK_ENTRY(e_isbn));
        const char *categorie = gtk_entry_get_text(GTK_ENTRY(e_cat));
        int annee = atoi(gtk_entry_get_text(GTK_ENTRY(e_annee)));
        if (ajouter_livre(g_bib, titre, auteur, isbn, categorie, annee)) {
            sauvegarder_tout(g_bib);
            show_info(parent, "Livre ajouté avec succès.");
        } else {
            show_info(parent, "Erreur lors de l'ajout du livre.");
        }
    }
    gtk_widget_destroy(dialog);
}

// Dialogs for users (add / edit / delete)
static void on_add_user(GtkButton *btn, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Ajouter un utilisateur", parent, GTK_DIALOG_MODAL, "Ajouter", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new(); gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *e_nom = gtk_entry_new(); GtkWidget *e_prenom = gtk_entry_new(); GtkWidget *e_id = gtk_entry_new(); GtkWidget *e_email = gtk_entry_new(); GtkWidget *e_pass = gtk_entry_new();
    GtkWidget *cb_type = gtk_combo_box_text_new(); gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(cb_type), "Utilisateur"); gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(cb_type), "Admin"); gtk_combo_box_set_active(GTK_COMBO_BOX(cb_type), 0);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Nom:"), 0,0,1,1); gtk_grid_attach(GTK_GRID(grid), e_nom, 1,0,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Prénom:"), 0,1,1,1); gtk_grid_attach(GTK_GRID(grid), e_prenom, 1,1,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ID étudiant:"), 0,2,1,1); gtk_grid_attach(GTK_GRID(grid), e_id, 1,2,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Email:"), 0,3,1,1); gtk_grid_attach(GTK_GRID(grid), e_email, 1,3,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Mot de passe:"), 0,4,1,1); gtk_grid_attach(GTK_GRID(grid), e_pass, 1,4,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Type:"), 0,5,1,1); gtk_grid_attach(GTK_GRID(grid), cb_type, 1,5,1,1);
    gtk_widget_show_all(dialog);
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        const char *nom = gtk_entry_get_text(GTK_ENTRY(e_nom));
        const char *prenom = gtk_entry_get_text(GTK_ENTRY(e_prenom));
        const char *id_et = gtk_entry_get_text(GTK_ENTRY(e_id));
        const char *email = gtk_entry_get_text(GTK_ENTRY(e_email));
        const char *pass = gtk_entry_get_text(GTK_ENTRY(e_pass));
        int type = gtk_combo_box_get_active(GTK_COMBO_BOX(cb_type));
        if (ajouter_utilisateur(g_bib, nom, prenom, id_et, email, pass, (TypeUtilisateur)type)) {
            sauvegarder_tout(g_bib);
            show_info(parent, "Utilisateur ajouté.");
        } else {
            show_info(parent, "Erreur lors de l'ajout.");
        }
    }
    gtk_widget_destroy(dialog);
}

static void on_delete_user(GtkButton *btn, gpointer data) {
    GtkWidget *tree = GTK_WIDGET(data);
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    GtkTreeModel *model; GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
        int id; gtk_tree_model_get(model, &iter, 0, &id, -1);
        if (supprimer_utilisateur(g_bib, id)) {
            sauvegarder_tout(g_bib);
            show_info(NULL, "Utilisateur supprimé.");
            GtkWidget *parent = gtk_widget_get_parent(gtk_widget_get_parent(tree));
            refresh_users_container(parent);
        } else {
            show_info(NULL, "Erreur suppression (utilisateur peut avoir des emprunts actifs).");
        }
    } else {
        show_info(NULL, "Aucun utilisateur sélectionné.");
    }
}

static void on_edit_user(GtkButton *btn, gpointer data) {
    GtkWidget *tree = GTK_WIDGET(data);
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    GtkTreeModel *model; GtkTreeIter iter;
    if (!gtk_tree_selection_get_selected(sel, &model, &iter)) { show_info(NULL, "Aucun utilisateur sélectionné."); return; }
    int id; gtk_tree_model_get(model, &iter, 0, &id, -1);
    Utilisateur *U = rechercher_utilisateur_par_id(g_bib, id);
    if (!U) { show_info(NULL, "Utilisateur introuvable."); return; }
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Modifier utilisateur", NULL, GTK_DIALOG_MODAL, "Enregistrer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog)); GtkWidget *grid = gtk_grid_new(); gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *e_nom = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(e_nom), U->nom);
    GtkWidget *e_prenom = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(e_prenom), U->prenom);
    GtkWidget *e_id = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(e_id), U->id_etudiant);
    GtkWidget *e_email = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(e_email), U->email);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Nom:"), 0,0,1,1); gtk_grid_attach(GTK_GRID(grid), e_nom, 1,0,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Prénom:"), 0,1,1,1); gtk_grid_attach(GTK_GRID(grid), e_prenom, 1,1,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ID étudiant:"), 0,2,1,1); gtk_grid_attach(GTK_GRID(grid), e_id, 1,2,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Email:"), 0,3,1,1); gtk_grid_attach(GTK_GRID(grid), e_email, 1,3,1,1);
    gtk_widget_show_all(dialog);
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        const char *nom = gtk_entry_get_text(GTK_ENTRY(e_nom));
        const char *prenom = gtk_entry_get_text(GTK_ENTRY(e_prenom));
        const char *id_et = gtk_entry_get_text(GTK_ENTRY(e_id));
        const char *email = gtk_entry_get_text(GTK_ENTRY(e_email));
        if (modifier_utilisateur(g_bib, id, nom, prenom, id_et, email)) {
            sauvegarder_tout(g_bib);
            show_info(NULL, "Utilisateur modifié.");
            GtkWidget *parent = gtk_widget_get_parent(gtk_widget_get_parent(tree));
            refresh_users_container(parent);
        } else {
            show_info(NULL, "Erreur modification.");
        }
    }
    gtk_widget_destroy(dialog);
}

// Emprunts actions: emprunter / retour
static void on_enregistrer_emprunt(GtkButton *btn, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Enregistrer emprunt", parent, GTK_DIALOG_MODAL, "Enregistrer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog)); GtkWidget *grid = gtk_grid_new(); gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *e_id_livre = gtk_entry_new(); GtkWidget *e_id_user = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ID livre:"), 0,0,1,1); gtk_grid_attach(GTK_GRID(grid), e_id_livre, 1,0,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ID utilisateur:"), 0,1,1,1); gtk_grid_attach(GTK_GRID(grid), e_id_user, 1,1,1,1);
    gtk_widget_show_all(dialog);
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        int id_l = atoi(gtk_entry_get_text(GTK_ENTRY(e_id_livre)));
        int id_u = atoi(gtk_entry_get_text(GTK_ENTRY(e_id_user)));
        if (enregistrer_emprunt(g_bib, id_l, id_u)) { sauvegarder_tout(g_bib); show_info(parent, "Emprunt enregistré."); }
        else show_info(parent, "Erreur: emprunt non enregistré.");
    }
    gtk_widget_destroy(dialog);
}

static void on_enregistrer_retour(GtkButton *btn, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Enregistrer retour", parent, GTK_DIALOG_MODAL, "Enregistrer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog)); GtkWidget *grid = gtk_grid_new(); gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *e_id_livre = gtk_entry_new(); GtkWidget *e_id_user = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ID livre:"), 0,0,1,1); gtk_grid_attach(GTK_GRID(grid), e_id_livre, 1,0,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ID utilisateur:"), 0,1,1,1); gtk_grid_attach(GTK_GRID(grid), e_id_user, 1,1,1,1);
    gtk_widget_show_all(dialog);
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        int id_l = atoi(gtk_entry_get_text(GTK_ENTRY(e_id_livre)));
        int id_u = atoi(gtk_entry_get_text(GTK_ENTRY(e_id_user)));
        if (enregistrer_retour(g_bib, id_l, id_u)) { sauvegarder_tout(g_bib); show_info(parent, "Retour enregistré."); }
        else show_info(parent, "Erreur: retour non enregistré.");
    }
    gtk_widget_destroy(dialog);
}

// Save callback for GTK button
static void on_save_clicked(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    if (g_bib) {
        sauvegarder_tout(g_bib);
        show_info(NULL, "Données sauvegardées.");
    }
}

// Delete selected book
static void on_delete_book(GtkButton *btn, gpointer data) {
    GtkWidget *tree = GTK_WIDGET(data);
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    GtkTreeModel *model; GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
        int id; gtk_tree_model_get(model, &iter, 0, &id, -1);
        if (supprimer_livre(g_bib, id)) {
            sauvegarder_tout(g_bib);
            show_info(NULL, "Livre supprimé.");
            // parent container refresh
            GtkWidget *parent = gtk_widget_get_parent(gtk_widget_get_parent(tree));
            refresh_books_container(parent);
        } else {
            show_info(NULL, "Erreur suppression.");
        }
    } else {
        show_info(NULL, "Aucun livre sélectionné.");
    }
}

// Edit selected book
static void on_edit_book(GtkButton *btn, gpointer data) {
    GtkWidget *tree = GTK_WIDGET(data);
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    GtkTreeModel *model; GtkTreeIter iter;
    if (!gtk_tree_selection_get_selected(sel, &model, &iter)) { show_info(NULL, "Aucun livre sélectionné."); return; }
    int id; gtk_tree_model_get(model, &iter, 0, &id, -1);
    Livre *L = rechercher_livre_par_id(g_bib, id);
    if (!L) { show_info(NULL, "Livre introuvable."); return; }

    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(tree));
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Modifier un livre", parent, GTK_DIALOG_MODAL, "Enregistrer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog)); GtkWidget *grid = gtk_grid_new(); gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *e_titre = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(e_titre), L->titre);
    GtkWidget *e_auteur = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(e_auteur), L->auteur);
    GtkWidget *e_isbn = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(e_isbn), L->isbn);
    GtkWidget *e_cat = gtk_entry_new(); gtk_entry_set_text(GTK_ENTRY(e_cat), L->categorie);
    GtkWidget *e_annee = gtk_entry_new(); char anbuf[16]; snprintf(anbuf, sizeof(anbuf), "%d", L->annee); gtk_entry_set_text(GTK_ENTRY(e_annee), anbuf);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Titre:"), 0,0,1,1); gtk_grid_attach(GTK_GRID(grid), e_titre, 1,0,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Auteur:"), 0,1,1,1); gtk_grid_attach(GTK_GRID(grid), e_auteur, 1,1,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ISBN:"), 0,2,1,1); gtk_grid_attach(GTK_GRID(grid), e_isbn, 1,2,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Catégorie:"), 0,3,1,1); gtk_grid_attach(GTK_GRID(grid), e_cat, 1,3,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Année:"), 0,4,1,1); gtk_grid_attach(GTK_GRID(grid), e_annee, 1,4,1,1);
    gtk_widget_show_all(dialog);
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        const char *titre = gtk_entry_get_text(GTK_ENTRY(e_titre));
        const char *auteur = gtk_entry_get_text(GTK_ENTRY(e_auteur));
        const char *isbn = gtk_entry_get_text(GTK_ENTRY(e_isbn));
        const char *categorie = gtk_entry_get_text(GTK_ENTRY(e_cat));
        int annee = atoi(gtk_entry_get_text(GTK_ENTRY(e_annee)));
        if (modifier_livre(g_bib, id, titre, auteur, isbn, categorie, annee)) {
            sauvegarder_tout(g_bib);
            show_info(parent, "Livre modifié.");
            GtkWidget *parent_container = gtk_widget_get_parent(gtk_widget_get_parent(tree));
            refresh_books_container(parent_container);
        } else {
            show_info(parent, "Erreur modification livre.");
        }
    }
    gtk_widget_destroy(dialog);
}

// Borrow selected book (create emprunt)
static void on_borrow_book(GtkButton *btn, gpointer data) {
    GtkWidget *tree = GTK_WIDGET(data);
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    GtkTreeModel *model; GtkTreeIter iter;
    if (!gtk_tree_selection_get_selected(sel, &model, &iter)) { show_info(NULL, "Aucun livre sélectionné."); return; }
    int id_livre; gtk_tree_model_get(model, &iter, 0, &id_livre, -1);
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(tree));
    int id_user = g_user ? g_user->id : -1;
    if (id_user != -1) {
        if (enregistrer_emprunt(g_bib, id_livre, id_user)) {
            sauvegarder_tout(g_bib);
            show_info(parent, "Emprunt enregistré pour l'utilisateur connecté.");
            GtkWidget *parent_container = gtk_widget_get_parent(gtk_widget_get_parent(tree));
            refresh_books_container(parent_container);
        } else {
            show_info(parent, "Erreur lors de l'enregistrement de l'emprunt.");
        }
        return;
    }
    /* fallback: ask for user id if nobody is logged */
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Enregistrer emprunt (par livre)", parent, GTK_DIALOG_MODAL, "Enregistrer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog)); GtkWidget *grid = gtk_grid_new(); gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *e_user = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ID utilisateur:"), 0,0,1,1); gtk_grid_attach(GTK_GRID(grid), e_user, 1,0,1,1);
    gtk_widget_show_all(dialog);
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        int id_user_entered = atoi(gtk_entry_get_text(GTK_ENTRY(e_user)));
        if (enregistrer_emprunt(g_bib, id_livre, id_user_entered)) {
            sauvegarder_tout(g_bib);
            show_info(parent, "Emprunt enregistré.");
            GtkWidget *parent_container = gtk_widget_get_parent(gtk_widget_get_parent(tree));
            refresh_books_container(parent_container);
        } else {
            show_info(parent, "Erreur lors de l'enregistrement de l'emprunt.");
        }
    }
    gtk_widget_destroy(dialog);
}

// Return selected book (rendre)
static void on_return_book(GtkButton *btn, gpointer data) {
    GtkWidget *tree = GTK_WIDGET(data);
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    GtkTreeModel *model; GtkTreeIter iter;
    if (!gtk_tree_selection_get_selected(sel, &model, &iter)) { show_info(NULL, "Aucun livre sélectionné."); return; }
    int id_livre; gtk_tree_model_get(model, &iter, 0, &id_livre, -1);
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(tree));
    int id_user = g_user ? g_user->id : -1;
    if (id_user != -1) {
        if (enregistrer_retour(g_bib, id_livre, id_user)) {
            sauvegarder_tout(g_bib);
            show_info(parent, "Retour enregistré pour l'utilisateur connecté.");
            GtkWidget *parent_container = gtk_widget_get_parent(gtk_widget_get_parent(tree));
            refresh_books_container(parent_container);
        } else {
            show_info(parent, "Erreur lors de l'enregistrement du retour.");
        }
        return;
    }
    /* fallback: ask for user id */
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Enregistrer retour (par livre)", parent, GTK_DIALOG_MODAL, "Enregistrer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog)); GtkWidget *grid = gtk_grid_new(); gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *e_user = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ID utilisateur:"), 0,0,1,1); gtk_grid_attach(GTK_GRID(grid), e_user, 1,0,1,1);
    gtk_widget_show_all(dialog);
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        int id_user_entered = atoi(gtk_entry_get_text(GTK_ENTRY(e_user)));
        if (enregistrer_retour(g_bib, id_livre, id_user_entered)) {
            sauvegarder_tout(g_bib);
            show_info(parent, "Retour enregistré.");
            GtkWidget *parent_container = gtk_widget_get_parent(gtk_widget_get_parent(tree));
            refresh_books_container(parent_container);
        } else {
            show_info(parent, "Erreur lors de l'enregistrement du retour.");
        }
    }
    gtk_widget_destroy(dialog);
}

// Build main window after login
// Build the main content inside the provided container (this will be the "main_page" of the stack).
typedef struct {
    GtkWidget *main_container;
    GtkWidget *notebook;
    GtkWidget *search_box;
    GtkWidget *histo_box;
    GtkWidget *current_box;
} MainCtx;

static void on_search_button_cb(GtkButton *b, gpointer ud) {
    (void)b;
    MainCtx *ctx = (MainCtx *)ud;
    GtkWidget *parent = gtk_widget_get_toplevel(ctx->main_container);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Recherche de livre", GTK_WINDOW(parent), GTK_DIALOG_MODAL, "Rechercher", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content), grid);
    GtkWidget *e_titre = gtk_entry_new(); GtkWidget *e_auteur = gtk_entry_new(); GtkWidget *e_cat = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Titre:"), 0,0,1,1); gtk_grid_attach(GTK_GRID(grid), e_titre, 1,0,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Auteur:"), 0,1,1,1); gtk_grid_attach(GTK_GRID(grid), e_auteur, 1,1,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Catégorie:"), 0,2,1,1); gtk_grid_attach(GTK_GRID(grid), e_cat, 1,2,1,1);
    gtk_widget_show_all(dialog);
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_OK) {
        const char *t = gtk_entry_get_text(GTK_ENTRY(e_titre));
        const char *a = gtk_entry_get_text(GTK_ENTRY(e_auteur));
        const char *c = gtk_entry_get_text(GTK_ENTRY(e_cat));
        // clear search_box and add results
        GList *kids = gtk_container_get_children(GTK_CONTAINER(ctx->search_box));
        for (GList *l = kids; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
        g_list_free(kids);
        GtkWidget *view = create_search_results_view(t, a, c);
        GtkWidget *sc = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add(GTK_CONTAINER(sc), view);
        gtk_widget_set_vexpand(sc, TRUE);
        gtk_container_add(GTK_CONTAINER(ctx->search_box), sc);
        gtk_widget_show_all(ctx->search_box);
        gtk_notebook_set_current_page(GTK_NOTEBOOK(ctx->notebook), 1);
    }
    gtk_widget_destroy(dialog);
}

static void on_show_history_cb(GtkButton *b, gpointer ud) {
    (void)b;
    MainCtx *ctx = (MainCtx *)ud;
    GList *kids = gtk_container_get_children(GTK_CONTAINER(ctx->histo_box));
    for (GList *l = kids; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(kids);
    GtkWidget *view = create_user_history_view(g_user->id);
    GtkWidget *sc = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(sc), view);
    gtk_widget_set_vexpand(sc, TRUE);
    gtk_container_add(GTK_CONTAINER(ctx->histo_box), sc);
    gtk_widget_show_all(ctx->histo_box);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(ctx->notebook), 2);
}

static void on_show_current_cb(GtkButton *b, gpointer ud) {
    (void)b;
    MainCtx *ctx = (MainCtx *)ud;
    GList *kids = gtk_container_get_children(GTK_CONTAINER(ctx->current_box));
    for (GList *l = kids; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(kids);
    GtkWidget *view = create_current_loans_view(g_user->id);
    GtkWidget *sc = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(sc), view);
    gtk_widget_set_vexpand(sc, TRUE);
    gtk_container_add(GTK_CONTAINER(ctx->current_box), sc);
    gtk_widget_show_all(ctx->current_box);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(ctx->notebook), 3);
}

static void on_logout_cb(GtkButton *b, gpointer ud) {
    (void)b;
    MainCtx *ctx = (MainCtx *)ud;
    g_user = NULL;
    g_last_user = NULL;
    clear_last_user_id();
    GtkWidget *parent = ctx->main_container;
    GtkWidget *stack = gtk_widget_get_parent(parent);
    if (GTK_IS_STACK(stack)) gtk_stack_set_visible_child_name(GTK_STACK(stack), "login_page");
}

static void build_main_content(GtkWidget *main_container) {
    // top: user info and buttons
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add(GTK_CONTAINER(main_container), vbox);

    // Build a compact main layout: left nav + notebook
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 6);

    GtkWidget *nav = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_size_request(nav, 220, -1);
    gtk_box_pack_start(GTK_BOX(hbox), nav, FALSE, FALSE, 6);

    GtkWidget *right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_pack_start(GTK_BOX(hbox), right, TRUE, TRUE, 6);

    GtkWidget *lbl_title = gtk_label_new("SYSTÈME DE GESTION - Bibliothèque");
    gtk_box_pack_start(GTK_BOX(nav), lbl_title, FALSE, FALSE, 6);

    char userinfo[256]; snprintf(userinfo, sizeof(userinfo), "%s %s\nEmprunts actifs: %d\nPénalités: %.2f €",
                                 g_user->prenom, g_user->nom, g_user->nombre_emprunts_actifs, g_user->penalites);
    GtkWidget *lbl_user = gtk_label_new(userinfo);
    gtk_label_set_xalign(GTK_LABEL(lbl_user), 0.0);
    gtk_box_pack_start(GTK_BOX(nav), lbl_user, FALSE, FALSE, 6);

    GtkWidget *btn_catalogue = gtk_button_new_with_label("Consulter le catalogue"); gtk_box_pack_start(GTK_BOX(nav), btn_catalogue, FALSE, FALSE, 4);
    GtkWidget *btn_recherche = gtk_button_new_with_label("Rechercher un livre"); gtk_box_pack_start(GTK_BOX(nav), btn_recherche, FALSE, FALSE, 4);
    GtkWidget *btn_histo = gtk_button_new_with_label("Mon historique d'emprunts"); gtk_box_pack_start(GTK_BOX(nav), btn_histo, FALSE, FALSE, 4);
    GtkWidget *btn_current = gtk_button_new_with_label("Mes emprunts en cours"); gtk_box_pack_start(GTK_BOX(nav), btn_current, FALSE, FALSE, 4);
    GtkWidget *btn_pw = gtk_button_new_with_label("Modifier mon mot de passe"); gtk_box_pack_start(GTK_BOX(nav), btn_pw, FALSE, FALSE, 4);
    GtkWidget *btn_pay = gtk_button_new_with_label("Payer mes pénalités"); gtk_box_pack_start(GTK_BOX(nav), btn_pay, FALSE, FALSE, 4);
    GtkWidget *btn_logout = gtk_button_new_with_label("Se déconnecter"); gtk_box_pack_end(GTK_BOX(nav), btn_logout, FALSE, FALSE, 6);

    gtk_box_set_homogeneous(GTK_BOX(right), FALSE);
    GtkWidget *action_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(right), action_bar, FALSE, FALSE, 6);
    GtkWidget *btn_save = gtk_button_new_with_label("Sauvegarder"); gtk_box_pack_end(GTK_BOX(action_bar), btn_save, FALSE, FALSE, 0);
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save_clicked), NULL);

    // Page title (single display of active page name)
    GtkWidget *page_title = gtk_label_new("Catalogue");
    gtk_widget_set_halign(page_title, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(right), page_title, FALSE, FALSE, 4);

    GtkWidget *notebook = gtk_notebook_new();
    /* hide tab headers — navigation comes from the left menu; update title via switch-page */
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
    g_signal_connect(notebook, "switch-page", G_CALLBACK(on_notebook_switch_page), page_title);
    gtk_box_pack_start(GTK_BOX(right), notebook, TRUE, TRUE, 6);

    // Pages
    GtkWidget *books_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    // Controls: add / edit / delete / emprunter / rendre (kept separate from the list container)
    GtkWidget *books_ctrl = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *btn_b_add = gtk_button_new_with_label("Ajouter livre");
    GtkWidget *btn_b_edit = gtk_button_new_with_label("Modifier livre");
    GtkWidget *btn_b_del = gtk_button_new_with_label("Supprimer livre");
    GtkWidget *btn_b_borrow = gtk_button_new_with_label("Emprunter");
    GtkWidget *btn_b_return = gtk_button_new_with_label("Rendre");
    gtk_box_pack_start(GTK_BOX(books_ctrl), btn_b_add, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(books_ctrl), btn_b_edit, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(books_ctrl), btn_b_del, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(books_ctrl), btn_b_borrow, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(books_ctrl), btn_b_return, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(books_box), books_ctrl, FALSE, FALSE, 6);

    /* Dedicated container for the books list so refresh doesn't remove controls */
    GtkWidget *books_list_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    refresh_books_container(books_list_container);
    gtk_box_pack_start(GTK_BOX(books_box), books_list_container, TRUE, TRUE, 6);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), books_box, gtk_label_new("Catalogue"));

    /* Try to find the tree view inside books_list_container to connect the buttons */
    GtkWidget *books_tree = NULL;
    GList *children = gtk_container_get_children(GTK_CONTAINER(books_list_container));
    for (GList *l = children; l != NULL; l = l->next) {
        if (GTK_IS_SCROLLED_WINDOW(l->data)) {
            GList *c2 = gtk_container_get_children(GTK_CONTAINER(l->data));
            if (c2 && GTK_IS_TREE_VIEW(c2->data)) {
                books_tree = GTK_WIDGET(c2->data);
            }
            g_list_free(c2);
        }
    }
    g_list_free(children);
    if (books_tree) {
        g_signal_connect(btn_b_add, "clicked", G_CALLBACK(on_add_book), gtk_widget_get_toplevel(books_box));
        g_signal_connect(btn_b_edit, "clicked", G_CALLBACK(on_edit_book), books_tree);
        g_signal_connect(btn_b_del, "clicked", G_CALLBACK(on_delete_book), books_tree);
        g_signal_connect(btn_b_borrow, "clicked", G_CALLBACK(on_borrow_book), books_tree);
        g_signal_connect(btn_b_return, "clicked", G_CALLBACK(on_return_book), books_tree);
    } else {
        /* fallback: connect add only */
        g_signal_connect(btn_b_add, "clicked", G_CALLBACK(on_add_book), gtk_widget_get_toplevel(books_box));
    }

    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_pack_start(GTK_BOX(search_box), gtk_label_new("Utilisez 'Rechercher un livre' dans le menu pour lancer une recherche."), FALSE, FALSE, 6);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), search_box, gtk_label_new("Recherche"));

    GtkWidget *histo_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), histo_box, gtk_label_new("Historique"));

    GtkWidget *current_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    /* Controls for current loans: emprunter & rendre */
    GtkWidget *current_ctrl = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *btn_m_borrow = gtk_button_new_with_label("Emprunter");
    GtkWidget *btn_m_return = gtk_button_new_with_label("Rendre sélection");
    gtk_box_pack_start(GTK_BOX(current_ctrl), btn_m_borrow, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(current_ctrl), btn_m_return, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(current_box), current_ctrl, FALSE, FALSE, 6);
    /* container that will hold the current loans list */
    GtkWidget *current_list_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    refresh_current_container(current_list_container, g_user ? g_user->id : -1);
    gtk_box_pack_start(GTK_BOX(current_box), current_list_container, TRUE, TRUE, 6);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), current_box, gtk_label_new("Mes emprunts"));

    GtkWidget *stats_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    build_stats_into_container(stats_box);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), stats_box, gtk_label_new("Statistiques"));

    // Prepare context for callbacks
    MainCtx *ctx = g_malloc0(sizeof(MainCtx));
    ctx->main_container = main_container;
    ctx->notebook = notebook;
    ctx->search_box = search_box;
    ctx->histo_box = histo_box;
    ctx->current_box = current_box;

    // Connect nav buttons
    PageSwitch *ps_cat = g_malloc(sizeof(PageSwitch)); ps_cat->nb = GTK_NOTEBOOK(notebook); ps_cat->idx = 0;
    g_signal_connect(btn_catalogue, "clicked", G_CALLBACK(switch_page_cb), ps_cat);
    g_signal_connect(btn_recherche, "clicked", G_CALLBACK(on_search_button_cb), ctx);
    g_signal_connect(btn_histo, "clicked", G_CALLBACK(on_show_history_cb), ctx);
    g_signal_connect(btn_current, "clicked", G_CALLBACK(on_show_current_cb), ctx);
    /* connect current page buttons: need to find the tree inside current_list_container */
    {
        GtkWidget *current_tree = NULL;
        GList *kids = gtk_container_get_children(GTK_CONTAINER(current_list_container));
        for (GList *l = kids; l != NULL; l = l->next) {
            if (GTK_IS_SCROLLED_WINDOW(l->data)) {
                GList *c2 = gtk_container_get_children(GTK_CONTAINER(l->data));
                if (c2 && GTK_IS_TREE_VIEW(c2->data)) current_tree = GTK_WIDGET(c2->data);
                g_list_free(c2);
            }
        }
        g_list_free(kids);
        if (current_tree) {
            g_signal_connect(btn_m_return, "clicked", G_CALLBACK(on_return_book), current_tree);
            g_signal_connect(btn_m_borrow, "clicked", G_CALLBACK(on_borrow_book), current_tree);
        } else {
            g_signal_connect(btn_m_borrow, "clicked", G_CALLBACK(on_borrow_book), gtk_widget_get_toplevel(current_box));
        }
    }
    g_signal_connect(btn_pw, "clicked", G_CALLBACK(on_change_password), gtk_widget_get_toplevel(main_container));
    g_signal_connect(btn_pay, "clicked", G_CALLBACK(on_pay_penalties), gtk_widget_get_toplevel(main_container));
    g_signal_connect(btn_logout, "clicked", G_CALLBACK(on_logout_cb), ctx);

    gtk_widget_show_all(main_container);
}

// Login window handler
// Structure passed to the login callback
struct LoginContext { GtkStack *stack; GtkEntry *e_email; GtkEntry *e_pass; GtkWidget *main_page; };

static void on_login_clicked(GtkButton *btn, gpointer data) {
    struct LoginContext *ctx = data;
    const char *email = gtk_entry_get_text(ctx->e_email);
    const char *pass = gtk_entry_get_text(ctx->e_pass);
    Utilisateur *u = authentifier(g_bib, email, pass);
    if (!u) {
        show_info(NULL, "Email ou mot de passe invalide.");
        return;
    }
    g_user = u;
    g_last_user = u; /* remember in RAM for auto-login during this run */
    save_last_user_id(u->id);
    // Clear any previous main_page contents (avoid duplication when re-logging)
    GList *kids = gtk_container_get_children(GTK_CONTAINER(ctx->main_page));
    for (GList *l = kids; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(kids);
    // Build main content into the main_page container and switch the stack to it
    build_main_content(ctx->main_page);
    gtk_stack_set_visible_child_name(ctx->stack, "main_page");
}

void run_ui_app(Bibliotheque *bib) {
    g_bib = bib;
    int argc = 0; char **argv = NULL;
    gtk_init(&argc, &argv);

    // Attempt to auto-login from persisted last user id (if any)
    int last_id = read_last_user_id();
    if (last_id != -1) {
        Utilisateur *lu = rechercher_utilisateur_par_id(g_bib, last_id);
        if (lu) g_last_user = lu;
    }

    // Create a single top-level window that will contain both the login view and the main application pages.
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 700);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Header bar
    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Système de gestion de bibliothèque");
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), "Interface graphique");
    gtk_window_set_titlebar(GTK_WINDOW(window), header);
    // small action icon on header
    GtkWidget *hb_save = gtk_button_new(); gtk_button_set_always_show_image(GTK_BUTTON(hb_save), TRUE);
    gtk_button_set_image(GTK_BUTTON(hb_save), gtk_image_new_from_icon_name("document-save", GTK_ICON_SIZE_BUTTON));
    gtk_tool_item_set_tooltip_text ? NULL : NULL; /* noop to avoid unused warnings in some builds */
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), hb_save);

    // Stack will switch between the login page and the main page inside the same window
    GtkWidget *stack = gtk_stack_new();
    gtk_container_add(GTK_CONTAINER(window), stack);

    // --- Login page (first view) ---
    GtkWidget *login_page = gtk_grid_new();
    GtkWidget *e_email = gtk_entry_new(); GtkWidget *e_pass = gtk_entry_new(); gtk_entry_set_visibility(GTK_ENTRY(e_pass), FALSE);
    gtk_grid_attach(GTK_GRID(login_page), gtk_label_new("Email:"), 0,0,1,1); gtk_grid_attach(GTK_GRID(login_page), e_email, 1,0,1,1);
    gtk_grid_attach(GTK_GRID(login_page), gtk_label_new("Mot de passe:"), 0,1,1,1); gtk_grid_attach(GTK_GRID(login_page), e_pass, 1,1,1,1);
    GtkWidget *btn = gtk_button_new_with_label("Se connecter"); gtk_grid_attach(GTK_GRID(login_page), btn, 1,2,1,1);
    gtk_stack_add_named(GTK_STACK(stack), login_page, "login_page");

    // --- Main page (hidden until login) ---
    GtkWidget *main_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_stack_add_named(GTK_STACK(stack), main_page, "main_page");

    // Prepare login context and connect
    struct LoginContext *lctx = g_new(struct LoginContext, 1);
    lctx->stack = GTK_STACK(stack);
    lctx->e_email = GTK_ENTRY(e_email);
    lctx->e_pass = GTK_ENTRY(e_pass);
    lctx->main_page = main_page;
    g_signal_connect(btn, "clicked", G_CALLBACK(on_login_clicked), lctx);

    // If we have a remembered user in RAM, auto-login without asking
    if (g_last_user) {
        g_user = g_last_user;
        build_main_content(main_page);
        gtk_stack_set_visible_child_name(GTK_STACK(stack), "main_page");
    }

    // Show the window (stack will show the login page first)
    gtk_widget_show_all(window);
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "login_page");
    gtk_main();
}
    // End of GTK UI implementation in this file.
