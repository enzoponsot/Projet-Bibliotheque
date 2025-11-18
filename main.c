#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structure.h"
#include "livres.h"
#include "utilisateurs.h"
#include "emprunts.h"
#include "fichiers.h"
#include "statistiques.h"
#include "authentification.h"

/* Fonction pour nettoyer le buffer d'entrée */
void vider_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Fonction pour lire une ligne sécurisée */
void lire_ligne(char *buffer, int taille) {
    if (fgets(buffer, taille, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
    }
}

/* Menu de connexion */
Utilisateur* menu_connexion(Bibliotheque *bib) {
    char email[MAX_EMAIL];
    char password[MAX_PASSWORD];
    Utilisateur *utilisateur;
    int choix;

    while (1) {
        printf("\n========================================\n");
        printf("   SYSTÈME DE GESTION DE BIBLIOTHÈQUE\n");
        printf("========================================\n");
        printf("1. Se connecter\n");
        printf("2. Creer un compte utilisateur\n");
        printf("0. Quitter\n");
        printf("Choix : ");

        if (scanf("%d", &choix) != 1) {
            vider_buffer();
            printf("Entrée invalide.\n");
            continue;
        }
        vider_buffer();

        switch (choix) {
            case 1:
                printf("\nEmail : ");
                lire_ligne(email, MAX_EMAIL);
                printf("Mot de passe : ");
                lire_ligne(password, MAX_PASSWORD);

                utilisateur = authentifier(bib, email, password);
                if (utilisateur != NULL) {
                    printf("\nConnexion réussie ! Bienvenue %s %s\n",
                           utilisateur->prenom, utilisateur->nom);
                    return utilisateur;
                } else {
                    printf("\nErreur : Email ou mot de passe incorrect.\n");
                }
                break;

            case 2: {
                char nom[MAX_NOM], prenom[MAX_PRENOM], id_etudiant[MAX_ID_ETUDIANT];
                printf("\nCréation d'un nouveau compte\n");
                printf("Nom : ");
                lire_ligne(nom, MAX_NOM);
                printf("Prénom : ");
                lire_ligne(prenom, MAX_PRENOM);
                printf("ID Étudiant : ");
                lire_ligne(id_etudiant, MAX_ID_ETUDIANT);
                printf("Email : ");
                lire_ligne(email, MAX_EMAIL);
                printf("Mot de passe : ");
                lire_ligne(password, MAX_PASSWORD);

                if (ajouter_utilisateur(bib, nom, prenom, id_etudiant, email,
                                        password, UTILISATEUR_SIMPLE)) {
                    printf("\nCompte créé avec succès ! Vous pouvez maintenant vous connecter.\n");
                    sauvegarder_tout(bib);
                }
                break;
            }

            case 0:
                return NULL;

            default:
                printf("Choix invalide.\n");
        }
    }
}

/* Menu administrateur */
void menu_admin(Bibliotheque *bib, Utilisateur *utilisateur) {
    int choix;

    while (1) {
        printf("\n========================================\n");
        printf("   MENU ADMINISTRATEUR\n");
        printf("   Connecté: %s %s\n", utilisateur->prenom, utilisateur->nom);
        printf("========================================\n");
        printf("1. Gestion des livres\n");
        printf("2. Gestion des utilisateurs\n");
        printf("3. Gestion des emprunts\n");
        printf("4. Statistiques et rapports\n");
        printf("5. Sauvegarder les données\n");
        printf("0. Se déconnecter\n");
        printf("Choix : ");

        if (scanf("%d", &choix) != 1) {
            vider_buffer();
            printf("Entrée invalide.\n");
            continue;
        }
        vider_buffer();

        switch (choix) {
            case 1: {
                int sous_choix;
                printf("\n--- Gestion des livres ---\n");
                printf("1. Ajouter un livre\n");
                printf("2. Supprimer un livre\n");
                printf("3. Modifier un livre\n");
                printf("4. Rechercher un livre\n");
                printf("5. Afficher tous les livres\n");
                printf("6. Afficher les livres triés\n");
                printf("7. Recherche multi-critères\n");
                printf("Choix : ");

                if (scanf("%d", &sous_choix) != 1) {
                    vider_buffer();
                    continue;
                }
                vider_buffer();

                if (sous_choix == 1) {
                    char titre[MAX_TITRE], auteur[MAX_AUTEUR], isbn[MAX_ISBN], categorie[MAX_CATEGORIE];
                    int annee;
                    printf("Titre : ");
                    lire_ligne(titre, MAX_TITRE);
                    printf("Auteur : ");
                    lire_ligne(auteur, MAX_AUTEUR);
                    printf("ISBN : ");
                    lire_ligne(isbn, MAX_ISBN);
                    printf("Catégorie : ");
                    lire_ligne(categorie, MAX_CATEGORIE);
                    printf("Année : ");
                    scanf("%d", &annee);
                    vider_buffer();
                    ajouter_livre(bib, titre, auteur, isbn, categorie, annee);
                    sauvegarder_tout(bib);
                } else if (sous_choix == 2) {
                    int id;
                    printf("ID du livre à supprimer : ");
                    scanf("%d", &id);
                    vider_buffer();
                    if (supprimer_livre(bib, id)) {
                        sauvegarder_tout(bib);
                    }
                } else if (sous_choix == 3) {
                    char titre[MAX_TITRE], auteur[MAX_AUTEUR], isbn[MAX_ISBN], categorie[MAX_CATEGORIE];
                    int id, annee;
                    printf("ID du livre à modifier : ");
                    scanf("%d", &id);
                    vider_buffer();
                    printf("Nouveau titre : ");
                    lire_ligne(titre, MAX_TITRE);
                    printf("Nouvel auteur : ");
                    lire_ligne(auteur, MAX_AUTEUR);
                    printf("Nouvel ISBN : ");
                    lire_ligne(isbn, MAX_ISBN);
                    printf("Nouvelle catégorie : ");
                    lire_ligne(categorie, MAX_CATEGORIE);
                    printf("Nouvelle année : ");
                    scanf("%d", &annee);
                    vider_buffer();
                    if (modifier_livre(bib, id, titre, auteur, isbn, categorie, annee)) {
                        sauvegarder_tout(bib);
                    }
                } else if (sous_choix == 4) {
                    int type_recherche;
                    printf("1. Par titre\n2. Par auteur\n3. Par ISBN\n4. Par catégorie\nChoix : ");
                    scanf("%d", &type_recherche);
                    vider_buffer();

                    if (type_recherche == 1) {
                        char titre[MAX_TITRE];
                        printf("Titre : ");
                        lire_ligne(titre, MAX_TITRE);
                        Livre *livre = rechercher_livre_par_titre(bib, titre);
                        if (livre != NULL) afficher_livre(livre);
                        else printf("Livre non trouvé.\n");
                    } else if (type_recherche == 2) {
                        char auteur[MAX_AUTEUR];
                        printf("Auteur : ");
                        lire_ligne(auteur, MAX_AUTEUR);
                        Livre *livre = rechercher_livre_par_auteur(bib, auteur);
                        if (livre != NULL) afficher_livre(livre);
                        else printf("Livre non trouvé.\n");
                    } else if (type_recherche == 3) {
                        char isbn[MAX_ISBN];
                        printf("ISBN : ");
                        lire_ligne(isbn, MAX_ISBN);
                        Livre *livre = rechercher_livre_par_isbn(bib, isbn);
                        if (livre != NULL) afficher_livre(livre);
                        else printf("Livre non trouvé.\n");
                    } else if (type_recherche == 4) {
                        char categorie[MAX_CATEGORIE];
                        printf("Catégorie : ");
                        lire_ligne(categorie, MAX_CATEGORIE);
                        afficher_livres_par_categorie(bib, categorie);
                    }
                } else if (sous_choix == 5) {
                    afficher_tous_les_livres(bib);
                } else if (sous_choix == 6) {
                    int type_tri;
                    printf("1. Par titre\n2. Par année\n3. Par auteur\n4. Par popularité\nChoix : ");
                    scanf("%d", &type_tri);
                    vider_buffer();
                    if (type_tri == 1) afficher_livres_tries_par_titre(bib);
                    else if (type_tri == 2) afficher_livres_tries_par_annee(bib);
                    else if (type_tri == 3) afficher_livres_tries_par_auteur(bib);
                    else if (type_tri == 4) afficher_livres_tries_par_emprunts(bib);
                } else if (sous_choix == 7) {
                    char titre[MAX_TITRE] = "", auteur[MAX_AUTEUR] = "", categorie[MAX_CATEGORIE] = "";
                    Livre *resultats[MAX_LIVRES];
                    int nb_resultats;

                    printf("Titre (vide pour ignorer) : ");
                    lire_ligne(titre, MAX_TITRE);
                    printf("Auteur (vide pour ignorer) : ");
                    lire_ligne(auteur, MAX_AUTEUR);
                    printf("Catégorie (vide pour ignorer) : ");
                    lire_ligne(categorie, MAX_CATEGORIE);

                    nb_resultats = rechercher_livres_multi_criteres(bib,
                                                                    strlen(titre) > 0 ? titre : NULL,
                                                                    strlen(auteur) > 0 ? auteur : NULL,
                                                                    strlen(categorie) > 0 ? categorie : NULL,
                                                                    resultats, MAX_LIVRES);

                    printf("\n%d résultat(s) trouvé(s) :\n", nb_resultats);
                    int i;
                    for (i = 0; i < nb_resultats; i++) {
                        afficher_livre(resultats[i]);
                    }
                }
                break;
            }

            case 2: {
                int sous_choix;
                printf("\n--- Gestion des utilisateurs ---\n");
                printf("1. Ajouter un utilisateur\n");
                printf("2. Supprimer un utilisateur\n");
                printf("3. Modifier un utilisateur\n");
                printf("4. Afficher tous les utilisateurs\n");
                printf("5. Rechercher un utilisateur\n");
                printf("Choix : ");

                if (scanf("%d", &sous_choix) != 1) {
                    vider_buffer();
                    continue;
                }
                vider_buffer();

                if (sous_choix == 1) {
                    char nom[MAX_NOM], prenom[MAX_PRENOM], id_etudiant[MAX_ID_ETUDIANT];
                    char email[MAX_EMAIL], password[MAX_PASSWORD];
                    int type;
                    printf("Nom : ");
                    lire_ligne(nom, MAX_NOM);
                    printf("Prénom : ");
                    lire_ligne(prenom, MAX_PRENOM);
                    printf("ID Étudiant : ");
                    lire_ligne(id_etudiant, MAX_ID_ETUDIANT);
                    printf("Email : ");
                    lire_ligne(email, MAX_EMAIL);
                    printf("Mot de passe : ");
                    lire_ligne(password, MAX_PASSWORD);
                    printf("Type (0=Utilisateur, 1=Admin) : ");
                    scanf("%d", &type);
                    vider_buffer();
                    if (ajouter_utilisateur(bib, nom, prenom, id_etudiant, email, password,
                                            type == 1 ? ADMIN : UTILISATEUR_SIMPLE)) {
                        sauvegarder_tout(bib);
                    }
                } else if (sous_choix == 2) {
                    int id;
                    printf("ID de l'utilisateur à supprimer : ");
                    scanf("%d", &id);
                    vider_buffer();
                    if (supprimer_utilisateur(bib, id)) {
                        sauvegarder_tout(bib);
                    }
                } else if (sous_choix == 3) {
                    char nom[MAX_NOM], prenom[MAX_PRENOM], id_etudiant[MAX_ID_ETUDIANT], email[MAX_EMAIL];
                    int id;
                    printf("ID de l'utilisateur à modifier : ");
                    scanf("%d", &id);
                    vider_buffer();
                    printf("Nouveau nom : ");
                    lire_ligne(nom, MAX_NOM);
                    printf("Nouveau prénom : ");
                    lire_ligne(prenom, MAX_PRENOM);
                    printf("Nouvel ID étudiant : ");
                    lire_ligne(id_etudiant, MAX_ID_ETUDIANT);
                    printf("Nouvel email : ");
                    lire_ligne(email, MAX_EMAIL);
                    if (modifier_utilisateur(bib, id, nom, prenom, id_etudiant, email)) {
                        sauvegarder_tout(bib);
                    }
                } else if (sous_choix == 4) {
                    afficher_utilisateurs_tries(bib);
                } else if (sous_choix == 5) {
                    int id;
                    printf("ID de l'utilisateur : ");
                    scanf("%d", &id);
                    vider_buffer();
                    Utilisateur *user = rechercher_utilisateur_par_id(bib, id);
                    if (user != NULL) afficher_utilisateur(user);
                    else printf("Utilisateur non trouvé.\n");
                }
                break;
            }

            case 3: {
                int sous_choix;
                printf("\n--- Gestion des emprunts ---\n");
                printf("1. Enregistrer un emprunt\n");
                printf("2. Enregistrer un retour\n");
                printf("3. Afficher les emprunts actifs\n");
                printf("4. Afficher tous les emprunts\n");
                printf("5. Vérifier les retards\n");
                printf("6. Afficher les emprunts en retard\n");
                printf("7. Historique d'un utilisateur\n");
                printf("8. Prolonger un emprunt\n");
                printf("Choix : ");

                if (scanf("%d", &sous_choix) != 1) {
                    vider_buffer();
                    continue;
                }
                vider_buffer();

                if (sous_choix == 1) {
                    int id_livre, id_utilisateur;
                    printf("ID du livre : ");
                    scanf("%d", &id_livre);
                    printf("ID de l'utilisateur : ");
                    scanf("%d", &id_utilisateur);
                    vider_buffer();
                    if (enregistrer_emprunt(bib, id_livre, id_utilisateur)) {
                        sauvegarder_tout(bib);
                    }
                } else if (sous_choix == 2) {
                    int id_livre, id_utilisateur;
                    printf("ID du livre : ");
                    scanf("%d", &id_livre);
                    printf("ID de l'utilisateur : ");
                    scanf("%d", &id_utilisateur);
                    vider_buffer();
                    if (enregistrer_retour(bib, id_livre, id_utilisateur)) {
                        sauvegarder_tout(bib);
                    }
                } else if (sous_choix == 3) {
                    afficher_emprunts_actifs(bib);
                } else if (sous_choix == 4) {
                    afficher_tous_les_emprunts(bib);
                } else if (sous_choix == 5) {
                    verifier_retards(bib);
                } else if (sous_choix == 6) {
                    afficher_emprunts_en_retard(bib);
                } else if (sous_choix == 7) {
                    int id_utilisateur;
                    printf("ID de l'utilisateur : ");
                    scanf("%d", &id_utilisateur);
                    vider_buffer();
                    afficher_historique_utilisateur(bib, id_utilisateur);
                } else if (sous_choix == 8) {
                    int id_livre, id_utilisateur, jours;
                    printf("ID du livre : ");
                    scanf("%d", &id_livre);
                    printf("ID de l'utilisateur : ");
                    scanf("%d", &id_utilisateur);
                    printf("Nombre de jours de prolongation : ");
                    scanf("%d", &jours);
                    vider_buffer();
                    if (prolonger_emprunt(bib, id_livre, id_utilisateur, jours)) {
                        sauvegarder_tout(bib);
                    }
                }
                break;
            }

            case 4: {
                int sous_choix;
                printf("\n--- Statistiques et rapports ---\n");
                printf("1. Statistiques générales\n");
                printf("2. Top livres les plus empruntés\n");
                printf("3. Top utilisateurs les plus actifs\n");
                printf("4. Statistiques par catégorie\n");
                printf("5. Générer rapport texte\n");
                printf("6. Générer rapport HTML\n");
                printf("Choix : ");

                if (scanf("%d", &sous_choix) != 1) {
                    vider_buffer();
                    continue;
                }
                vider_buffer();

                if (sous_choix == 1) {
                    afficher_statistiques_generales(bib);
                } else if (sous_choix == 2) {
                    int n;
                    printf("Nombre de livres à afficher : ");
                    scanf("%d", &n);
                    vider_buffer();
                    afficher_top_livres(bib, n);
                } else if (sous_choix == 3) {
                    int n;
                    printf("Nombre d'utilisateurs à afficher : ");
                    scanf("%d", &n);
                    vider_buffer();
                    afficher_top_utilisateurs(bib, n);
                } else if (sous_choix == 4) {
                    afficher_statistiques_categories(bib);
                } else if (sous_choix == 5) {
                    generer_rapport_texte(bib, "rapport_bibliotheque.txt");
                } else if (sous_choix == 6) {
                    generer_rapport_html(bib, "rapport_bibliotheque.html");
                }
                break;
            }

            case 5:
                sauvegarder_tout(bib);
                break;

            case 0:
                printf("Déconnexion...\n");
                return;

            default:
                printf("Choix invalide.\n");
        }
    }
}

/* Menu utilisateur simple */
void menu_utilisateur(Bibliotheque *bib, Utilisateur *utilisateur) {
    int choix;

    while (1) {
        printf("\n========================================\n");
        printf("   MENU UTILISATEUR\n");
        printf("   Connecté: %s %s\n", utilisateur->prenom, utilisateur->nom);
        printf("   Emprunts actifs: %d/%d\n", utilisateur->nombre_emprunts_actifs, MAX_EMPRUNTS_PAR_USER);
        printf("   Pénalités: %.2f €\n", utilisateur->penalites);
        printf("========================================\n");
        printf("1. Consulter le catalogue\n");
        printf("2. Rechercher un livre\n");
        printf("3. Mon historique d'emprunts\n");
        printf("4. Mes emprunts en cours\n");
        printf("5. Modifier mon mot de passe\n");
        printf("6. Payer mes pénalités\n");
        printf("0. Se déconnecter\n");
        printf("Choix : ");

        if (scanf("%d", &choix) != 1) {
            vider_buffer();
            printf("Entrée invalide.\n");
            continue;
        }
        vider_buffer();

        switch (choix) {
            case 1: {
                int type_affichage;
                printf("1. Tous les livres\n2. Par catégorie\n3. Triés\nChoix : ");
                scanf("%d", &type_affichage);
                vider_buffer();

                if (type_affichage == 1) {
                    afficher_tous_les_livres(bib);
                } else if (type_affichage == 2) {
                    char categorie[MAX_CATEGORIE];
                    printf("Catégorie : ");
                    lire_ligne(categorie, MAX_CATEGORIE);
                    afficher_livres_par_categorie(bib, categorie);
                } else if (type_affichage == 3) {
                    int type_tri;
                    printf("1. Par titre\n2. Par année\n3. Par auteur\nChoix : ");
                    scanf("%d", &type_tri);
                    vider_buffer();
                    if (type_tri == 1) afficher_livres_tries_par_titre(bib);
                    else if (type_tri == 2) afficher_livres_tries_par_annee(bib);
                    else if (type_tri == 3) afficher_livres_tries_par_auteur(bib);
                }
                break;
            }

            case 2: {
                int type_recherche;
                printf("1. Par titre\n2. Par auteur\n3. Par ISBN\nChoix : ");
                scanf("%d", &type_recherche);
                vider_buffer();

                if (type_recherche == 1) {
                    char titre[MAX_TITRE];
                    printf("Titre : ");
                    lire_ligne(titre, MAX_TITRE);
                    Livre *livre = rechercher_livre_par_titre(bib, titre);
                    if (livre != NULL) afficher_livre(livre);
                    else printf("Livre non trouvé.\n");
                } else if (type_recherche == 2) {
                    char auteur[MAX_AUTEUR];
                    printf("Auteur : ");
                    lire_ligne(auteur, MAX_AUTEUR);
                    Livre *livre = rechercher_livre_par_auteur(bib, auteur);
                    if (livre != NULL) afficher_livre(livre);
                    else printf("Livre non trouvé.\n");
                } else if (type_recherche == 3) {
                    char isbn[MAX_ISBN];
                    printf("ISBN : ");
                    lire_ligne(isbn, MAX_ISBN);
                    Livre *livre = rechercher_livre_par_isbn(bib, isbn);
                    if (livre != NULL) afficher_livre(livre);
                    else printf("Livre non trouvé.\n");
                }
                break;
            }

            case 3:
                afficher_historique_utilisateur(bib, utilisateur->id);
                break;

            case 4: {
                int i, count = 0;
                printf("\n=== Mes emprunts en cours ===\n");
                for (i = 0; i < bib->nombre_emprunts; i++) {
                    if (bib->emprunts[i].id_utilisateur == utilisateur->id &&
                        bib->emprunts[i].date_retour_effectif == 0) {
                        Livre *livre = rechercher_livre_par_id(bib, bib->emprunts[i].id_livre);
                        printf("----------------------------------------\n");
                        printf("Livre: %s\n", livre->titre);
                        printf("Date d'emprunt: %s", ctime(&bib->emprunts[i].date_emprunt));
                        printf("Date de retour prévue: %s", ctime(&bib->emprunts[i].date_retour_prevue));
                        if (bib->emprunts[i].est_en_retard) {
                            printf("ATTENTION: Emprunt en retard !\n");
                        }
                        count++;
                    }
                }
                if (count == 0) {
                    printf("Aucun emprunt en cours.\n");
                }
                break;
            }

            case 5: {
                char ancien[MAX_PASSWORD], nouveau[MAX_PASSWORD];
                printf("Ancien mot de passe : ");
                lire_ligne(ancien, MAX_PASSWORD);
                printf("Nouveau mot de passe : ");
                lire_ligne(nouveau, MAX_PASSWORD);
                if (modifier_password_utilisateur(bib, utilisateur->id, ancien, nouveau)) {
                    sauvegarder_tout(bib);
                }
                break;
            }

            case 6: {
                double montant;
                printf("Pénalités actuelles : %.2f €\n", utilisateur->penalites);
                if (utilisateur->penalites > 0) {
                    printf("Montant à payer : ");
                    scanf("%lf", &montant);
                    vider_buffer();
                    if (payer_penalites(bib, utilisateur->id, montant)) {
                        sauvegarder_tout(bib);
                    }
                } else {
                    printf("Vous n'avez aucune pénalité à payer.\n");
                }
                break;
            }

            case 0:
                printf("Déconnexion...\n");
                return;

            default:
                printf("Choix invalide.\n");
        }
    }
}

/* Fonction principale */
int main() {
    Bibliotheque bib;
    Utilisateur *utilisateur_connecte;

    printf("========================================\n");
    printf("   BIBLIOTHÈQUE NUMÉRIQUE AVANCÉE\n");
    printf("   Version 1.0\n");
    printf("========================================\n");

    /* Initialiser la bibliothèque */
    initialiser_bibliotheque(&bib);

    /* Charger les données existantes */
    charger_tout(&bib);

    /* Créer l'admin par défaut si nécessaire */
    creer_admin_par_defaut(&bib);
    sauvegarder_tout(&bib);

    /* Boucle principale */
    while (1) {
        utilisateur_connecte = menu_connexion(&bib);

        if (utilisateur_connecte == NULL) {
            /* L'utilisateur a choisi de quitter */
            break;
        }

        /* Rediriger vers le bon menu selon le type d'utilisateur */
        if (est_admin(utilisateur_connecte)) {
            menu_admin(&bib, utilisateur_connecte);
        } else {
            menu_utilisateur(&bib, utilisateur_connecte);
        }
    }

    /* Sauvegarder avant de quitter */
    printf("\nSauvegarde finale...\n");
    sauvegarder_tout(&bib);

    printf("\nMerci d'avoir utilisé le système de gestion de bibliothèque.\n");
    printf("Au revoir !\n");

    return 0;
}