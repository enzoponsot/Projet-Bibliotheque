//
// Gestion des livres – Implémentation
//

#include "livres.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* =========================
 *  Helpers (comparateurs)
 * ========================= */

static int cmp_titre(const void *a, const void *b) {
    const Livre *l1 = (const Livre *)a;
    const Livre *l2 = (const Livre *)b;
    int c = strcmp(l1->titre, l2->titre);
    if (c == 0) return strcmp(l1->auteur, l2->auteur);
    return c;
}

static int cmp_annee(const void *a, const void *b) {
    const Livre *l1 = (const Livre *)a;
    const Livre *l2 = (const Livre *)b;
    if (l1->annee != l2->annee) return l1->annee - l2->annee;  /* croissant */
    return strcmp(l1->titre, l2->titre);
}

static int cmp_auteur(const void *a, const void *b) {
    const Livre *l1 = (const Livre *)a;
    const Livre *l2 = (const Livre *)b;
    int c = strcmp(l1->auteur, l2->auteur);
    if (c == 0) return strcmp(l1->titre, l2->titre);
    return c;
}

static int cmp_emprunts_desc(const void *a, const void *b) {
    const Livre *l1 = (const Livre *)a;
    const Livre *l2 = (const Livre *)b;
    return (l2->nombre_emprunts - l1->nombre_emprunts); /* décroissant */
}

/* =========================
 *     Implémentations
 * ========================= */

int ajouter_livre(Bibliotheque *bib,
                  const char *titre, const char *auteur,
                  const char *isbn, const char *categorie, int annee) {
    if (bib->nombre_livres >= MAX_LIVRES) {
        printf("Erreur : La base de livres est pleine.\n");
        return 0;
    }

    /* Unicité ISBN */
    if (rechercher_livre_par_isbn(bib, isbn) != NULL) {
        printf("Erreur : Un livre avec cet ISBN existe déjà.\n");
        return 0;
    }

    Livre *nv = &bib->livres[bib->nombre_livres];
    nv->id = bib->prochain_id_livre++;
    strncpy(nv->titre, titre, MAX_TITRE - 1);     nv->titre[MAX_TITRE - 1] = '\0';
    strncpy(nv->auteur, auteur, MAX_AUTEUR - 1);  nv->auteur[MAX_AUTEUR - 1] = '\0';
    strncpy(nv->isbn, isbn, MAX_ISBN - 1);        nv->isbn[MAX_ISBN - 1] = '\0';
    strncpy(nv->categorie, categorie, MAX_CATEGORIE - 1); nv->categorie[MAX_CATEGORIE - 1] = '\0';
    nv->annee = annee;
    nv->statut = DISPONIBLE;
    nv->nombre_emprunts = 0;

    bib->nombre_livres++;
    printf("Livre ajouté avec succès (ID: %d).\n", nv->id);
    return 1;
}

int supprimer_livre(Bibliotheque *bib, int id_livre) {
    int i, pos = -1;

    for (i = 0; i < bib->nombre_livres; i++) {
        if (bib->livres[i].id == id_livre) {
            pos = i;
            break;
        }
    }
    if (pos == -1) {
        printf("Erreur : Livre non trouvé.\n");
        return 0;
    }

    if (bib->livres[pos].statut == EMPRUNTE) {
        printf("Erreur : Impossible de supprimer un livre actuellement emprunté.\n");
        return 0;
    }

    /* décalage */
    for (i = pos; i < bib->nombre_livres - 1; i++) {
        bib->livres[i] = bib->livres[i + 1];
    }
    bib->nombre_livres--;
    printf("Livre supprimé avec succès.\n");
    return 1;
}

int modifier_livre(Bibliotheque *bib, int id_livre,
                   const char *titre, const char *auteur,
                   const char *isbn, const char *categorie, int annee) {
    Livre *livre = rechercher_livre_par_id(bib, id_livre);
    if (livre == NULL) {
        printf("Erreur : Livre non trouvé.\n");
        return 0;
    }

    /* Unicité ISBN si changé */
    if (strcmp(livre->isbn, isbn) != 0) {
        Livre *existant = rechercher_livre_par_isbn(bib, isbn);
        if (existant != NULL && existant->id != id_livre) {
            printf("Erreur : Cet ISBN est déjà utilisé.\n");
            return 0;
        }
    }

    strncpy(livre->titre, titre, MAX_TITRE - 1);       livre->titre[MAX_TITRE - 1] = '\0';
    strncpy(livre->auteur, auteur, MAX_AUTEUR - 1);    livre->auteur[MAX_AUTEUR - 1] = '\0';
    strncpy(livre->isbn, isbn, MAX_ISBN - 1);          livre->isbn[MAX_ISBN - 1] = '\0';
    strncpy(livre->categorie, categorie, MAX_CATEGORIE - 1); livre->categorie[MAX_CATEGORIE - 1] = '\0';
    livre->annee = annee;

    printf("Livre modifié avec succès.\n");
    return 1;
}

/* ==== Recherche ==== */

