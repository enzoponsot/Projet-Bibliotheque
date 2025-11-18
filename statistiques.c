#include "statistiques.h"
#include "livres.h"
#include "utilisateurs.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

/* Structure pour compter les emprunts par utilisateur */
typedef struct {
    int id_utilisateur;
    int nombre_emprunts;
} StatUtilisateur;

/* Fonction de comparaison pour trier les statistiques utilisateurs */
static int comparer_stat_utilisateurs(const void *a, const void *b) {
    const StatUtilisateur *stat1 = (const StatUtilisateur *)a;
    const StatUtilisateur *stat2 = (const StatUtilisateur *)b;
    return stat2->nombre_emprunts - stat1->nombre_emprunts;
}

void afficher_top_livres(Bibliotheque *bib, int n) {
    int i;
    Livre copie[MAX_LIVRES];
    int max = (n < bib->nombre_livres) ? n : bib->nombre_livres;

    if (bib->nombre_livres == 0) {
        printf("Aucun livre dans la bibliothèque.\n");
        return;
    }

    /* Copier et trier les livres par nombre d'emprunts */
    for (i = 0; i < bib->nombre_livres; i++) {
        copie[i] = bib->livres[i];
    }

    /* Tri par nombre d'emprunts décroissant */
    for (i = 0; i < bib->nombre_livres - 1; i++) {
        int j;
        for (j = i + 1; j < bib->nombre_livres; j++) {
            if (copie[j].nombre_emprunts > copie[i].nombre_emprunts) {
                Livre temp = copie[i];
                copie[i] = copie[j];
                copie[j] = temp;
            }
        }
    }

    printf("\n=== Top %d des livres les plus empruntés ===\n", max);
    for (i = 0; i < max; i++) {
        printf("%d. %s - %s (%d emprunts)\n",
               i + 1,
               copie[i].titre,
               copie[i].auteur,
               copie[i].nombre_emprunts);
    }
}

void afficher_top_utilisateurs(Bibliotheque *bib, int n) {
    int i, j;
    StatUtilisateur stats[MAX_UTILISATEURS];
    int max = (n < bib->nombre_utilisateurs) ? n : bib->nombre_utilisateurs;

    if (bib->nombre_utilisateurs == 0) {
        printf("Aucun utilisateur enregistré.\n");
        return;
    }

    /* Compter les emprunts par utilisateur */
    for (i = 0; i < bib->nombre_utilisateurs; i++) {
        stats[i].id_utilisateur = bib->utilisateurs[i].id;
        stats[i].nombre_emprunts = 0;

        for (j = 0; j < bib->nombre_emprunts; j++) {
            if (bib->emprunts[j].id_utilisateur == bib->utilisateurs[i].id) {
                stats[i].nombre_emprunts++;
            }
        }
    }

    /* Trier les statistiques */
    qsort(stats, bib->nombre_utilisateurs, sizeof(StatUtilisateur), comparer_stat_utilisateurs);

    printf("\n=== Top %d des utilisateurs les plus actifs ===\n", max);
    for (i = 0; i < max; i++) {
        Utilisateur *user = rechercher_utilisateur_par_id(bib, stats[i].id_utilisateur);
        if (user != NULL) {
            printf("%d. %s %s (%d emprunts)\n",
                   i + 1,
                   user->prenom,
                   user->nom,
                   stats[i].nombre_emprunts);
        }
    }
}

