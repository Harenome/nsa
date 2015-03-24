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
            unsigned long long int ii;
            for(ii=3*i+1; ii<n/2; ii+=2*i+1 )
            {
                MET1(tab,ii);   // ce nombre n'est pas premier
            }
        }
    }

    return( tab );
}

unsigned char *eratostene_intervalle( unsigned char *tab,
        const unsigned long long min, const unsigned char *const e_base ){
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

void eratostene_coordinator(const int size, mpz_t x, const unsigned int multiplier,
        const int pretty_print){
    mpz_t q, racine2x, racine4x, div, reste;
    mpz_inits(q, racine2x, racine4x, div, reste, NULL);
    double time_begin, time_found;
    unsigned int jobs_distributed = (unsigned int) size - 1;
    struct job *send_buffers = malloc(jobs_distributed * sizeof(struct job));

    for(int i = 0; i < size-1; i++)
        send_buffers[i].multiplier = multiplier;

    time_begin = MPI_Wtime();
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
                time_found = MPI_Wtime();
                // gagné !
                printf( "\n" );
                gmp_printf( "%Zd\n", q );
                gmp_printf( "%Zd\n", div );
                printf("Temps: %.3fs\n", time_found - time_begin);

                mpz_clears(q, racine2x, racine4x, div, reste, NULL);
                wait_for_any_other_response(&jobs_distributed);
                shutdown_all_slaves(size);
                return;
            }
        }
    }

    const unsigned long long r2xull = mpz_get_ull(racine2x);

    unsigned long long soluce;
    MPI_Status status;

    while(current <= r2xull){
        MPI_Recv(&soluce, 1, MPI_UNSIGNED_LONG_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG,
                MPI_COMM_WORLD, &status);
        switch(status.MPI_TAG){
            case we_found_something:
                jobs_distributed--;
                time_found = MPI_Wtime();
                mpz_set_ull(div, soluce);
                mpz_cdiv_qr(q, reste, x, div);
                printf( "\n" );
                gmp_printf( "%Zd\n", q );
                gmp_printf( "%Zd\n", div );
                printf("Temps: %.3fs\n", time_found - time_begin);

                if(wait_for_any_other_response(&jobs_distributed))
                    wait_for_any_other_response(&jobs_distributed);

                shutdown_all_slaves(size);
                mpz_clears(q, racine2x, racine4x, div, reste, NULL);
                return;
            case my_life_for_the_master:
                send_buffers[status.MPI_SOURCE - 1].start = current;
                send_job(status.MPI_SOURCE, &send_buffers[status.MPI_SOURCE - 1]);
                current += multiplier * TAILLE_CRIBLE;
                jobs_distributed++;
                if(pretty_print){
                    for(int i = 0; i < size - status.MPI_SOURCE; i++)
                        printf("\x1B[1F");
                    printf("\r ** Thread %d: %lld-%lld **", status.MPI_SOURCE, current, current + multiplier*TAILLE_CRIBLE);
                    for(int i = 0; i < size - status.MPI_SOURCE; i++)
                        printf("\n");
                }
                printf("\r ** %lld / %lld (%.2lf%%) **", current, r2xull,
                        (double) current / (double)r2xull * 100.);
                fflush(stdout);
                break;
            case hey_listen:
                jobs_distributed--;
            default:
                break;
        }
    }

    soluce = wait_for_any_other_response(&jobs_distributed);
    if(soluce){
        time_found = MPI_Wtime();
        mpz_set_ull(div, soluce);
        mpz_cdiv_qr(q, reste, x, div);
        printf( "\n" );
        gmp_printf( "%Zd\n", q );
        gmp_printf( "%Zd\n", div );
        printf("Temps: %.3fs\n", time_found - time_begin);
        if(wait_for_any_other_response(&jobs_distributed))
            wait_for_any_other_response(&jobs_distributed);
    }
    else{
        time_found = MPI_Wtime();
        printf( "\npas de solution (c'est un nombre premier) !\n" );
        printf("Temps: %.3fs\n", time_found - time_begin);
    }

    shutdown_all_slaves(size);
    mpz_clears(q, racine2x, racine4x, div, reste, NULL);
    free(send_buffers);
}

