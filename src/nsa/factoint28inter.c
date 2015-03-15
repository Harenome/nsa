/** recherche des facteurs d'un nombre semi-premier < 128 bits **/

/* Copyright © 2015 Vincent Loechner <loechner@unistra.fr>
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <gmp.h>
#include <sys/time.h>

// note générale :
// j'utilise des unsigned long long qui sont en 64 bits même sur les archi 32 bits.
// (les "unsigned long" sont 32 bits sur les archis 32)

//décommenter pour avoir un peu de debug pendant l'exécution
//(affiche les intervalles et les diviseurs testés)
//#define DEBUG

//mesure du temps
#define DIFFTEMPS(a,b) (((b).tv_sec - (a).tv_sec) + ((b).tv_usec - (a).tv_usec)/1000000.)

//4M valeurs dans chaque tableau crible, ça fait un tableau de 250ko
//(attention: doit être pair, et multiple de 16 c'est mieux)
#define TAILLE_CRIBLE 4000000

//vrai si l'entrée j du tableau correspond à un nombre premier (2j+1 est premier)
#define PREMIER(tab,j) (((tab[(j)>>3])&(1<<((j)&7)))==0)
//met 1 dans le tableau en position j (bitwise)
#define MET1(tab,j) (tab[(j)>>3] |= (1<<((j)&7)))


/** fonctions de conversion ULL <-> mpz_t
 * elles n'existent pas par défaut dans gmp :(
 * source: stackoverflow.
 */
void mpz_set_ull(mpz_t n, unsigned long long ull)
{
    mpz_set_ui(n, (unsigned int)(ull >> 32)); /* n = (unsigned int)(ull >> 32) */
    mpz_mul_2exp(n, n, 32);                   /* n <<= 32 */
    mpz_add_ui(n, n, (unsigned int)ull);      /* n += (unsigned int)ull */
}

unsigned long long mpz_get_ull(mpz_t n)
{
    unsigned int lo, hi;
    mpz_t tmp;

    mpz_init( tmp );
    mpz_mod_2exp( tmp, n, 64 );   /* tmp = (lower 64 bits of n) */

    lo = mpz_get_ui( tmp );       /* lo = tmp & 0xffffffff */ 
    mpz_div_2exp( tmp, tmp, 32 ); /* tmp >>= 32 */
    hi = mpz_get_ui( tmp );       /* hi = tmp & 0xffffffff */

    mpz_clear( tmp );

    return (((unsigned long long)hi) << 32) + lo;
}

/** crible d'ératostène initial, entre 1 et n
 * le tableau renvoyé contient, pour chaque entier impair (1,3,5,7,...),
 * des bits à 0 pour les nombres premiers, 1 pour les autres.
 * les deux macros PREMIER(tab,j) et MET1(tab,j) accèdent aux bits
 * du tableau : PREMIER teste si le nombre en position j est premier,
 * MET1 met un 1 en position j.
 */
unsigned char *eratostene_base( unsigned long long n )
{
    unsigned long long taille = (n+15)/(2*8), i;

    unsigned char *tab = malloc( taille );
    if( tab == NULL )
    {
        return( NULL );
    }

    for( i=0 ; i<taille ; i++ )
        tab[i] = 0;
    MET1(tab,0);	// 1 n'est pas premier :)

    /* le tableau tab contient des booléens pour les numéros 1, 3, 5, 7, 9, ...
     * dont les valeurs sont 1 pour les nombres non-premiers,
     * 0 pour les nombres premiers */

    // parcourt tous les entiers du tableau jusqu'à racine(n)
    unsigned long long ractaille = sqrt(n/2);
    for( i=1 ; i<=ractaille ; i++ )
    {
        if( PREMIER(tab,i) )	// i*2+1 est premier
        {
            // donc on supprime ses multiples : 3i+1, 5i+2, 7i+3, ...
            unsigned int ii;
            for( ii=3*i+1 ; ii<n/2 ; ii+=2*i+1 )
            {
                MET1(tab,ii);	// ce nombre n'est pas premier
            }
        }
    }

    return( tab );
}

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

