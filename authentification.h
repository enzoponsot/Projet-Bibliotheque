#ifndef AUTHENTIFICATION_H
#define AUTHENTIFICATION_H

#include "structure.h"

/**
 * Authentifie un utilisateur avec son email et mot de passe
 * Retourne le pointeur vers l'utilisateur si succès, NULL sinon
 */
Utilisateur* authentifier(Bibliotheque *bib, const char *email, const char *password);

/**
 * Vérifie si un utilisateur a les droits d'administrateur
 * Retourne 1 si admin, 0 sinon
 */
int est_admin(Utilisateur *utilisateur);

/**
 * Crée le compte administrateur par défaut si aucun admin n'existe
 */
void creer_admin_par_defaut(Bibliotheque *bib);

#endif /* AUTHENTIFICATION_H */