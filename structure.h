//
// Header principal du projet Bibliothèque
// Regroupe toutes les constantes, enums, structures et prototypes
// Date : 03/10/2025
//

#ifndef STRUCTURES_H
#define STRUCTURES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

/* =========================
 *   Constantes générales
 * ========================= */
#define MAX_TITRE             128
#define MAX_AUTEUR            128
#define MAX_ISBN              20
#define MAX_CATEGORIE         64

#define MAX_NOM               64
#define MAX_PRENOM            64
#define MAX_ID_ETUDIANT       32
#define MAX_EMAIL             128
#define MAX_PASSWORD          64

#define MAX_LIVRES            1000
#define MAX_UTILISATEURS      500
#define MAX_EMPRUNTS          5000

#define MAX_EMPRUNTS_PAR_USER 5        /* quota d’emprunts simultanés */
#define JOURS_LIMITE_EMPRUNT  14       /* durée standard d’un emprunt (jours) */
#define PENALITE_PAR_JOUR     0.50     /* € par jour de retard */

/* (optionnel, utile pour des boucles/allocs de stats) */
#define MAX_CATEGORIES        100

/* =========================
 *       Énumérations
 * ========================= */
typedef enum {
    DISPONIBLE = 0,
    EMPRUNTE   = 1
} StatutLivre;

typedef enum {
    UTILISATEUR_SIMPLE = 0,
    ADMIN              = 1
} TypeUtilisateur;

/* =========================
 *        Structures
 * ========================= */

/* Livre */
typedef struct {
    int   id;
    char  titre[MAX_TITRE];
    char  auteur[MAX_AUTEUR];
    char  isbn[MAX_ISBN];
    char  categorie[MAX_CATEGORIE];
    int   annee;
    StatutLivre statut;
    int   nombre_emprunts;   /* compteur d’emprunts (popularité) */
} Livre;

/* Utilisateur */
typedef struct {
    int   id;
    char  nom[MAX_NOM];
    char  prenom[MAX_PRENOM];
    char  id_etudiant[MAX_ID_ETUDIANT];
    char  email[MAX_EMAIL];
    char  password[MAX_PASSWORD];
    TypeUtilisateur type;
    int   nombre_emprunts_actifs;  /* emprunts en cours */
    double penalites;              /* total dû en € */
} Utilisateur;

/* Emprunt */
typedef struct {
    int    id;
    int    id_livre;
    int    id_utilisateur;
    time_t date_emprunt;
    time_t date_retour_prevue;
    time_t date_retour_effectif; /* 0 si non rendu */
    int    est_en_retard;        /* booléen (0/1) */
    double penalite;             /* pénalité associée à cet emprunt */
} Emprunt;

/* Bibliothèque (agrégat principal) */
typedef struct {
    /* collections */
    Livre        livres[MAX_LIVRES];
    Utilisateur  utilisateurs[MAX_UTILISATEURS];
    Emprunt      emprunts[MAX_EMPRUNTS];

    /* tailles courantes */
    int nombre_livres;
    int nombre_utilisateurs;
    int nombre_emprunts;

    /* prochains identifiants */
    int prochain_id_livre;
    int prochain_id_utilisateur;
    int prochain_id_emprunt;
} Bibliotheque;

/* =========================
 * Prototypes transverses
 * ========================= */

/* --- Initialisation & persistance (fichiers.[ch]) --- */
void initialiser_bibliotheque(Bibliotheque *bib);
int charger_tout(Bibliotheque *bib);
int sauvegarder_tout(Bibliotheque *bib);

/* --- Authentification (authentification.[ch]) --- */
Utilisateur* authentifier(Bibliotheque *bib, const char *email, const char *password);
int          est_admin(Utilisateur *utilisateur);
void         creer_admin_par_defaut(Bibliotheque *bib);

/* --- Gestion des livres (livres.[ch]) --- */
int    ajouter_livre(Bibliotheque *bib,
                     const char *titre, const char *auteur,
                     const char *isbn, const char *categorie, int annee);
int    supprimer_livre(Bibliotheque *bib, int id_livre);
int    modifier_livre(Bibliotheque *bib, int id_livre,
                      const char *titre, const char *auteur,
                      const char *isbn, const char *categorie, int annee);

