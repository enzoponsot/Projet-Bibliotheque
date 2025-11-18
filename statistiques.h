#ifndef STATISTIQUES_H
#define STATISTIQUES_H

#include "structure.h"

/**
 * Affiche le top N des livres les plus empruntés
 */
void afficher_top_livres(Bibliotheque *bib, int n);

/**
 * Affiche le top N des utilisateurs les plus actifs
 */
void afficher_top_utilisateurs(Bibliotheque *bib, int n);

/**
 * Affiche les statistiques générales de la bibliothèque
 */
void afficher_statistiques_generales(Bibliotheque *bib);

/**
 * Affiche les statistiques par catégorie
 */
void afficher_statistiques_categories(Bibliotheque *bib);

/**
 * Génère un rapport complet au format texte
 */
void generer_rapport_texte(Bibliotheque *bib, const char *nom_fichier);

/**
 * Génère un rapport au format HTML
 */
void generer_rapport_html(Bibliotheque *bib, const char *nom_fichier);

#endif /* STATISTIQUES_H */