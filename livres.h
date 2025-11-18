//
// Gestion des livres – Header
//

#ifndef LIVRES_H
#define LIVRES_H

#include "structure.h"

/* ==== CRUD ==== */

/**
 * Ajoute un livre dans la bibliothèque.
 * - Vérifie la capacité et l’unicité de l’ISBN.
 * - Initialise le statut à DISPONIBLE et nombre_emprunts à 0.
 * Retourne 1 si succès, 0 sinon.
 */
int ajouter_livre(Bibliotheque *bib,
                  const char *titre, const char *auteur,
                  const char *isbn, const char *categorie, int annee);

/**
 * Supprime un livre par ID.
 * - Refuse si le livre est actuellement emprunté.
 * Retourne 1 si succès, 0 sinon.
 */
int supprimer_livre(Bibliotheque *bib, int id_livre);

/**
 * Modifie un livre par ID.
 * - Maintient l’unicité de l’ISBN (hors livre courant).
 * Retourne 1 si succès, 0 sinon.
 */
int modifier_livre(Bibliotheque *bib, int id_livre,
                   const char *titre, const char *auteur,
                   const char *isbn, const char *categorie, int annee);

/* ==== Recherche ==== */

Livre* rechercher_livre_par_id(Bibliotheque *bib, int id_livre);
Livre* rechercher_livre_par_titre(Bibliotheque *bib, const char *titre);
Livre* rechercher_livre_par_auteur(Bibliotheque *bib, const char *auteur);
Livre* rechercher_livre_par_isbn(Bibliotheque *bib, const char *isbn);

/**
 * Recherche multi-critères (les paramètres NULL sont ignorés).
 * Remplit le tableau de pointeurs resultats[] (jusqu’à max_resultats).
 * Retourne le nombre de résultats écrits.
 */
int rechercher_livres_multi_criteres(Bibliotheque *bib,
                                     const char *titre /* nullable */,
                                     const char *auteur /* nullable */,
                                     const char *categorie /* nullable */,
                                     Livre **resultats, int max_resultats);

/* ==== Affichage ==== */

void afficher_livre(const Livre *livre);
void afficher_tous_les_livres(Bibliotheque *bib);
void afficher_livres_par_categorie(Bibliotheque *bib, const char *categorie);

/* ==== Tri & affichage trié ==== */

void afficher_livres_tries_par_titre(Bibliotheque *bib);
void afficher_livres_tries_par_annee(Bibliotheque *bib);
void afficher_livres_tries_par_auteur(Bibliotheque *bib);
/* tri décroissant par popularité (nombre_emprunts) */
void afficher_livres_tries_par_emprunts(Bibliotheque *bib);

#endif /* LIVRES_H */