/**********************************************************************/
int main(int argc, char ** argv)
{
    struct timeval tv_beg, tv_erato, tv_end;

    if( argc != 2 )
    {
        fprintf( stderr, "Usage: %s <number>\n", argv[0] );
        MPI_Finalize();
        exit( 1 );
    }

    /* lecture param en gmp */
    mpz_t x, q;
    mpz_init( q );
    mpz_init_set_str( x, argv[1], 10 );

    // mesure du temps
    gettimeofday( &tv_beg, NULL);

    /* vérifie que x n'est pas divisible par 2, sinon s'arrête ! */
    if( mpz_cdiv_q_ui( q, x, 2 ) == 0 )	// retourne le reste, stocke le quotient dans q
    {
        printf( "\n" );
        printf( "2\n");
        gmp_printf( "%Zd\n", q );

        mpz_clears( x, q, NULL );
        exit( 0 );
    }

    mpz_t racine2x, racine4x;
    mpz_inits( racine2x, racine4x, NULL );

    mpz_sqrt( racine2x , x );
    mpz_sqrt( racine4x , racine2x );
    // racine(x) et racine(racine(x)) sont supposés tenir dans un unsigned long long
    // Le premier morceau doit faire au moins TAILLE_CRIBLE...
    const unsigned long long r4xull = (mpz_get_ull(racine4x)<TAILLE_CRIBLE)?TAILLE_CRIBLE:mpz_get_ull(racine4x);

    // cherche les nombres premiers jusqu'à r4xull = racine(racine(x)) avec le crible d'eratostène
    unsigned char *taberato = eratostene_base( r4xull );

    // temps intermédiaire
    gettimeofday( &tv_erato, NULL);

    unsigned long long i;
    mpz_t div, reste;
    mpz_inits( div, reste, NULL );

    // cheche les diviseurs dans le premier intervalle (1->r4xull)
    for( i=3 ; i<=r4xull ; i+=2 )
    {
        // si i est premier
        if( PREMIER(taberato,i/2) )
        {
            // essaye de diviser x par i (en mpz)
            mpz_set_ull( div, i );
            mpz_cdiv_qr( q, reste, x, div );
            if( mpz_cmp_ui( reste, 0 ) == 0 )
            {
                // gagné !
                printf( "\n" );
                gmp_printf( "%Zd\n", q );
                gmp_printf( "%Zd\n", div );
                break;
            }
        }
    }

    unsigned long long position=i;	// impair supérieur à r4xull
    const unsigned long long r2xull = mpz_get_ull(racine2x);

    // affichage de la progression
    char fmt[100];
    sprintf(fmt,"\r  * %%0%dllu/%%llu *", (int)log10(r2xull)+1);

    /* parcourt les intervalles suivants successivement (r4xull -> r2xull) */
    if( i>r4xull )
    {
        unsigned char *tabsuite = malloc( (TAILLE_CRIBLE>>4) );
        if( tabsuite==NULL )
            exit( 1 );	// plouf

        while( position <= r2xull )
        {
            unsigned long long j;
            printf(fmt,position,r2xull);fflush(stdout);

            // tabsuite = nombres premiers entre position et position+TAILLE_CRIBLE
            eratostene_intervalle( tabsuite, position, taberato );

            for( j=0 ; j<(TAILLE_CRIBLE>>1) ; j++ )
            {
                // si j correspond à un nombre premier
                if( PREMIER(tabsuite,j) )
                {
#ifdef DEBUG
                    printf("\ndiv=%llu\n", position+(j<<1));
#endif
                    // ce nombre premier est : p= position+j*2
                    // essaye de diviser x par p (en mpz)
                    mpz_set_ull( div, position+(j<<1) );
                    mpz_cdiv_qr( q, reste, x, div );
                    if( mpz_cmp_ui( reste, 0 ) == 0 )
                    {
                        // gagné !
                        break;
                    }
                }
            }

            if( j<(TAILLE_CRIBLE>>1) )
                break;	// on a trouvé
            position = position+TAILLE_CRIBLE;
        }
        if( position <= r2xull )
        {
            printf( "\n" );
            gmp_printf( "%Zd\n", q );
            gmp_printf( "%Zd\n", div );
        }
        else
            printf( "\npas de solution (c'est un nombre premier) !\n" );

        free( tabsuite );
    }
    gettimeofday( &tv_end, NULL);

    printf( "Eratostène base : %lfs. Eratostène intervalles + divisions : %lfs\n",
            DIFFTEMPS(tv_beg,tv_erato), DIFFTEMPS(tv_erato,tv_end) );

    free( taberato );
    mpz_clears( div, reste, q, x, racine2x, racine4x, NULL );
    return 0;
}
