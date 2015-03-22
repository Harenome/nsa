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

#define COORDINATOR_ID 0
#define TAILLE_CRIBLE 4000000ULL

enum message_type{
    slave_shutdown,         // Indique aux esclaves de terminer
    we_found_something,     // Indique au coordinateur qu'une solution est troouvée.
    my_life_for_the_master, // Indique au coordinateur qu'un esclave demande du travail
    here_take_my_gift_son,  // Indique a un esclave un travail à effectuer
    //whats_up,               // Demande du coordinateur aux esclaves ou ils en sont
};

struct job{
    unsigned long long start;
    unsigned int multiplier;
};

void init_eratostene(void);

/** crible d'ératostène initial, entre 1 et n
 * le tableau renvoyé contient, pour chaque entier impair (1,3,5,7,...),
 * des bits à 0 pour les nombres premiers, 1 pour les autres.
 * les deux macros PREMIER(tab,j) et MET1(tab,j) accèdent aux bits
 * du tableau : PREMIER teste si le nombre en position j est premier,
 * MET1 met un 1 en position j.
 */
unsigned char *eratostene_base( unsigned long long n );

/** crible d'ératostène dans l'intervalle min-max,
 * e_base est le résultat du crible d'ératostène de base
 * max est inférieur au carré du plus grand nombre stocké dans e_base,
 * min est impair,
 * le tableau tab de taille suffisante est alloué par l'appelant
 * tous les multiples des nombres premiers stockés dans e_base vont être
 * éliminés de l'intervalle min-max par cette fonction.
 * renvoie un champ de bits correspondant aux entiers min, min+2, min+4, ...
 * qui vaut 0 si le nombre est premier, 1 sinon
 */
unsigned char *eratostene_intervalle( unsigned char *tab,
        const unsigned long long min, const unsigned char *const e_base );

void eratostene_coordinator(int size, mpz_t x, unsigned int multiplier);

void eratostene_slaves(mpz_t x);

void shutdown_all_slaves(int size);

enum message_type slave_wait_for_jobs(struct job *todo);

void i_got_it(unsigned long long holy_grail);

unsigned long long yes_my_lord_work_in_progress(struct job todo, mpz_t number,
        const unsigned char *const e_base, unsigned char *temp);

unsigned long long send_job(const int process_id,
        const unsigned long long current,
        const unsigned int times);

unsigned long long wait_for_any_other_response(int number, int *current);

#endif // __ERATHOSTENE_H