void afficher_statistiques_generales(Bibliotheque *bib) {
    int i;
    int livres_disponibles = 0;
    int livres_empruntes = 0;
    int emprunts_actifs = 0;
    int emprunts_en_retard = 0;
    double total_penalites = 0.0;
    time_t maintenant = time(NULL);

    /* Calculer les statistiques sur les livres */
    for (i = 0; i < bib->nombre_livres; i++) {
        if (bib->livres[i].statut == DISPONIBLE) {
            livres_disponibles++;
        } else {
            livres_empruntes++;
        }
    }

    /* Calculer les statistiques sur les emprunts */
    for (i = 0; i < bib->nombre_emprunts; i++) {
        if (bib->emprunts[i].date_retour_effectif == 0) {
            emprunts_actifs++;
            if (bib->emprunts[i].est_en_retard) {
                emprunts_en_retard++;
            }
        }
    }

    /* Calculer le total des pénalités */
    for (i = 0; i < bib->nombre_utilisateurs; i++) {
        total_penalites += bib->utilisateurs[i].penalites;
    }

    printf("\n========================================\n");
    printf("   STATISTIQUES GÉNÉRALES\n");
    printf("========================================\n");
    printf("Date: %s", ctime(&maintenant));
    printf("\n--- Livres ---\n");
    printf("Total de livres: %d\n", bib->nombre_livres);
    printf("Livres disponibles: %d\n", livres_disponibles);
    printf("Livres empruntés: %d\n", livres_empruntes);

    printf("\n--- Utilisateurs ---\n");
    printf("Total d'utilisateurs: %d\n", bib->nombre_utilisateurs);

    printf("\n--- Emprunts ---\n");
    printf("Total d'emprunts (historique): %d\n", bib->nombre_emprunts);
    printf("Emprunts actifs: %d\n", emprunts_actifs);
    printf("Emprunts en retard: %d\n", emprunts_en_retard);

    printf("\n--- Financier ---\n");
    printf("Total des pénalités à percevoir: %.2f €\n", total_penalites);

    if (bib->nombre_livres > 0) {
        double taux_utilisation = (double)livres_empruntes / bib->nombre_livres * 100;
        printf("\nTaux d'utilisation: %.1f%%\n", taux_utilisation);
    }

    printf("========================================\n");
}

void afficher_statistiques_categories(Bibliotheque *bib) {
    int i, j;
    char categories[100][MAX_CATEGORIE];
    int count_categories[100];
    int nb_categories = 0;
    int trouve;

    if (bib->nombre_livres == 0) {
        printf("Aucun livre dans la bibliothèque.\n");
        return;
    }

    /* Compter les livres par catégorie */
    for (i = 0; i < bib->nombre_livres; i++) {
        trouve = 0;
        for (j = 0; j < nb_categories; j++) {
            if (strcmp(categories[j], bib->livres[i].categorie) == 0) {
                count_categories[j]++;
                trouve = 1;
                break;
            }
        }
        if (!trouve && nb_categories < 100) {
            strcpy(categories[nb_categories], bib->livres[i].categorie);
            count_categories[nb_categories] = 1;
            nb_categories++;
        }
    }

    printf("\n=== Statistiques par catégorie ===\n");
    for (i = 0; i < nb_categories; i++) {
        double pourcentage = (double)count_categories[i] / bib->nombre_livres * 100;
        printf("%s: %d livres (%.1f%%)\n", categories[i], count_categories[i], pourcentage);
    }
}

void generer_rapport_texte(Bibliotheque *bib, const char *nom_fichier) {
    FILE *fichier;
    time_t maintenant = time(NULL);

    fichier = fopen(nom_fichier, "w");
    if (fichier == NULL) {
        printf("Erreur : Impossible de créer le fichier %s.\n", nom_fichier);
        return;
    }

    fprintf(fichier, "========================================\n");
    fprintf(fichier, "   RAPPORT DE LA BIBLIOTHÈQUE\n");
    fprintf(fichier, "========================================\n");
    fprintf(fichier, "Date de génération: %s\n", ctime(&maintenant));

    fprintf(fichier, "\nNombre total de livres: %d\n", bib->nombre_livres);
    fprintf(fichier, "Nombre total d'utilisateurs: %d\n", bib->nombre_utilisateurs);
    fprintf(fichier, "Nombre total d'emprunts: %d\n", bib->nombre_emprunts);

    fprintf(fichier, "\n--- Liste des livres ---\n");
    int i;
    for (i = 0; i < bib->nombre_livres; i++) {
        fprintf(fichier, "%d. %s - %s (%d) [%s]\n",
                i + 1,
                bib->livres[i].titre,
                bib->livres[i].auteur,
                bib->livres[i].annee,
                bib->livres[i].statut == DISPONIBLE ? "Disponible" : "Emprunté");
    }

    fprintf(fichier, "\n========================================\n");

    fclose(fichier);
    printf("Rapport généré avec succès : %s\n", nom_fichier);
}

