#include "emprunts.h"
#include "livres.h"
#include "utilisateurs.h"
#include <stdio.h>
#include <time.h>

/* Calcule le nombre de jours entre deux dates */
static int calculer_jours_difference(time_t date1, time_t date2) {
    return (int)difftime(date2, date1) / (60 * 60 * 24);
}

int enregistrer_emprunt(Bibliotheque *bib, int id_livre, int id_utilisateur) {
    Livre *livre;
    Utilisateur *utilisateur;
    Emprunt *nouvel_emprunt;

    if (bib->nombre_emprunts >= MAX_EMPRUNTS) {
        printf("Erreur : La base d'emprunts est pleine.\n");
        return 0;
    }

    /* Vérifier que le livre existe et est disponible */
    livre = rechercher_livre_par_id(bib, id_livre);
    if (livre == NULL) {
        printf("Erreur : Livre non trouvé.\n");
        return 0;
    }

    if (livre->statut == EMPRUNTE) {
        printf("Erreur : Ce livre est déjà emprunté.\n");
        return 0;
    }

    /* Vérifier que l'utilisateur existe et peut emprunter */
    utilisateur = rechercher_utilisateur_par_id(bib, id_utilisateur);
    if (utilisateur == NULL) {
        printf("Erreur : Utilisateur non trouvé.\n");
        return 0;
    }

    if (!peut_emprunter(bib, id_utilisateur)) {
        printf("Erreur : L'utilisateur a atteint la limite d'emprunts (%d/%d).\n",
               utilisateur->nombre_emprunts_actifs, MAX_EMPRUNTS_PAR_USER);
        return 0;
    }

    /* Créer le nouvel emprunt */
    nouvel_emprunt = &bib->emprunts[bib->nombre_emprunts];
    nouvel_emprunt->id = bib->prochain_id_emprunt++;
    nouvel_emprunt->id_livre = id_livre;
    nouvel_emprunt->id_utilisateur = id_utilisateur;
    nouvel_emprunt->date_emprunt = time(NULL);
    nouvel_emprunt->date_retour_prevue = nouvel_emprunt->date_emprunt +
                                         (JOURS_LIMITE_EMPRUNT * 24 * 60 * 60);
    nouvel_emprunt->date_retour_effectif = 0;
    nouvel_emprunt->est_en_retard = 0;
    nouvel_emprunt->penalite = 0.0;

    /* Mettre à jour le statut du livre et de l'utilisateur */
    livre->statut = EMPRUNTE;
    livre->nombre_emprunts++;
    utilisateur->nombre_emprunts_actifs++;

    bib->nombre_emprunts++;

    printf("Emprunt enregistré avec succès (ID: %d).\n", nouvel_emprunt->id);
    printf("Date de retour prévue : %s", ctime(&nouvel_emprunt->date_retour_prevue));

    return 1;
}

int enregistrer_retour(Bibliotheque *bib, int id_livre, int id_utilisateur) {
    Emprunt *emprunt;
    Livre *livre;
    Utilisateur *utilisateur;
    time_t maintenant;
    int jours_retard;
    double penalite;

    /* Rechercher l'emprunt actif */
    emprunt = rechercher_emprunt_actif(bib, id_livre, id_utilisateur);
    if (emprunt == NULL) {
        printf("Erreur : Aucun emprunt actif trouvé pour ce livre et cet utilisateur.\n");
        return 0;
    }

    livre = rechercher_livre_par_id(bib, id_livre);
    utilisateur = rechercher_utilisateur_par_id(bib, id_utilisateur);

    /* Enregistrer la date de retour */
    maintenant = time(NULL);
    emprunt->date_retour_effectif = maintenant;

    /* Calculer le retard éventuel */
    jours_retard = calculer_jours_difference(emprunt->date_retour_prevue, maintenant);

    if (jours_retard > 0) {
        emprunt->est_en_retard = 1;
        penalite = jours_retard * PENALITE_PAR_JOUR;
        emprunt->penalite = penalite;
        ajouter_penalite(bib, id_utilisateur, penalite);

        printf("ATTENTION : Retour en retard de %d jour(s).\n", jours_retard);
        printf("Pénalité appliquée : %.2f €\n", penalite);
        printf("Total pénalités de l'utilisateur : %.2f €\n", utilisateur->penalites);
    } else {
        printf("Retour dans les délais.\n");
    }

    /* Mettre à jour le statut du livre et de l'utilisateur */
    livre->statut = DISPONIBLE;
    utilisateur->nombre_emprunts_actifs--;

    printf("Retour enregistré avec succès.\n");
    return 1;
}

