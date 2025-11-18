#ifndef EMPRUNTS_H
#define EMPRUNTS_H

#include "structure.h"

/* Fonctions de gestion des emprunts */

/**
 * Enregistre un nouvel emprunt
 * Retourne 1 si succès, 0 si échec
 */
int enregistrer_emprunt(Bibliotheque *bib, int id_livre, int id_utilisateur);

/**
 * Enregistre le retour d'un livre
 * Calcule automatiquement les pénalités si retard
 * Retourne 1 si succès, 0 si échec
 */
int enregistrer_retour(Bibliotheque *bib, int id_livre, int id_utilisateur);

/**
 * Recherche un emprunt actif par livre et utilisateur
 */
Emprunt* rechercher_emprunt_actif(Bibliotheque *bib, int id_livre, int id_utilisateur);

/**
 * Vérifie si un utilisateur peut emprunter (quota non dépassé)
 * Retourne 1 si oui, 0 sinon
 */
int peut_emprunter(Bibliotheque *bib, int id_utilisateur);

/**
 * Vérifie et met à jour les emprunts en retard
 * Calcule les pénalités automatiquement
 */
void verifier_retards(Bibliotheque *bib);

/**
 * Affiche tous les emprunts actifs
 */
void afficher_emprunts_actifs(Bibliotheque *bib);

/**
 * Affiche tous les emprunts (actifs et terminés)
 */
void afficher_tous_les_emprunts(Bibliotheque *bib);

/**
 * Affiche l'historique des emprunts d'un utilisateur
 */
void afficher_historique_utilisateur(Bibliotheque *bib, int id_utilisateur);

/**
 * Affiche les emprunts en retard
 */
void afficher_emprunts_en_retard(Bibliotheque *bib);

/**
 * Prolonge la durée d'un emprunt de X jours
 * Retourne 1 si succès, 0 sinon
 */
int prolonger_emprunt(Bibliotheque *bib, int id_livre, int id_utilisateur, int jours);

#endif /* EMPRUNTS_H */