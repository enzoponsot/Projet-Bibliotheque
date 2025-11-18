//
// Persistance des données (fichiers .txt) – Implémentation
//
#include "fichiers.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


static void sanitize_for_save(char *s) {
    if (!s) return;
    for (char *p = s; *p; ++p) {
        if (*p == '\t' || *p == '\n' || *p == '\r') *p = ' ';
    }
}
static void trim_eol(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n && (s[n-1] == '\n' || s[n-1] == '\r')) s[--n] = '\0';
}

/* Recalcule le statut & la popularité des livres à partir des emprunts */
static void recalculer_popularite_livres(Bibliotheque *bib) {
    for (int i = 0; i < bib->nombre_livres; ++i) {
        bib->livres[i].nombre_emprunts = 0;
        bib->livres[i].statut = DISPONIBLE;
    }
    for (int i = 0; i < bib->nombre_emprunts; ++i) {
        for (int j = 0; j < bib->nombre_livres; ++j) {
            if (bib->emprunts[i].id_livre == bib->livres[j].id) {
                bib->livres[j].nombre_emprunts++;
                if (bib->emprunts[i].date_retour_effectif == 0) {
                    bib->livres[j].statut = EMPRUNTE;
                }
                break;
            }
        }
    }
}

/* Recalcule le nombre d’emprunts actifs par utilisateur */
static void recalculer_emprunts_actifs_utilisateurs(Bibliotheque *bib) {
    for (int i = 0; i < bib->nombre_utilisateurs; ++i)
        bib->utilisateurs[i].nombre_emprunts_actifs = 0;

    for (int i = 0; i < bib->nombre_emprunts; ++i) {
        if (bib->emprunts[i].date_retour_effectif == 0) {
            for (int u = 0; u < bib->nombre_utilisateurs; ++u) {
                if (bib->utilisateurs[u].id == bib->emprunts[i].id_utilisateur) {
                    bib->utilisateurs[u].nombre_emprunts_actifs++;
                    break;
                }
            }
        }
    }
}

/* --------------------------------
 *  Initialisation mémoire
 * -------------------------------- */
void initialiser_bibliotheque(Bibliotheque *bib) {
    if (!bib) return;
    bib->nombre_livres = 0;
    bib->nombre_utilisateurs = 0;
    bib->nombre_emprunts = 0;
    bib->prochain_id_livre = 1;
    bib->prochain_id_utilisateur = 1;
    bib->prochain_id_emprunt = 1;
}

/* --------------------------------
 *  LIVRES
 * -------------------------------- */
int sauvegarder_livres(Bibliotheque *bib) {
    FILE *f = fopen(FICHIER_LIVRES, "w");
    if (!f) {
        fprintf(stderr, "Erreur: %s (%s)\n", FICHIER_LIVRES, strerror(errno));
        return 0;
    }
    for (int i = 0; i < bib->nombre_livres; ++i) {
        char titre[MAX_TITRE], auteur[MAX_AUTEUR], isbn[MAX_ISBN], cat[MAX_CATEGORIE];
        strncpy(titre,  bib->livres[i].titre,      MAX_TITRE);      titre[MAX_TITRE-1] = '\0';
        strncpy(auteur, bib->livres[i].auteur,     MAX_AUTEUR);     auteur[MAX_AUTEUR-1] = '\0';
        strncpy(isbn,   bib->livres[i].isbn,       MAX_ISBN);       isbn[MAX_ISBN-1] = '\0';
        strncpy(cat,    bib->livres[i].categorie,  MAX_CATEGORIE);  cat[MAX_CATEGORIE-1] = '\0';
        sanitize_for_save(titre); sanitize_for_save(auteur);
        sanitize_for_save(isbn);  sanitize_for_save(cat);

        /* id  titre  auteur  isbn  categorie  annee  statut  nombre_emprunts */
        fprintf(f, "%d\t%s\t%s\t%s\t%s\t%d\t%d\t%d\n",
                bib->livres[i].id, titre, auteur, isbn, cat,
                bib->livres[i].annee, (int)bib->livres[i].statut,
                bib->livres[i].nombre_emprunts);
    }
    fclose(f);
    return 1;
}

