cmake_minimum_required(VERSION 3.26)
project(test C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

message(STATUS "GTK3 found: ${GTK3_VERSION}")

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
        authentification.c
        ui.c)

target_include_directories(test PRIVATE ${GTK3_INCLUDE_DIRS})
foreach(_flag ${GTK3_CFLAGS_OTHER})
    target_compile_options(test PRIVATE ${_flag})
endforeach()
target_link_libraries(test PRIVATE ${GTK3_LIBRARIES})

