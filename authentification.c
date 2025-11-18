//
// Autentification – Implémentation (sans “h”)
//
#include "authentification.h"
#include "utilisateurs.h"
#include <string.h>
#include <stdio.h>

Utilisateur* authentifier(Bibliotheque *bib, const char *email, const char *password) {
    if (!bib || !email || !password) return NULL;
    Utilisateur *u = rechercher_utilisateur_par_email(bib, email);
    if (!u) return NULL;
    return (strcmp(u->password, password) == 0) ? u : NULL;
}

int est_admin(Utilisateur *utilisateur) {
    if (!utilisateur) return 0;
    return (utilisateur->type == ADMIN) ? 1 : 0;
}

void creer_admin_par_defaut(Bibliotheque *bib) {
    if (!bib) return;

    /* Y a-t-il déjà un admin ? */
    for (int i = 0; i < bib->nombre_utilisateurs; ++i) {
        if (bib->utilisateurs[i].type == ADMIN) return;
    }

    const char *nom_base     = "Admin";
    const char *prenom_base  = "Systeme";
    const char *id_base      = "ADMIN-0001";
    const char *email_base   = "admin@bibliotheque.local";
    const char *password_def = "admin123";

    char email[MAX_EMAIL];
    char id_etud[MAX_ID_ETUDIANT];
    strncpy(email, email_base, MAX_EMAIL - 1); email[MAX_EMAIL - 1] = '\0';
    strncpy(id_etud, id_base, MAX_ID_ETUDIANT - 1); id_etud[MAX_ID_ETUDIANT - 1] = '\0';

    int suffix = 1;
    while (rechercher_utilisateur_par_email(bib, email) ||
           rechercher_utilisateur_par_id_etudiant(bib, id_etud)) {
        snprintf(email, MAX_EMAIL, "admin%d@bibliotheque.local", suffix);
        snprintf(id_etud, MAX_ID_ETUDIANT, "ADMIN-%04d", 1 + suffix);
        if (++suffix > 9999) {
            fprintf(stderr, "Erreur : imposible de créer l’admin par défaut.\n");
            return;
        }
    }

    if (ajouter_utilisateur(bib, nom_base, prenom_base, id_etud, email, password_def, ADMIN)) {
        printf("Compte admin par défaut créé : %s / %s\n", email, password_def);
    } else {
        fprintf(stderr, "Erreur : création du compte admin par défaut.\n");
    }
}