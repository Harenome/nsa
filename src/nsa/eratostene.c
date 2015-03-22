/**
 * \file eratostene.c
 * \brief Functions about the Sieve of Erathostenes
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

#include "nsa/eratostene.h"
#include "nsa/tools.h"

static MPI_Datatype message_datatype;

static void create_job_datatype(void){
    struct job todo;
    MPI_Aint aint[2] = { 0 , (void*) &todo.multiplier - (void*) &todo };
    const MPI_Datatype previous_types[2] = { MPI_UNSIGNED_LONG_LONG , MPI_UNSIGNED };
    const int blocklengths[2] = { 1 , 1 };
    MPI_Type_create_struct(2, blocklengths, aint, previous_types, &message_datatype);
    MPI_Type_commit(&message_datatype);
}

void init_eratostene(void){
    create_job_datatype();
}

unsigned char *eratostene_base(unsigned long long n)
{
    unsigned long long taille = (n+15)/(2*8), i;

    unsigned char *tab = malloc( taille );
    if(tab == NULL)
    {
        return(NULL);
    }

    for(i=0; i<taille ;i++)
        tab[i] = 0;
    MET1(tab,0);    // 1 n'est pas premier :)

    /* le tableau tab contient des booléens pour les numéros 1, 3, 5, 7, 9, ...
     * dont les valeurs sont 1 pour les nombres non-premiers,
     * 0 pour les nombres premiers */

    // parcourt tous les entiers du tableau jusqu'à racine(n)
    unsigned long long ractaille = sqrt(n/2);
    for(i=1; i<=ractaille; i++)
    {
        if( PREMIER(tab,i) )    // i*2+1 est premier
        {
            // donc on supprime ses multiples : 3i+1, 5i+2, 7i+3, ...
            unsigned int ii;
            for(ii=3*i+1; ii<n/2; ii+=2*i+1 )
            {
                MET1(tab,ii);   // ce nombre n'est pas premier
            }
        }
    }

    return( tab );
}

unsigned char *eratostene_intervalle( unsigned char *tab, const unsigned long long min, const unsigned char *const e_base )
{
    unsigned long long i;
    const unsigned long long max = min+TAILLE_CRIBLE;

#ifdef DEBUG
    printf("\neratostene_intervalle: %llu-%llu\n", min,max );
#endif

    for( i=0 ; i<(TAILLE_CRIBLE>>4) ; i++ )
        tab[i] = 0;

    // enlève tous les multiples des premiers de e_base
    // dans le tableau tab.

    // parcourt les premiers de e_base jusqu'à racine(max) :
    const unsigned long long racmax = sqrt(max)/2;
    for( i=1 ; i<=racmax ; i++ )
    {
        if( PREMIER(e_base,i) )
        {
            unsigned long long j;
            const unsigned long long prem = 2*i+1; // 2i+1 est premier

            // on supprime tous ses multiples compris entre min et max
            // ça commence au premier nombre de la forme (2k+1)*i+1 supérieur ou égal à min/2.
            // et ça s'arrête à taille/2
            const unsigned long long inf = ((min/2+i)/prem)*prem+i;

            //			for( j= (inf==i)?3*i+1:inf-min/2 ; j<(max-min)/2 ; j+=2*i+1 )
            // la borne inf devrait être comme ci-dessus,
            // mais comme le premier morceau fait au moins TAILLE_CRIBLE (garanti par l'appelant)
            // ça ne devrait jamais arriver (inf==i).

            for( j= inf-min/2 ; j<(TAILLE_CRIBLE>>1) ; j+=prem )
            {
                MET1(tab,j);
            }
        }
    }

    return( tab );
}