void generer_rapport_html(Bibliotheque *bib, const char *nom_fichier) {
    FILE *fichier;
    time_t maintenant = time(NULL);
    int i;

    fichier = fopen(nom_fichier, "w");
    if (fichier == NULL) {
        printf("Erreur : Impossible de créer le fichier %s.\n", nom_fichier);
        return;
    }

    /* En-tête HTML */
    fprintf(fichier, "<!DOCTYPE html>\n");
    fprintf(fichier, "<html lang=\"fr\">\n");
    fprintf(fichier, "<head>\n");
    fprintf(fichier, "    <meta charset=\"UTF-8\">\n");
    fprintf(fichier, "    <title>Rapport Bibliothèque</title>\n");
    fprintf(fichier, "    <style>\n");
    fprintf(fichier, "        body { font-family: Arial, sans-serif; margin: 20px; }\n");
    fprintf(fichier, "        h1 { color: #333; }\n");
    fprintf(fichier, "        table { border-collapse: collapse; width: 100%%; margin-top: 20px; }\n");
    fprintf(fichier, "        th, td { border: 1px solid #ddd; padding: 12px; text-align: left; }\n");
    fprintf(fichier, "        th { background-color: #4CAF50; color: white; }\n");
    fprintf(fichier, "        tr:nth-child(even) { background-color: #f2f2f2; }\n");
    fprintf(fichier, "        .disponible { color: green; font-weight: bold; }\n");
    fprintf(fichier, "        .emprunte { color: red; font-weight: bold; }\n");
    fprintf(fichier, "    </style>\n");
    fprintf(fichier, "</head>\n");
    fprintf(fichier, "<body>\n");

    /* Contenu */
    fprintf(fichier, "    <h1>Rapport de la Bibliothèque</h1>\n");
    fprintf(fichier, "    <p><strong>Date de génération:</strong> %s</p>\n", ctime(&maintenant));

    fprintf(fichier, "    <h2>Statistiques générales</h2>\n");
    fprintf(fichier, "    <ul>\n");
    fprintf(fichier, "        <li>Nombre total de livres: %d</li>\n", bib->nombre_livres);
    fprintf(fichier, "        <li>Nombre total d'utilisateurs: %d</li>\n", bib->nombre_utilisateurs);
    fprintf(fichier, "        <li>Nombre total d'emprunts: %d</li>\n", bib->nombre_emprunts);
    fprintf(fichier, "    </ul>\n");

    fprintf(fichier, "    <h2>Liste des livres</h2>\n");
    fprintf(fichier, "    <table>\n");
    fprintf(fichier, "        <tr>\n");
    fprintf(fichier, "            <th>ID</th>\n");
    fprintf(fichier, "            <th>Titre</th>\n");
    fprintf(fichier, "            <th>Auteur</th>\n");
    fprintf(fichier, "            <th>ISBN</th>\n");
    fprintf(fichier, "            <th>Catégorie</th>\n");
    fprintf(fichier, "            <th>Année</th>\n");
    fprintf(fichier, "            <th>Statut</th>\n");
    fprintf(fichier, "            <th>Emprunts</th>\n");
    fprintf(fichier, "        </tr>\n");

    for (i = 0; i < bib->nombre_livres; i++) {
        fprintf(fichier, "        <tr>\n");
        fprintf(fichier, "            <td>%d</td>\n", bib->livres[i].id);
        fprintf(fichier, "            <td>%s</td>\n", bib->livres[i].titre);
        fprintf(fichier, "            <td>%s</td>\n", bib->livres[i].auteur);
        fprintf(fichier, "            <td>%s</td>\n", bib->livres[i].isbn);
        fprintf(fichier, "            <td>%s</td>\n", bib->livres[i].categorie);
        fprintf(fichier, "            <td>%d</td>\n", bib->livres[i].annee);
        fprintf(fichier, "            <td class=\"%s\">%s</td>\n",
                bib->livres[i].statut == DISPONIBLE ? "disponible" : "emprunte",
                bib->livres[i].statut == DISPONIBLE ? "Disponible" : "Emprunté");
        fprintf(fichier, "            <td>%d</td>\n", bib->livres[i].nombre_emprunts);
        fprintf(fichier, "        </tr>\n");
    }

    fprintf(fichier, "    </table>\n");
    fprintf(fichier, "</body>\n");
    fprintf(fichier, "</html>\n");

    fclose(fichier);
    printf("Rapport HTML généré avec succès : %s\n", nom_fichier);
}