Livre* rechercher_livre_par_id(Bibliotheque *bib, int id_livre) {
    int i;
    for (i = 0; i < bib->nombre_livres; i++) {
        if (bib->livres[i].id == id_livre) {
            return &bib->livres[i];
        }
    }
    return NULL;
}

Livre* rechercher_livre_par_titre(Bibliotheque *bib, const char *titre) {
    int i;
    for (i = 0; i < bib->nombre_livres; i++) {
        if (strcmp(bib->livres[i].titre, titre) == 0) {
            return &bib->livres[i];
        }
    }
    return NULL;
}

Livre* rechercher_livre_par_auteur(Bibliotheque *bib, const char *auteur) {
    int i;
    for (i = 0; i < bib->nombre_livres; i++) {
        if (strcmp(bib->livres[i].auteur, auteur) == 0) {
            return &bib->livres[i];
        }
    }
    return NULL;
}

Livre* rechercher_livre_par_isbn(Bibliotheque *bib, const char *isbn) {
    int i;
    for (i = 0; i < bib->nombre_livres; i++) {
        if (strcmp(bib->livres[i].isbn, isbn) == 0) {
            return &bib->livres[i];
        }
    }
    return NULL;
}

int rechercher_livres_multi_criteres(Bibliotheque *bib,
                                     const char *titre,
                                     const char *auteur,
                                     const char *categorie,
                                     Livre **resultats, int max_resultats) {
    int i, count = 0;
    for (i = 0; i < bib->nombre_livres && count < max_resultats; i++) {
        int ok = 1;
        if (titre && strcmp(bib->livres[i].titre, titre) != 0) ok = 0;
        if (auteur && strcmp(bib->livres[i].auteur, auteur) != 0) ok = 0;
        if (categorie && strcmp(bib->livres[i].categorie, categorie) != 0) ok = 0;

        if (ok) {
            resultats[count++] = &bib->livres[i];
        }
    }
    return count;
}

/* ==== Affichages ==== */

void afficher_livre(const Livre *livre) {
    if (!livre) return;
    printf("----------------------------------------\n");
    printf("ID: %d\n", livre->id);
    printf("Titre: %s\n", livre->titre);
    printf("Auteur: %s\n", livre->auteur);
    printf("ISBN: %s\n", livre->isbn);
    printf("Catégorie: %s\n", livre->categorie);
    printf("Année: %d\n", livre->annee);
    printf("Statut: %s\n", livre->statut == DISPONIBLE ? "Disponible" : "Emprunté");
    printf("Nombre d'emprunts: %d\n", livre->nombre_emprunts);
}

void afficher_tous_les_livres(Bibliotheque *bib) {
    int i;
    if (bib->nombre_livres == 0) {
        printf("Aucun livre dans la bibliothèque.\n");
        return;
    }
    printf("\n=== Liste de tous les livres (%d) ===\n", bib->nombre_livres);
    for (i = 0; i < bib->nombre_livres; i++) {
        afficher_livre(&bib->livres[i]);
    }
}

void afficher_livres_par_categorie(Bibliotheque *bib, const char *categorie) {
    int i, count = 0;
    printf("\n=== Livres de la catégorie: %s ===\n", categorie);
    for (i = 0; i < bib->nombre_livres; i++) {
        if (strcmp(bib->livres[i].categorie, categorie) == 0) {
            afficher_livre(&bib->livres[i]);
            count++;
        }
    }
    if (count == 0) {
        printf("Aucun livre trouvé dans cette catégorie.\n");
    }
}

/* ==== Affichages triés (copies locales + qsort) ==== */

static void afficher_copie_triee(Bibliotheque *bib,
                                 int (*cmp)(const void*, const void*)) {
    if (bib->nombre_livres == 0) {
        printf("Aucun livre dans la bibliothèque.\n");
        return;
    }
    Livre *copie = (Livre *)malloc(sizeof(Livre) * bib->nombre_livres);
    if (!copie) {
        printf("Erreur : mémoire insuffisante.\n");
        return;
    }
    for (int i = 0; i < bib->nombre_livres; i++) copie[i] = bib->livres[i];
    qsort(copie, bib->nombre_livres, sizeof(Livre), cmp);

    for (int i = 0; i < bib->nombre_livres; i++) {
        afficher_livre(&copie[i]);
    }
    free(copie);
}

void afficher_livres_tries_par_titre(Bibliotheque *bib) {
    printf("\n=== Livres triés par titre ===\n");
    afficher_copie_triee(bib, cmp_titre);
}

void afficher_livres_tries_par_annee(Bibliotheque *bib) {
    printf("\n=== Livres triés par année ===\n");
    afficher_copie_triee(bib, cmp_annee);
}

void afficher_livres_tries_par_auteur(Bibliotheque *bib) {
    printf("\n=== Livres triés par auteur ===\n");
    afficher_copie_triee(bib, cmp_auteur);
}

void afficher_livres_tries_par_emprunts(Bibliotheque *bib) {
    printf("\n=== Livres triés par popularité (nombre d'emprunts) ===\n");
    afficher_copie_triee(bib, cmp_emprunts_desc);
}