void eratostene_coordinator(int size, mpz_t x){
    const unsigned int nbtimes = 100;
    mpz_t q, racine2x, racine4x, div, reste;
    mpz_inits(q, racine2x, racine4x, div, reste, NULL);

    /* vérifie que x n'est pas divisible par 2, sinon s'arrête ! */
    if( mpz_cdiv_q_ui( q, x, 2 ) == 0 )	// retourne le reste, stocke le quotient dans q
    {
        printf( "\n" );
        printf( "2\n");
        gmp_printf( "%Zd\n", q );

        mpz_clears(q, racine2x, racine4x, div, reste, NULL);
        return;
    }

    mpz_sqrt( racine2x , x );
    mpz_sqrt( racine4x , racine2x );

    // racine(x) et racine(racine(x)) sont supposés tenir dans un unsigned long long
    // Le premier morceau doit faire au moins TAILLE_CRIBLE...
    const unsigned long long r4xull = (mpz_get_ull(racine4x)<TAILLE_CRIBLE)?TAILLE_CRIBLE:mpz_get_ull(racine4x);

    // cherche les nombres premiers jusqu'à r4xull = racine(racine(x)) avec le crible d'eratostène
    unsigned char *tab_erato = eratostene_base( r4xull );

    unsigned long long current;

    // cheche les diviseurs dans le premier intervalle (1->r4xull)
    for(current=3; current<=r4xull; current+=2)
    {
        // si current est premier
        if( PREMIER(tab_erato,current/2) )
        {
            // essaye de diviser x par current (en mpz)
            mpz_set_ull( div, current );
            mpz_cdiv_qr( q, reste, x, div );
            if( mpz_cmp_ui( reste, 0 ) == 0 )
            {
                // gagné !
                printf( "\n" );
                gmp_printf( "%Zd\n", q );
                gmp_printf( "%Zd\n", div );

                mpz_clears(q, racine2x, racine4x, div, reste, NULL);
                shutdown_all_slaves(size);
                return;
            }
        }
    }

    const unsigned long long r2xull = mpz_get_ull(racine2x);
    int slaves_without_job = size - 1;

    for(int i=1; slaves_without_job && current <= r2xull; i++){
        current = send_job(i, current, nbtimes);
        slaves_without_job--;
    }

    unsigned long long soluce;
    MPI_Status status;

    while(current <= r2xull){
        MPI_Recv(&soluce, 1, MPI_UNSIGNED_LONG_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG,
                MPI_COMM_WORLD, &status);
        switch(status.MPI_TAG){
            case we_found_something:
                mpz_set_ull(div, soluce);
                mpz_cdiv_qr(q, reste, x, div);
                printf( "\n" );
                gmp_printf( "%Zd\n", q );
                gmp_printf( "%Zd\n", div );
                slaves_without_job++;

                soluce = wait_for_any_other_response(size - 1, &slaves_without_job);
                if(soluce)
                    fprintf(stderr, "erreur, reçu %lld\n", soluce);

                shutdown_all_slaves(size);
                mpz_clears(q, racine2x, racine4x, div, reste, NULL);
                return;
            case my_life_for_the_master:
                current = send_job(status.MPI_SOURCE, current, nbtimes);
                break;
            default:
                break;
        }
    }

    soluce = wait_for_any_other_response(size - 1, &slaves_without_job);
    if(soluce){
        mpz_set_ull(div, soluce);
        mpz_cdiv_qr(q, reste, x, div);
        printf( "\n" );
        gmp_printf( "%Zd\n", q );
        gmp_printf( "%Zd\n", div );
    }
    else
        printf( "\npas de solution (c'est un nombre premier) !\n" );

    shutdown_all_slaves(size);
    mpz_clears(q, racine2x, racine4x, div, reste, NULL);
}

