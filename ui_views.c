#include "ui_views.h"
#include "ui_helpers.h"
#include "structure.h"
#include "livres.h"
#include "utilisateurs.h"
#include "emprunts.h"
#include <gtk/gtk.h>
#include <time.h>
#include <stdio.h>

extern Bibliotheque *g_bib; /* provided by ui.c */

GtkWidget* create_books_view() {
    GtkListStore *store = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
    for (int i = 0; i < g_bib->nombre_livres; ++i) {
        Livre *L = &g_bib->livres[i];
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        const char *statut = (L->statut == EMPRUNTE) ? "Emprunté" : "Disponible";
        gtk_list_store_set(store, &iter,
                           0, L->id,
                           1, L->titre,
                           2, L->auteur,
                           3, L->categorie,
                           4, L->annee,
                           5, statut,
                           -1);
    }
    const char *titles[] = { "ID", "Titre", "Auteur", "Catégorie", "Année", "Statut" };
    GtkWidget *view = tree_view_with_columns(store, titles, 6);
    return view;
}

GtkWidget* create_users_view() {
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
    const char *titles[] = { "ID", "Nom", "Prénom", "ID étudiant", "Email", "Emprunts actifs", "Pénalités (€)" };
    GtkWidget *view = tree_view_with_columns(store, titles, 7);
    return view;
}

GtkWidget* create_emprunts_view() {
    GtkListStore *store = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE);
    for (int i = 0; i < g_bib->nombre_emprunts; ++i) {
        Emprunt *E = &g_bib->emprunts[i];
        if (E->date_retour_effectif != 0) continue;
        Livre *L = rechercher_livre_par_id(g_bib, E->id_livre);
        Utilisateur *U = rechercher_utilisateur_par_id(g_bib, E->id_utilisateur);
        char buf_date[64]; struct tm tmv; localtime_r(&E->date_emprunt, &tmv);
        strftime(buf_date, sizeof(buf_date), "%Y-%m-%d", &tmv);
        GtkTreeIter iter; gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, E->id,
                           1, L ? L->titre : "(livre supprimé)",
                           2, U ? U->prenom : "(utilisateur)",
                           3, U ? U->nom : "",
                           4, buf_date,
                           5, E->penalite,
                           -1);
    }
    const char *titles[] = { "ID", "Livre", "Prénom", "Nom", "Date emprunt", "Pénalité (€)" };
    GtkWidget *view = tree_view_with_columns(store, titles, 6);
    return view;
}

GtkWidget* create_search_results_view(const char *titre, const char *auteur, const char *categorie) {
    Livre *resultats[MAX_LIVRES];
    int n = rechercher_livres_multi_criteres(g_bib, titre && *titre ? titre : NULL, auteur && *auteur ? auteur : NULL, categorie && *categorie ? categorie : NULL, resultats, MAX_LIVRES);
    GtkListStore *store = gtk_list_store_new(5, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
    for (int i = 0; i < n; ++i) {
        Livre *L = resultats[i]; GtkTreeIter iter; gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, L->id, 1, L->titre, 2, L->auteur, 3, L->categorie, 4, L->annee, -1);
    }
    const char *titles[] = { "ID", "Titre", "Auteur", "Catégorie", "Année" };
    GtkWidget *view = tree_view_with_columns(store, titles, 5);
    return view;
}

GtkWidget* create_user_history_view(int id_utilisateur) {
    GtkListStore *store = gtk_list_store_new(7, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE);
    for (int i = 0; i < g_bib->nombre_emprunts; ++i) {
        Emprunt *E = &g_bib->emprunts[i]; if (E->id_utilisateur != id_utilisateur) continue;
        Livre *L = rechercher_livre_par_id(g_bib, E->id_livre);
        char date_emp[32] = ""; char date_ret[32] = "(non rendu)"; struct tm tmv; localtime_r(&E->date_emprunt, &tmv); strftime(date_emp, sizeof(date_emp), "%Y-%m-%d", &tmv);
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
    const char *titles[] = { "ID", "Livre", "Date emprunt", "Date retour", "En retard", "", "Pénalité (€)" };
    GtkWidget *view = tree_view_with_columns(store, titles, 7);
    return view;
}

GtkWidget* create_current_loans_view(int id_utilisateur) {
    GtkListStore *store = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE);
    for (int i = 0; i < g_bib->nombre_emprunts; ++i) {
        Emprunt *E = &g_bib->emprunts[i]; if (E->id_utilisateur != id_utilisateur) continue; if (E->date_retour_effectif != 0) continue;
        Livre *L = rechercher_livre_par_id(g_bib, E->id_livre);
        char date_emp[32] = ""; struct tm tmv; localtime_r(&E->date_emprunt, &tmv); strftime(date_emp, sizeof(date_emp), "%Y-%m-%d", &tmv);
        GtkTreeIter iter; gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, E->id, 1, L ? L->titre : "(supprimé)", 2, date_emp, 3, "", 4, "", 5, E->penalite, -1);
    }
    const char *titles[] = { "ID", "Livre", "Date emprunt", "", "", "Pénalité (€)" };
    GtkWidget *view = tree_view_with_columns(store, titles, 6);
    return view;
}

void refresh_books_container(GtkWidget *container) {
    clear_container_children(container);
    GtkWidget *view = create_books_view();
    add_view_in_scrolled(container, view);
}

void refresh_users_container(GtkWidget *container) {
    clear_container_children(container);
    GtkWidget *view = create_users_view();
    add_view_in_scrolled(container, view);
}

void refresh_emprunts_container(GtkWidget *container) {
    clear_container_children(container);
    GtkWidget *view = create_emprunts_view();
    add_view_in_scrolled(container, view);
}

void refresh_current_container(GtkWidget *container, int id_utilisateur) {
    clear_container_children(container);
    GtkWidget *view = create_current_loans_view(id_utilisateur);
    add_view_in_scrolled(container, view);
}

void build_stats_into_container(GtkWidget *container) {
    clear_container_children(container);
    char buf[256]; GtkWidget *v = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    snprintf(buf, sizeof(buf), "Livres: %d", g_bib->nombre_livres); gtk_box_pack_start(GTK_BOX(v), gtk_label_new(buf), FALSE, FALSE, 4);
    snprintf(buf, sizeof(buf), "Utilisateurs: %d", g_bib->nombre_utilisateurs); gtk_box_pack_start(GTK_BOX(v), gtk_label_new(buf), FALSE, FALSE, 4);
    snprintf(buf, sizeof(buf), "Emprunts (total): %d", g_bib->nombre_emprunts); gtk_box_pack_start(GTK_BOX(v), gtk_label_new(buf), FALSE, FALSE, 4);
    int active = 0, en_retard = 0; time_t now = time(NULL);
    for (int i = 0; i < g_bib->nombre_emprunts; ++i) {
        Emprunt *E = &g_bib->emprunts[i]; if (E->date_retour_effectif == 0) { active++; if (difftime(now, E->date_retour_prevue) > 0) en_retard++; }
    }
    snprintf(buf, sizeof(buf), "Emprunts actifs: %d", active); gtk_box_pack_start(GTK_BOX(v), gtk_label_new(buf), FALSE, FALSE, 4);
    snprintf(buf, sizeof(buf), "Emprunts en retard: %d", en_retard); gtk_box_pack_start(GTK_BOX(v), gtk_label_new(buf), FALSE, FALSE, 4);
    gtk_container_add(GTK_CONTAINER(container), v); gtk_widget_show_all(container);
}