int charger_livres(Bibliotheque *bib) {
    FILE *f = fopen(FICHIER_LIVRES, "r");
    if (!f) { /* fichier absent => ok, base vide */
        return 1;
    }
    char line[2048];
    int max_id = 0;
    while (fgets(line, sizeof line, f)) {
        trim_eol(line);
        if (!line[0]) continue;

        Livre lv; memset(&lv, 0, sizeof lv);
        char *rest = line, *tok;
        int field = 0;
        /* id  titre  auteur  isbn  categorie  annee  statut  nombre_emprunts */
        while ((tok = strtok(field == 0 ? rest : NULL, "\t")) != NULL) {
            switch (field) {
                case 0: lv.id = atoi(tok); break;
                case 1: strncpy(lv.titre, tok, MAX_TITRE-1); break;
                case 2: strncpy(lv.auteur, tok, MAX_AUTEUR-1); break;
                case 3: strncpy(lv.isbn, tok, MAX_ISBN-1); break;
                case 4: strncpy(lv.categorie, tok, MAX_CATEGORIE-1); break;
                case 5: lv.annee = atoi(tok); break;
                case 6: lv.statut = (StatutLivre)atoi(tok); break;
                case 7: lv.nombre_emprunts = atoi(tok); break;
                default: break;
            }
            ++field;
        }
        if (field >= 6 && bib->nombre_livres < MAX_LIVRES) {
            bib->livres[bib->nombre_livres++] = lv;
            if (lv.id > max_id) max_id = lv.id;
        }
    }
    fclose(f);
    bib->prochain_id_livre = (max_id >= 1) ? max_id + 1 : bib->prochain_id_livre;
    return 1;
}

/* --------------------------------
 *  UTILISATEURS
 * -------------------------------- */
int sauvegarder_utilisateurs(Bibliotheque *bib) {
    FILE *f = fopen(FICHIER_UTILISATEURS, "w");
    if (!f) { fprintf(stderr, "Erreur: %s (%s)\n", FICHIER_UTILISATEURS, strerror(errno)); return 0; }

    for (int i = 0; i < bib->nombre_utilisateurs; ++i) {
        char nom[MAX_NOM], prenom[MAX_PRENOM], idet[MAX_ID_ETUDIANT], email[MAX_EMAIL], pass[MAX_PASSWORD];
        strncpy(nom,    bib->utilisateurs[i].nom,         MAX_NOM);        nom[MAX_NOM-1] = '\0';
        strncpy(prenom, bib->utilisateurs[i].prenom,      MAX_PRENOM);     prenom[MAX_PRENOM-1] = '\0';
        strncpy(idet,   bib->utilisateurs[i].id_etudiant, MAX_ID_ETUDIANT);idet[MAX_ID_ETUDIANT-1] = '\0';
        strncpy(email,  bib->utilisateurs[i].email,       MAX_EMAIL);      email[MAX_EMAIL-1] = '\0';
        strncpy(pass,   bib->utilisateurs[i].password,    MAX_PASSWORD);   pass[MAX_PASSWORD-1] = '\0';
        sanitize_for_save(nom); sanitize_for_save(prenom);
        sanitize_for_save(idet); sanitize_for_save(email); sanitize_for_save(pass);

        /* id nom prenom id_etudiant email password type nb_actifs penalites */
        fprintf(f, "%d\t%s\t%s\t%s\t%s\t%s\t%d\t%d\t%.2f\n",
                bib->utilisateurs[i].id, nom, prenom, idet, email, pass,
                (int)bib->utilisateurs[i].type,
                bib->utilisateurs[i].nombre_emprunts_actifs,
                bib->utilisateurs[i].penalites);
    }
    fclose(f);
    return 1;
}

int charger_utilisateurs(Bibliotheque *bib) {
    FILE *f = fopen(FICHIER_UTILISATEURS, "r");
    if (!f) { return 1; } /* vide/absent => ok */

    char line[2048];
    int max_id = 0;
    while (fgets(line, sizeof line, f)) {
        trim_eol(line);
        if (!line[0]) continue;

        Utilisateur u; memset(&u, 0, sizeof u);
        char *rest = line, *tok; int field = 0;
        while ((tok = strtok(field == 0 ? rest : NULL, "\t")) != NULL) {
            switch (field) {
                case 0: u.id = atoi(tok); break;
                case 1: strncpy(u.nom, tok, MAX_NOM-1); break;
                case 2: strncpy(u.prenom, tok, MAX_PRENOM-1); break;
                case 3: strncpy(u.id_etudiant, tok, MAX_ID_ETUDIANT-1); break;
                case 4: strncpy(u.email, tok, MAX_EMAIL-1); break;
                case 5: strncpy(u.password, tok, MAX_PASSWORD-1); break;
                case 6: u.type = (TypeUtilisateur)atoi(tok); break;
                case 7: u.nombre_emprunts_actifs = atoi(tok); break;
                case 8: u.penalites = atof(tok); break;
                default: break;
            }
            ++field;
        }
        if (field >= 7 && bib->nombre_utilisateurs < MAX_UTILISATEURS) {
            bib->utilisateurs[bib->nombre_utilisateurs++] = u;
            if (u.id > max_id) max_id = u.id;
        }
    }
    fclose(f);
    bib->prochain_id_utilisateur = (max_id >= 1) ? max_id + 1 : bib->prochain_id_utilisateur;
    return 1;
}

/* --------------------------------
 *  EMPRUNTS
 * -------------------------------- */