void eratostene_slaves(mpz_t x){
    struct job todo;
    unsigned long long response;
    enum message_type what_should_i_do;

    mpz_t q, racine2x, racine4x;
    mpz_inits(q, racine2x, racine4x, NULL);

    /* vérifie que x n'est pas divisible par 2, sinon s'arrête ! */
    if( mpz_cdiv_q_ui( q, x, 2 ) == 0 ){ // retourne le reste, stocke le quotient dans q
        mpz_clears(q, racine2x, racine4x, NULL);
        return;
    }

    mpz_sqrt( racine2x , x );
    mpz_sqrt( racine4x , racine2x );

    // racine(x) et racine(racine(x)) sont supposés tenir dans un unsigned long long
    // Le premier morceau doit faire au moins TAILLE_CRIBLE...
    const unsigned long long r4xull = (mpz_get_ull(racine4x)<TAILLE_CRIBLE)?TAILLE_CRIBLE:mpz_get_ull(racine4x);

    // cherche les nombres premiers jusqu'à r4xull = racine(racine(x)) avec le crible d'eratostène
    unsigned char *tab_erato = eratostene_base( r4xull );

    unsigned char *temp = malloc( (TAILLE_CRIBLE>>4) );

    for(;;){
        what_should_i_do = slave_wait_for_jobs(&todo);
        switch(what_should_i_do){
            case slave_shutdown:
                mpz_clears(q, racine2x, racine4x, NULL);
                free(temp);
                free(tab_erato);
                return;
            case here_take_my_gift_son:
                response = yes_my_lord_work_in_progress(todo, x, tab_erato, temp);
                if(response)
                    i_got_it(response);
                break;
            default:
                break;
        }
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    printf("esclave %d demande de job\n", rank);
        MPI_Ssend(NULL, 0, MPI_CHAR, COORDINATOR_ID, my_life_for_the_master,
                MPI_COMM_WORLD);
    printf("esclave %d demande de job recu par master\n", rank);
    }
}

void shutdown_all_slaves(int size){
    if(size < 1)
        return;
    MPI_Status status;
    MPI_Request *req = malloc(((size_t)size-1)*sizeof(MPI_Request));
    for(int i = 1; i < size; i++){
        MPI_Isend(NULL, 0, MPI_CHAR, i, slave_shutdown, MPI_COMM_WORLD, &req[i-1]);
    }
    for(int i = 1; i < size; i++){
        MPI_Wait(&req[i-1], &status);
    }
    free(req);
}

inline void i_got_it(unsigned long long holy_grail){
    printf("FOUND IT %lld\n", holy_grail);
    MPI_Ssend(&holy_grail, 1, MPI_UNSIGNED_LONG_LONG, COORDINATOR_ID, we_found_something,
            MPI_COMM_WORLD);
}

enum message_type slave_wait_for_jobs(struct job *todo){
    MPI_Status status;
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    printf("%d Waiting\n", rank);
    MPI_Recv(todo, 2, message_datatype, COORDINATOR_ID, MPI_ANY_TAG,
            MPI_COMM_WORLD, &status);
    return status.MPI_TAG;
}

unsigned long long yes_my_lord_work_in_progress(struct job todo, mpz_t number,
        const unsigned char *const e_base, unsigned char *temp){
    mpz_t q, div, reste;
    mpz_inits( q, div, reste, NULL);
    unsigned long long position = todo.start;
    printf("TODO %lld, %ld\n", position, todo.multiplier);
    for(unsigned int i = 0; i < todo.multiplier; i++, position += TAILLE_CRIBLE){
        eratostene_intervalle(temp, position, e_base);
        for(unsigned long long j=0 ; j<(TAILLE_CRIBLE>>1) ; j++ )
        {
            // si j correspond à un nombre premier
            if( PREMIER(temp,j) )
            {
                // ce nombre premier est : p= position+j*2
                // essaye de diviser number par p (en mpz)
                mpz_set_ull( div, position+(j<<1) );
                mpz_cdiv_qr( q, reste, number, div );
                if( mpz_cmp_ui( reste, 0 ) == 0 )
                {
                    mpz_clears(q, div, reste, NULL);
                    return position+(j<<1);
                }
            }
        }
    }
    mpz_clears(q, div, reste, NULL);
    return 0;
}

unsigned long long send_job(const int process_id,
        const unsigned long long current,
        const unsigned int times){
    struct job todo = { current, times };
    MPI_Send(&todo, 1, message_datatype, process_id, here_take_my_gift_son,
            MPI_COMM_WORLD);
    return current + times * TAILLE_CRIBLE;
}

unsigned long long wait_for_any_other_response(int number, int *current){
    unsigned long long soluce, retval = 0;
    MPI_Status status;
    for(int i = *current; i<number; i++){
        MPI_Recv(&soluce, 1, MPI_UNSIGNED_LONG_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG,
                MPI_COMM_WORLD, &status);
        if(status.MPI_TAG == we_found_something)
            retval = soluce;
        i++;
        printf("Recu\n");
    }
    return retval;
}