Emprunt* rechercher_emprunt_actif(Bibliotheque *bib, int id_livre, int id_utilisateur) {
    int i;
    for (i = 0; i < bib->nombre_emprunts; i++) {
        if (bib->emprunts[i].id_livre == id_livre &&
            bib->emprunts[i].id_utilisateur == id_utilisateur &&
            bib->emprunts[i].date_retour_effectif == 0) {
            return &bib->emprunts[i];
        }
    }
    return NULL;
}

int peut_emprunter(Bibliotheque *bib, int id_utilisateur) {
    Utilisateur *utilisateur = rechercher_utilisateur_par_id(bib, id_utilisateur);
    if (utilisateur == NULL) {
        return 0;
    }
    return utilisateur->nombre_emprunts_actifs < MAX_EMPRUNTS_PAR_USER;
}

void verifier_retards(Bibliotheque *bib) {
    int i;
    time_t maintenant = time(NULL);
    int nombre_retards = 0;

    printf("\n=== Vérification des retards ===\n");

    for (i = 0; i < bib->nombre_emprunts; i++) {
        if (bib->emprunts[i].date_retour_effectif == 0) { /* Emprunt actif */
            int jours_retard = calculer_jours_difference(bib->emprunts[i].date_retour_prevue, maintenant);

            if (jours_retard > 0) {
                Livre *livre = rechercher_livre_par_id(bib, bib->emprunts[i].id_livre);
                Utilisateur *utilisateur = rechercher_utilisateur_par_id(bib, bib->emprunts[i].id_utilisateur);

                if (!bib->emprunts[i].est_en_retard) {
                    bib->emprunts[i].est_en_retard = 1;
                }

                nombre_retards++;
                printf("\nRetard détecté :\n");
                printf("  Livre : %s (ID: %d)\n", livre->titre, livre->id);
                printf("  Utilisateur : %s %s (ID: %d)\n",
                       utilisateur->prenom, utilisateur->nom, utilisateur->id);
                printf("  Retard : %d jour(s)\n", jours_retard);
                printf("  Pénalité actuelle : %.2f €\n", jours_retard * PENALITE_PAR_JOUR);
            }
        }
    }

    if (nombre_retards == 0) {
        printf("Aucun retard détecté.\n");
    } else {
        printf("\nTotal : %d emprunt(s) en retard.\n", nombre_retards);
    }
}

void afficher_emprunts_actifs(Bibliotheque *bib) {
    int i;
    int count = 0;

    printf("\n=== Emprunts actifs ===\n");

    for (i = 0; i < bib->nombre_emprunts; i++) {
        if (bib->emprunts[i].date_retour_effectif == 0) {
            Livre *livre = rechercher_livre_par_id(bib, bib->emprunts[i].id_livre);
            Utilisateur *utilisateur = rechercher_utilisateur_par_id(bib, bib->emprunts[i].id_utilisateur);

            printf("----------------------------------------\n");
            printf("ID Emprunt: %d\n", bib->emprunts[i].id);
            printf("Livre: %s (ID: %d)\n", livre->titre, livre->id);
            printf("Utilisateur: %s %s (ID: %d)\n",
                   utilisateur->prenom, utilisateur->nom, utilisateur->id);
            printf("Date d'emprunt: %s", ctime(&bib->emprunts[i].date_emprunt));
            printf("Date de retour prévue: %s", ctime(&bib->emprunts[i].date_retour_prevue));

            if (bib->emprunts[i].est_en_retard) {
                int jours_retard = calculer_jours_difference(bib->emprunts[i].date_retour_prevue, time(NULL));
                printf("STATUT: EN RETARD (%d jours)\n", jours_retard);
            } else {
                printf("STATUT: Dans les délais\n");
            }

            count++;
        }
    }

    if (count == 0) {
        printf("Aucun emprunt actif.\n");
    } else {
        printf("\nTotal : %d emprunt(s) actif(s).\n", count);
    }
}