int sauvegarder_emprunts(Bibliotheque *bib) {
    FILE *f = fopen(FICHIER_EMPRUNTS, "w");
    if (!f) { fprintf(stderr, "Erreur: %s (%s)\n", FICHIER_EMPRUNTS, strerror(errno)); return 0; }

    for (int i = 0; i < bib->nombre_emprunts; ++i) {
        /* id id_livre id_utilisateur date_emprunt date_retour_prevue date_retour_effectif est_en_retard penalite */
        fprintf(f, "%d\t%d\t%d\t%lld\t%lld\t%lld\t%d\t%.2f\n",
                bib->emprunts[i].id,
                bib->emprunts[i].id_livre,
                bib->emprunts[i].id_utilisateur,
                (long long)bib->emprunts[i].date_emprunt,
                (long long)bib->emprunts[i].date_retour_prevue,
                (long long)bib->emprunts[i].date_retour_effectif,
                bib->emprunts[i].est_en_retard,
                bib->emprunts[i].penalite);
    }
    fclose(f);
    return 1;
}

int charger_emprunts(Bibliotheque *bib) {
    FILE *f = fopen(FICHIER_EMPRUNTS, "r");
    if (!f) { return 1; } /* vide/absent => ok */

    char line[2048];
    int max_id = 0;
    while (fgets(line, sizeof line, f)) {
        trim_eol(line);
        if (!line[0]) continue;

        Emprunt e; memset(&e, 0, sizeof e);
        char *rest = line, *tok; int field = 0;
        while ((tok = strtok(field == 0 ? rest : NULL, "\t")) != NULL) {
            switch (field) {
                case 0: e.id = atoi(tok); break;
                case 1: e.id_livre = atoi(tok); break;
                case 2: e.id_utilisateur = atoi(tok); break;
                case 3: e.date_emprunt = (time_t)strtoll(tok, NULL, 10); break;
                case 4: e.date_retour_prevue = (time_t)strtoll(tok, NULL, 10); break;
                case 5: e.date_retour_effectif = (time_t)strtoll(tok, NULL, 10); break;
                case 6: e.est_en_retard = atoi(tok); break;
                case 7: e.penalite = atof(tok); break;
                default: break;
            }
            ++field;
        }
        if (field >= 5 && bib->nombre_emprunts < MAX_EMPRUNTS) {
            bib->emprunts[bib->nombre_emprunts++] = e;
            if (e.id > max_id) max_id = e.id;
        }
    }
    fclose(f);
    bib->prochain_id_emprunt = (max_id >= 1) ? max_id + 1 : bib->prochain_id_emprunt;
    return 1;
}

/* --------------------------------
 *  CONFIG (prochains IDs)
 *  Format: prochain_id_livre \t prochain_id_utilisateur \t prochain_id_emprunt
 * -------------------------------- */
int sauvegarder_config(Bibliotheque *bib) {
    FILE *f = fopen(FICHIER_CONFIG, "w");
    if (!f) { fprintf(stderr, "Erreur: %s (%s)\n", FICHIER_CONFIG, strerror(errno)); return 0; }
    fprintf(f, "%d\t%d\t%d\n", bib->prochain_id_livre, bib->prochain_id_utilisateur, bib->prochain_id_emprunt);
    fclose(f);
    return 1;
}

int charger_config(Bibliotheque *bib) {
    FILE *f = fopen(FICHIER_CONFIG, "r");
    if (!f) return 1; /* optionnel */

    int pidL=0, pidU=0, pidE=0;
    int read = fscanf(f, "%d\t%d\t%d", &pidL, &pidU, &pidE);
    fclose(f);

    if (read == 3) {
        if (pidL >= bib->prochain_id_livre)        bib->prochain_id_livre = pidL;
        if (pidU >= bib->prochain_id_utilisateur)  bib->prochain_id_utilisateur = pidU;
        if (pidE >= bib->prochain_id_emprunt)      bib->prochain_id_emprunt = pidE;
    }
    return 1;
}

/* --------------------------------
 *  TOUT-EN-UN
 * -------------------------------- */
int charger_tout(Bibliotheque *bib) {
    if (!bib) return 0;
    initialiser_bibliotheque(bib);

    if (!charger_livres(bib))        return 0;
    if (!charger_utilisateurs(bib))  return 0;
    if (!charger_emprunts(bib))      return 0;

    /* Cohérences dérivées */
    recalculer_popularite_livres(bib);
    recalculer_emprunts_actifs_utilisateurs(bib);

    /* Optionnel : surcharge depuis config si plus grand */
    charger_config(bib);

    return 1;
}

int sauvegarder_tout(Bibliotheque *bib) {
    if (!bib) return 0;

    if (!sauvegarder_livres(bib))        return 0;
    if (!sauvegarder_utilisateurs(bib))  return 0;
    if (!sauvegarder_emprunts(bib))      return 0;
    if (!sauvegarder_config(bib))        return 0;

    return 1;
}