void eratostene_slaves(mpz_t x){
    struct job todo;
    unsigned long long response = 0;
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
    MPI_Request *next_response = NULL;

    for(;;){
        if(next_response == NULL)
            what_should_i_do = slave_wait_for_jobs(&todo);
        else{
            what_should_i_do = slave_wait_for_pending_jobs(next_response);
            free(next_response);
            next_response = NULL;
        }
        switch(what_should_i_do){
            case slave_shutdown:
                mpz_clears(q, racine2x, racine4x, NULL);
                free(temp);
                free(tab_erato);
                return;
            case here_take_my_gift_son:
                response = yes_my_lord_work_in_progress(todo, x, tab_erato,
                        temp, &todo, &next_response);
                if(response)
                    i_got_it(response);
                break;
            default:
                break;
        }
    }
}

void shutdown_all_slaves(const int size){
    if(size < 1)
        return;
    MPI_Request *req = malloc(((size_t)size-1)*sizeof(MPI_Request));
    for(int i = 1; i < size; i++){
        MPI_Isend(NULL, 0, MPI_CHAR, i, slave_shutdown, MPI_COMM_WORLD, &req[i-1]);
    }
    MPI_Waitall(size-1, req, MPI_STATUSES_IGNORE);
    free(req);
}

enum message_type slave_wait_for_jobs(struct job * const todo){
    MPI_Request request;
    MPI_Isend(NULL, 0, MPI_CHAR, COORDINATOR_ID, my_life_for_the_master,
            MPI_COMM_WORLD, &request);
    MPI_Request_free(&request);
    MPI_Status status;
    MPI_Recv(todo, 2, message_datatype, COORDINATOR_ID, MPI_ANY_TAG,
            MPI_COMM_WORLD, &status);
    return status.MPI_TAG;
}

enum message_type slave_wait_for_pending_jobs(MPI_Request *request){
    MPI_Status status;
    MPI_Wait(request, &status);
    return status.MPI_TAG;
}

unsigned long long yes_my_lord_work_in_progress(const struct job todo, mpz_t number,
        const unsigned char * const e_base, unsigned char * const temp,
        struct job *next, MPI_Request **response){
    MPI_Request request;
    mpz_t q, div, reste;
    mpz_inits( q, div, reste, NULL);
    unsigned long long position = todo.start;
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
        if(i == todo.multiplier * 3 / 4){
            MPI_Isend(NULL, 0, MPI_CHAR, COORDINATOR_ID, my_life_for_the_master,
                    MPI_COMM_WORLD, &request);
            MPI_Request_free(&request);
            *response = malloc(sizeof(MPI_Request));
            MPI_Irecv(next, 1, message_datatype, COORDINATOR_ID, MPI_ANY_TAG,
                    MPI_COMM_WORLD, *response);
        }
    }
    MPI_Isend(NULL, 0, MPI_CHAR, COORDINATOR_ID, hey_listen,
            MPI_COMM_WORLD, &request);
    MPI_Request_free(&request);
    mpz_clears(q, div, reste, NULL);
    return 0;
}

void send_job(const int process_id, struct job *todo){
    MPI_Request request;
    MPI_Isend(todo, 1, message_datatype, process_id, here_take_my_gift_son,
            MPI_COMM_WORLD, &request);
    MPI_Request_free(&request);
}

unsigned long long wait_for_any_other_response(unsigned int *jobs){
    unsigned long long soluce;
    MPI_Status status;
    while(*jobs != 0){
        MPI_Recv(&soluce, 1, MPI_UNSIGNED_LONG_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG,
                MPI_COMM_WORLD, &status);
        (*jobs)--;
        if(status.MPI_TAG == we_found_something)
            return soluce;
    }
    return 0;
}