void afficher_tous_les_emprunts(Bibliotheque *bib) {
    int i;

    if (bib->nombre_emprunts == 0) {
        printf("Aucun emprunt enregistré.\n");
        return;
    }

    printf("\n=== Historique complet des emprunts (%d) ===\n", bib->nombre_emprunts);

    for (i = 0; i < bib->nombre_emprunts; i++) {
        Livre *livre = rechercher_livre_par_id(bib, bib->emprunts[i].id_livre);
        Utilisateur *utilisateur = rechercher_utilisateur_par_id(bib, bib->emprunts[i].id_utilisateur);

        printf("----------------------------------------\n");
        printf("ID Emprunt: %d\n", bib->emprunts[i].id);
        printf("Livre: %s (ID: %d)\n", livre->titre, livre->id);
        printf("Utilisateur: %s %s (ID: %d)\n",
               utilisateur->prenom, utilisateur->nom, utilisateur->id);
        printf("Date d'emprunt: %s", ctime(&bib->emprunts[i].date_emprunt));
        printf("Date de retour prévue: %s", ctime(&bib->emprunts[i].date_retour_prevue));

        if (bib->emprunts[i].date_retour_effectif != 0) {
            printf("Date de retour effectif: %s", ctime(&bib->emprunts[i].date_retour_effectif));
            if (bib->emprunts[i].penalite > 0) {
                printf("Pénalité: %.2f €\n", bib->emprunts[i].penalite);
            }
        } else {
            printf("STATUT: En cours\n");
        }
    }
}

void afficher_historique_utilisateur(Bibliotheque *bib, int id_utilisateur) {
    int i;
    int count = 0;
    Utilisateur *utilisateur = rechercher_utilisateur_par_id(bib, id_utilisateur);

    if (utilisateur == NULL) {
        printf("Erreur : Utilisateur non trouvé.\n");
        return;
    }

    printf("\n=== Historique des emprunts de %s %s ===\n",
           utilisateur->prenom, utilisateur->nom);

    for (i = 0; i < bib->nombre_emprunts; i++) {
        if (bib->emprunts[i].id_utilisateur == id_utilisateur) {
            Livre *livre = rechercher_livre_par_id(bib, bib->emprunts[i].id_livre);

            printf("----------------------------------------\n");
            printf("Livre: %s\n", livre->titre);
            printf("Date d'emprunt: %s", ctime(&bib->emprunts[i].date_emprunt));

            if (bib->emprunts[i].date_retour_effectif != 0) {
                printf("Date de retour: %s", ctime(&bib->emprunts[i].date_retour_effectif));
                if (bib->emprunts[i].penalite > 0) {
                    printf("Pénalité: %.2f €\n", bib->emprunts[i].penalite);
                }
            } else {
                printf("STATUT: En cours\n");
            }

            count++;
        }
    }

    if (count == 0) {
        printf("Aucun emprunt pour cet utilisateur.\n");
    } else {
        printf("\nTotal : %d emprunt(s).\n", count);
    }
}

void afficher_emprunts_en_retard(Bibliotheque *bib) {
    int i;
    int count = 0;
    time_t maintenant = time(NULL);

    printf("\n=== Emprunts en retard ===\n");

    for (i = 0; i < bib->nombre_emprunts; i++) {
        if (bib->emprunts[i].date_retour_effectif == 0 && bib->emprunts[i].est_en_retard) {
            Livre *livre = rechercher_livre_par_id(bib, bib->emprunts[i].id_livre);
            Utilisateur *utilisateur = rechercher_utilisateur_par_id(bib, bib->emprunts[i].id_utilisateur);
            int jours_retard = calculer_jours_difference(bib->emprunts[i].date_retour_prevue, maintenant);

            printf("----------------------------------------\n");
            printf("Livre: %s (ID: %d)\n", livre->titre, livre->id);
            printf("Utilisateur: %s %s (ID: %d)\n",
                   utilisateur->prenom, utilisateur->nom, utilisateur->id);
            printf("Email: %s\n", utilisateur->email);
            printf("Retard: %d jour(s)\n", jours_retard);
            printf("Pénalité accumulée: %.2f €\n", jours_retard * PENALITE_PAR_JOUR);

            count++;
        }
    }

    if (count == 0) {
        printf("Aucun emprunt en retard.\n");
    } else {
        printf("\nTotal : %d emprunt(s) en retard.\n", count);
    }
}

int prolonger_emprunt(Bibliotheque *bib, int id_livre, int id_utilisateur, int jours) {
    Emprunt *emprunt = rechercher_emprunt_actif(bib, id_livre, id_utilisateur);

    if (emprunt == NULL) {
        printf("Erreur : Aucun emprunt actif trouvé.\n");
        return 0;
    }

    if (emprunt->est_en_retard) {
        printf("Erreur : Impossible de prolonger un emprunt en retard.\n");
        return 0;
    }

    emprunt->date_retour_prevue += (jours * 24 * 60 * 60);
    printf("Emprunt prolongé de %d jour(s).\n", jours);
    printf("Nouvelle date de retour prévue : %s", ctime(&emprunt->date_retour_prevue));

    return 1;
}