Livre* rechercher_livre_par_id(Bibliotheque *bib, int id_livre);
Livre* rechercher_livre_par_titre(Bibliotheque *bib, const char *titre);
Livre* rechercher_livre_par_auteur(Bibliotheque *bib, const char *auteur);
Livre* rechercher_livre_par_isbn(Bibliotheque *bib, const char *isbn);

void   afficher_livre(const Livre *livre);
void   afficher_tous_les_livres(Bibliotheque *bib);
void   afficher_livres_par_categorie(Bibliotheque *bib, const char *categorie);

void   afficher_livres_tries_par_titre(Bibliotheque *bib);
void   afficher_livres_tries_par_annee(Bibliotheque *bib);
void   afficher_livres_tries_par_auteur(Bibliotheque *bib);
void   afficher_livres_tries_par_emprunts(Bibliotheque *bib);

/* multi-critères : renvoie le nombre de résultats, remplit resultats[] */
int    rechercher_livres_multi_criteres(Bibliotheque *bib,
                                        const char *titre /* nullable */,
                                        const char *auteur /* nullable */,
                                        const char *categorie /* nullable */,
                                        Livre **resultats, int max_resultats);

/* --- Gestion des utilisateurs (utilisateurs.[ch]) --- */
int   ajouter_utilisateur(Bibliotheque *bib, const char *nom, const char *prenom,
                          const char *id_etudiant, const char *email,
                          const char *password, TypeUtilisateur type);
int   supprimer_utilisateur(Bibliotheque *bib, int id_utilisateur);
int   modifier_utilisateur(Bibliotheque *bib, int id_utilisateur,
                           const char *nom, const char *prenom,
                           const char *id_etudiant, const char *email);

Utilisateur* rechercher_utilisateur_par_id(Bibliotheque *bib, int id);
Utilisateur* rechercher_utilisateur_par_id_etudiant(Bibliotheque *bib, const char *id_etudiant);
Utilisateur* rechercher_utilisateur_par_email(Bibliotheque *bib, const char *email);

void  afficher_utilisateur(const Utilisateur *utilisateur);
void  afficher_tous_les_utilisateurs(Bibliotheque *bib);
void  afficher_utilisateurs_tries(Bibliotheque *bib);

int   modifier_password_utilisateur(Bibliotheque *bib, int id_utilisateur,
                                    const char *ancien_password, const char *nouveau_password);
void  ajouter_penalite(Bibliotheque *bib, int id_utilisateur, double montant);
int   payer_penalites(Bibliotheque *bib, int id_utilisateur, double montant);

/* --- Gestion des emprunts (emprunts.[ch]) --- */
int      enregistrer_emprunt(Bibliotheque *bib, int id_livre, int id_utilisateur);
int      enregistrer_retour(Bibliotheque *bib, int id_livre, int id_utilisateur);
Emprunt* rechercher_emprunt_actif(Bibliotheque *bib, int id_livre, int id_utilisateur);
int      peut_emprunter(Bibliotheque *bib, int id_utilisateur);
void     verifier_retards(Bibliotheque *bib);
void     afficher_emprunts_actifs(Bibliotheque *bib);
void     afficher_tous_les_emprunts(Bibliotheque *bib);
void     afficher_historique_utilisateur(Bibliotheque *bib, int id_utilisateur);
void     afficher_emprunts_en_retard(Bibliotheque *bib);
int      prolonger_emprunt(Bibliotheque *bib, int id_livre, int id_utilisateur, int jours);

/* --- Statistiques & rapports (statistiques.[ch]) --- */
void afficher_top_livres(Bibliotheque *bib, int n);
void afficher_top_utilisateurs(Bibliotheque *bib, int n);
void afficher_statistiques_generales(Bibliotheque *bib);
void afficher_statistiques_categories(Bibliotheque *bib);
void generer_rapport_texte(Bibliotheque *bib, const char *nom_fichier);
void generer_rapport_html(Bibliotheque *bib, const char *nom_fichier);

/* --- Initialisation & persistance (fichiers.[ch]) --- */
void initialiser_bibliotheque(Bibliotheque *bib);



#ifdef __cplusplus
}
#endif

#endif /* STRUCTURES_H */