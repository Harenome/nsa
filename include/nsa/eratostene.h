/**
 * \file eratostene.h
 * \brief Functions about the Sieve of Erathostenes (header)
 * \author RAZANAJATO RANAIVOARIVONY Harenome
 * \author SCHMITT Maxime
 * \date 2015
 * \copyright WTFPLv2
 */

/* Copyright © 2015
 *      Harenome RAZANAJATO <razanajato@etu.unistra.fr>
 *      Maxime SCHMITT <maxime.schmitt@etu.unistra.fr>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar.
 *
 * See http://www.wtfpl.net/ or read below for more details.
 */

/*        DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *                    Version 2, December 2004
 *
 * Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
 *
 * Everyone is permitted to copy and distribute verbatim or modified
 * copies of this license document, and changing it is allowed as long
 * as the name is changed.
 *
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *  0. You just DO WHAT THE FUCK YOU WANT TO.
 */

#ifndef __ERATHOSTENE_H
#define __ERATHOSTENE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <gmp.h>
#include <mpi.h>

/**
 * \brief The identity of the coordinator process
 */
#define COORDINATOR_ID 0
/**
 * \brief Taille des chunks minimum a distribuer par noeuds
 */
#define TAILLE_CRIBLE 4000000ULL

/**
 * \brief Les messages échangés entre les processus.
 */
enum message_type{
    slave_shutdown,         /**> Indique aux esclaves de terminer */
    we_found_something,     /**> Indique au coordinateur qu'une solution est trouvée. */
    my_life_for_the_master, /**> Indique au coordinateur qu'un esclave demande du travail */
    here_take_my_gift_son,  /**> Indique a un esclave un travail à effectuer */
    hey_listen,             /**> Indique au père qu'un intervalle est calculé */
    //whats_up,               /** Demande du coordinateur aux esclaves ou ils en sont */
};

/**
 * \brief Structure pour donner du travail aux processus qui en demandent.
 */
struct job{
    unsigned long long start;   /**> Donne le point de départ du calcul */
    unsigned int multiplier;    /**> Donne TAILLE_CRIBLE * multiplier à calculer */
};

/**
 * \brief Initialisation pour créer les datatypes pour MPI.
 */
void init_eratostene(void);

/**
 * \brief Crible d'ératostène initial, entre 1 et n
 * \param n Le nombre maximum pour lequel on cherchera les nombres premiers.
 * \return Un tableau contenant les nombres premiers jusqu'à n
 * le tableau renvoyé contient, pour chaque entier impair (1,3,5,7,...),
 * des bits à 0 pour les nombres premiers, 1 pour les autres.
 * les deux macros PREMIER(tab,j) et MET1(tab,j) accèdent aux bits
 * du tableau : PREMIER teste si le nombre en position j est premier,
 * MET1 met un 1 en position j.
 */
unsigned char *eratostene_base( unsigned long long n );

/**
 * \brief Crible d'ératostène dans l'intervalle [min , min+TAILLE_CRIBLE]
 * \param e_base Est le résultat du crible d'Ératostène de base
 * \param tab Est un tableau de taille TAILLE_CRIBLE/(8*2). 8 impairs par char.
 * \pre max est inférieur au carré du plus grand nombre stocké dans e_base
 * \pre min est impair
 * Le tableau tab de taille suffisante est alloué par l'appelant
 * tous les multiples des nombres premiers stockés dans e_base vont être
 * éliminés de l'intervalle min-max par cette fonction.
 * renvoie un champ de bits correspondant aux entiers min, min+2, min+4, ...
 * qui vaut 0 si le nombre est premier, 1 sinon
 */
void eratostene_intervalle(unsigned char *tab,
        const unsigned long long min, const unsigned char * const e_base );

/**
 * \brief Fonction du thread qui donne du travail aux autres threads et termine
 * si une solution est trouvée.
 * \param size Le nombre total de threads crées par MPI.
 * \param x Le nombre dont on cherche la factorisation.
 * \param pretty_print Positionner à 0 pour un affichage minimaliste et autre
 * pour afficher le travail effectuer par chaque thread.
 */
void eratostene_coordinator(const int size, mpz_t x, const unsigned int multiplier,
        const int pretty_print);

/**
 * \brief Fonction du thread qui attend du travail et l'effectue sans discuter.
 * \param x Le nombre dont on cherche la factorisation.
 */
void eratostene_slaves(mpz_t x);

/**
 * \brief Fonction appelé par le coordinateur pour envoyer un signal de fin aux
 * autres processus.
 * \param size Le nombre total de threads crée par MPI
 */
void shutdown_all_slaves(const int size);

/**
 * \brief Attend un message dont la réponse se trouvera dans todo
 * \param todo La structure contenant le job à effectuer si le type de message
 * est here_take_my_gift_son
 * \return Le type de message envoyé par le père.
 * \retval here_take_my_gift_son Dans le cas de la réception d'un job à
 * effectuer.
 * \retval slave_shutdown Dans le cas ou le fils doit finir.
 */
enum message_type slave_wait_for_jobs(struct job * const todo);

/**
 * \brief Attend la réponse à une requête déjà effectuée.
 * \param request La requête MPI
 * \return Le type de message envoyé par le père.
 * \retval here_take_my_gift_son Dans le cas de la réception d'un job à
 * effectuer.
 * \retval slave_shutdown Dans le cas ou le fils doit finir.
 */
enum message_type slave_wait_for_pending_jobs(MPI_Request *request);

/**
 * \brief Fonction pour avertir le coordinateur qu'une réponse à été trouvée.
 * \param holy_grail La valeur première divisant le nombre cherché.
 */
static inline void i_got_it(const unsigned long long holy_grail){
    MPI_Ssend(&holy_grail, 1, MPI_UNSIGNED_LONG_LONG, COORDINATOR_ID, we_found_something,
            MPI_COMM_WORLD);
}

/**
 * \brief Effectue le travail présent dans todo.
 * \param todo Le job à effectuer.
 * \param number Le nombre dont on cherche la factorisation.
 * \param e_base Est le résultat du crible d'Ératostène de base
 * \param temp Est un tableau de taille TAILLE_CRIBLE/(8*2). 8 impairs par char.
 * \param next Le buffer contenant le prochain job demandé par la fonction
 * si *response n'est pas NULL.
 * \param response Si la valeur contenue est non NULL, elle contiens le
 * pointeur vers une requête faite au coordinateur pour un prochain job. Ce
 * pointeur est à free(3) après un appel à MPI_Wait.
 * \return La valeur trouvée comme diviseur.
 * \retval 0 Si aucun nombre premier contenu dans todo n'est un diviseur
 * de number.
 */
unsigned long long yes_my_lord_work_in_progress(struct job todo, mpz_t number,
        const unsigned char * const e_base, unsigned char *temp,
        struct job *next, MPI_Request **response);

/**
 * \brief Le coordinateur envoyant un job à un de ses esclaves.
 * \param process_id Le numéro de processus auquel envoyer un job.
 * \param todo Le buffer a transférer au processus.
 */
void send_job(const int process_id, struct job *todo);

/**
 * \brief Attend response nombre de réponses possibles des esclaves.
 * \param responses Un pointeur vers le nombre de réponses à attendre.
 * \return La valeur trouvée comme diviseur.
 * \retval 0 Si aucune réponse ne contient de diviseur premier.
 */
unsigned long long wait_for_any_other_response(unsigned int *responses);

#endif // __ERATHOSTENE_H
