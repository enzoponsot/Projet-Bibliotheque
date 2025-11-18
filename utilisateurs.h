//
// Gestion des utilisateurs – Header
//

#ifndef UTILISATEURS_H
#define UTILISATEURS_H

#include "structure.h"

/* ==== CRUD ==== */

/**
 * Ajoute un utilisateur.
 * - Vérifie la capacité et l’unicité (ID étudiant, email).
 * Retourne 1 si succès, 0 sinon.
 */
int ajouter_utilisateur(Bibliotheque *bib, const char *nom, const char *prenom,
                        const char *id_etudiant, const char *email,
                        const char *password, TypeUtilisateur type);

/**
 * Supprime un utilisateur par ID.
 * - Refuse si l’utilisateur a des emprunts actifs.
 * Retourne 1 si succès, 0 sinon.
 */
int supprimer_utilisateur(Bibliotheque *bib, int id_utilisateur);

/**
 * Modifie un utilisateur (nom, prénom, id étudiant, email).
 * - Maintient l’unicité de l’ID étudiant et de l’email.
 * Retourne 1 si succès, 0 sinon.
 */
int modifier_utilisateur(Bibliotheque *bib, int id_utilisateur, const char *nom,
                         const char *prenom, const char *id_etudiant, const char *email);

/* ==== Recherche ==== */

Utilisateur* rechercher_utilisateur_par_id(Bibliotheque *bib, int id);
Utilisateur* rechercher_utilisateur_par_id_etudiant(Bibliotheque *bib, const char *id_etudiant);
Utilisateur* rechercher_utilisateur_par_email(Bibliotheque *bib, const char *email);

/* ==== Affichage ==== */

void afficher_utilisateur(const Utilisateur *utilisateur);
void afficher_tous_les_utilisateurs(Bibliotheque *bib);
/* Affichage trié par nom puis prénom (copie + qsort) */
void afficher_utilisateurs_tries(Bibliotheque *bib);

/* ==== Sécurité / Pénalités ==== */

/**
 * Change le mot de passe après vérification de l’ancien.
 * Retourne 1 si succès, 0 sinon.
 */
int modifier_password_utilisateur(Bibliotheque *bib, int id_utilisateur,
                                  const char *ancien_password, const char *nouveau_password);

/** Ajoute une pénalité (montant en €) à l’utilisateur (silencieux si non trouvé). */
void ajouter_penalite(Bibliotheque *bib, int id_utilisateur, double montant);

/**
 * Règle tout ou partie des pénalités de l’utilisateur.
 * Retourne 1 si succès, 0 sinon.
 */
int payer_penalites(Bibliotheque *bib, int id_utilisateur, double montant);

#endif /* UTILISATEURS_H */
