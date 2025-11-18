cmake_minimum_required(VERSION 3.26)
project(test C)

set(CMAKE_C_STANDARD 11)

add_executable(test main.c
        structure.h
        livres.h
        livres.c
        utilisateurs.h
        utilisateurs.c
        emprunts.h
        emprunts.c
        fichiers.h
        fichiers.c
        statistiques.h
        statistiques.c
        authentification.h
        authentification.c)
