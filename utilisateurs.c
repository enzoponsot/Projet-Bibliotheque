#include "utilisateurs.h"
#include <stdio.h>
#include <string.h>
#include "structure.h"
#include <stdlib.h>

/* Fonction utilitaire pour comparer les noms (tri) */
static int comparer_noms(const void *a, const void *b) {
    const Utilisateur *user1 = (const Utilisateur *)a;
    const Utilisateur *user2 = (const Utilisateur *)b;
    int cmp = strcmp(user1->nom, user2->nom);
    if (cmp == 0) {
        return strcmp(user1->prenom, user2->prenom);
    }
    return cmp;
}

int ajouter_utilisateur(Bibliotheque *bib, const char *nom, const char *prenom,
                        const char *id_etudiant, const char *email,
                        const char *password, TypeUtilisateur type) {
    if (bib->nombre_utilisateurs >= MAX_UTILISATEURS) {
        printf("Erreur : La base d'utilisateurs est pleine.\n");
        return 0;
    }

    /* Vérifier si l'ID étudiant existe déjà */
    if (rechercher_utilisateur_par_id_etudiant(bib, id_etudiant) != NULL) {
        printf("Erreur : Un utilisateur avec cet ID étudiant existe déjà.\n");
        return 0;
    }

    /* Vérifier si l'email existe déjà */
    if (rechercher_utilisateur_par_email(bib, email) != NULL) {
        printf("Erreur : Un utilisateur avec cet email existe déjà.\n");
        return 0;
    }

    Utilisateur *nouvel_utilisateur = &bib->utilisateurs[bib->nombre_utilisateurs];
    nouvel_utilisateur->id = bib->prochain_id_utilisateur++;
    strncpy(nouvel_utilisateur->nom, nom, MAX_NOM - 1);
    nouvel_utilisateur->nom[MAX_NOM - 1] = '\0';
    strncpy(nouvel_utilisateur->prenom, prenom, MAX_PRENOM - 1);
    nouvel_utilisateur->prenom[MAX_PRENOM - 1] = '\0';
    strncpy(nouvel_utilisateur->id_etudiant, id_etudiant, MAX_ID_ETUDIANT - 1);
    nouvel_utilisateur->id_etudiant[MAX_ID_ETUDIANT - 1] = '\0';
    strncpy(nouvel_utilisateur->email, email, MAX_EMAIL - 1);
    nouvel_utilisateur->email[MAX_EMAIL - 1] = '\0';
    strncpy(nouvel_utilisateur->password, password, MAX_PASSWORD - 1);
    nouvel_utilisateur->password[MAX_PASSWORD - 1] = '\0';
    nouvel_utilisateur->type = type;
    nouvel_utilisateur->nombre_emprunts_actifs = 0;
    nouvel_utilisateur->penalites = 0.0;

    bib->nombre_utilisateurs++;
    printf("Utilisateur ajouté avec succès (ID: %d).\n", nouvel_utilisateur->id);
    return 1;
}

int supprimer_utilisateur(Bibliotheque *bib, int id_utilisateur) {
    int i;
    for (i = 0; i < bib->nombre_utilisateurs; i++) {
        if (bib->utilisateurs[i].id == id_utilisateur) {
            /* Vérifier si l'utilisateur a des emprunts actifs */
            if (bib->utilisateurs[i].nombre_emprunts_actifs > 0) {
                printf("Erreur : L'utilisateur a des emprunts en cours.\n");
                return 0;
            }

            /* Décaler tous les utilisateurs suivants */
            int j;
            for (j = i; j < bib->nombre_utilisateurs - 1; j++) {
                bib->utilisateurs[j] = bib->utilisateurs[j + 1];
            }
            bib->nombre_utilisateurs--;
            printf("Utilisateur supprimé avec succès.\n");
            return 1;
        }
    }
    printf("Erreur : Utilisateur non trouvé.\n");
    return 0;
}

int modifier_utilisateur(Bibliotheque *bib, int id_utilisateur, const char *nom,
                         const char *prenom, const char *id_etudiant, const char *email) {
    Utilisateur *utilisateur = rechercher_utilisateur_par_id(bib, id_utilisateur);
    if (utilisateur == NULL) {
        printf("Erreur : Utilisateur non trouvé.\n");
        return 0;
    }

    /* Vérifier si le nouvel ID étudiant n'est pas déjà utilisé */
    if (strcmp(utilisateur->id_etudiant, id_etudiant) != 0) {
        Utilisateur *user_id = rechercher_utilisateur_par_id_etudiant(bib, id_etudiant);
        if (user_id != NULL && user_id->id != id_utilisateur) {
            printf("Erreur : Cet ID étudiant est déjà utilisé.\n");
            return 0;
        }
    }

    /* Vérifier si le nouvel email n'est pas déjà utilisé */
    if (strcmp(utilisateur->email, email) != 0) {
        Utilisateur *user_email = rechercher_utilisateur_par_email(bib, email);
        if (user_email != NULL && user_email->id != id_utilisateur) {
            printf("Erreur : Cet email est déjà utilisé.\n");
            return 0;
        }
    }

    strncpy(utilisateur->nom, nom, MAX_NOM - 1);
    utilisateur->nom[MAX_NOM - 1] = '\0';
    strncpy(utilisateur->prenom, prenom, MAX_PRENOM - 1);
    utilisateur->prenom[MAX_PRENOM - 1] = '\0';
    strncpy(utilisateur->id_etudiant, id_etudiant, MAX_ID_ETUDIANT - 1);
    utilisateur->id_etudiant[MAX_ID_ETUDIANT - 1] = '\0';
    strncpy(utilisateur->email, email, MAX_EMAIL - 1);
    utilisateur->email[MAX_EMAIL - 1] = '\0';

    printf("Utilisateur modifié avec succès.\n");
    return 1;
}

