#ifndef FICHIERS_H
#define FICHIERS_H

#include "structure.h"

/* Noms des fichiers de sauvegarde */
#define FICHIER_LIVRES        "livres.txt"
#define FICHIER_UTILISATEURS  "utilisateurs.txt"
#define FICHIER_EMPRUNTS      "emprunts.txt"
#define FICHIER_CONFIG        "config.txt"

/* --------- API par catégorie --------- */
int sauvegarder_livres(Bibliotheque *bib);
int charger_livres(Bibliotheque *bib);

int sauvegarder_utilisateurs(Bibliotheque *bib);
int charger_utilisateurs(Bibliotheque *bib);

int sauvegarder_emprunts(Bibliotheque *bib);
int charger_emprunts(Bibliotheque *bib);

/* --------- Prochains IDs --------- */
int sauvegarder_config(Bibliotheque *bib);
int charger_config(Bibliotheque *bib);

/* --------- Tout-en-un --------- */
int sauvegarder_tout(Bibliotheque *bib);
int charger_tout(Bibliotheque *bib);

/* --------- Init mémoire --------- */
void initialiser_bibliotheque(Bibliotheque *bib);

#endif /* FICHIERS_H */