Utilisateur* rechercher_utilisateur_par_id(Bibliotheque *bib, int id) {
    int i;
    for (i = 0; i < bib->nombre_utilisateurs; i++) {
        if (bib->utilisateurs[i].id == id) {
            return &bib->utilisateurs[i];
        }
    }
    return NULL;
}

Utilisateur* rechercher_utilisateur_par_id_etudiant(Bibliotheque *bib, const char *id_etudiant) {
    int i;
    for (i = 0; i < bib->nombre_utilisateurs; i++) {
        if (strcmp(bib->utilisateurs[i].id_etudiant, id_etudiant) == 0) {
            return &bib->utilisateurs[i];
        }
    }
    return NULL;
}

Utilisateur* rechercher_utilisateur_par_email(Bibliotheque *bib, const char *email) {
    int i;
    for (i = 0; i < bib->nombre_utilisateurs; i++) {
        if (strcmp(bib->utilisateurs[i].email, email) == 0) {
            return &bib->utilisateurs[i];
        }
    }
    return NULL;
}

void afficher_utilisateur(const Utilisateur *utilisateur) {
    printf("----------------------------------------\n");
    printf("ID: %d\n", utilisateur->id);
    printf("Nom: %s %s\n", utilisateur->prenom, utilisateur->nom);
    printf("ID Étudiant: %s\n", utilisateur->id_etudiant);
    printf("Email: %s\n", utilisateur->email);
    printf("Type: %s\n", utilisateur->type == ADMIN ? "Administrateur" : "Utilisateur");
    printf("Emprunts actifs: %d/%d\n", utilisateur->nombre_emprunts_actifs, MAX_EMPRUNTS_PAR_USER);
    printf("Pénalités: %.2f €\n", utilisateur->penalites);
}

void afficher_tous_les_utilisateurs(Bibliotheque *bib) {
    int i;
    if (bib->nombre_utilisateurs == 0) {
        printf("Aucun utilisateur enregistré.\n");
        return;
    }

    printf("\n=== Liste de tous les utilisateurs (%d) ===\n", bib->nombre_utilisateurs);
    for (i = 0; i < bib->nombre_utilisateurs; i++) {
        afficher_utilisateur(&bib->utilisateurs[i]);
    }
}

void afficher_utilisateurs_tries(Bibliotheque *bib) {
    if (bib->nombre_utilisateurs == 0) {
        printf("Aucun utilisateur enregistré.\n");
        return;
    }

    /* Créer une copie pour le tri */
    Utilisateur copie[MAX_UTILISATEURS];
    int i;
    for (i = 0; i < bib->nombre_utilisateurs; i++) {
        copie[i] = bib->utilisateurs[i];
    }

    /* Tri par nom */
    qsort(copie, bib->nombre_utilisateurs, sizeof(Utilisateur), comparer_noms);

    printf("\n=== Utilisateurs triés par nom ===\n");
    for (i = 0; i < bib->nombre_utilisateurs; i++) {
        afficher_utilisateur(&copie[i]);
    }
}

int modifier_password_utilisateur(Bibliotheque *bib, int id_utilisateur,
                                  const char *ancien_password, const char *nouveau_password) {
    Utilisateur *utilisateur = rechercher_utilisateur_par_id(bib, id_utilisateur);
    if (utilisateur == NULL) {
        printf("Erreur : Utilisateur non trouvé.\n");
        return 0;
    }

    if (strcmp(utilisateur->password, ancien_password) != 0) {
        printf("Erreur : Ancien mot de passe incorrect.\n");
        return 0;
    }

    strncpy(utilisateur->password, nouveau_password, MAX_PASSWORD - 1);
    utilisateur->password[MAX_PASSWORD - 1] = '\0';
    printf("Mot de passe modifié avec succès.\n");
    return 1;
}

void ajouter_penalite(Bibliotheque *bib, int id_utilisateur, double montant) {
    Utilisateur *utilisateur = rechercher_utilisateur_par_id(bib, id_utilisateur);
    if (utilisateur != NULL) {
        utilisateur->penalites += montant;
    }
}

int payer_penalites(Bibliotheque *bib, int id_utilisateur, double montant) {
    Utilisateur *utilisateur = rechercher_utilisateur_par_id(bib, id_utilisateur);
    if (utilisateur == NULL) {
        printf("Erreur : Utilisateur non trouvé.\n");
        return 0;
    }

    if (montant > utilisateur->penalites) {
        printf("Le montant est supérieur aux pénalités dues (%.2f €).\n", utilisateur->penalites);
        utilisateur->penalites = 0.0;
    } else {
        utilisateur->penalites -= montant;
    }

    printf("Paiement effectué. Pénalités restantes : %.2f €\n", utilisateur->penalites);
    return 1;
}