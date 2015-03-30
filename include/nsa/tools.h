#ifndef __NSA_TOOLS_H
#define __NSA_TOOLS_H

#include <gmp.h>

/**
 * \brief Test si le nombre j est premier dans le tableau
 *
 * Vrai si l'entrée j du tableau correspond à un nombre premier
 * (2j+1 est premier)
 */
#define PREMIER(tab,j) (((tab[(j)>>3])&(1<<((j)&7)))==0)

/**
 * \brief Met 1 dans le tableau en position j (bitwise)
 */
#define MET1(tab,j) (tab[(j)>>3] |= (1<<((j)&7)))

/**
 * \brief Fonctions de conversion ULL <-> mpz_t
 *
 * Elles n'existent pas par défaut dans gmp :(
 * source: stackoverflow.
 */
static inline void mpz_set_ull(mpz_t n, unsigned long long ull)
{
    mpz_set_ui(n, (unsigned int)(ull >> 32)); /* n = (unsigned int)(ull >> 32) */
    mpz_mul_2exp(n, n, 32);                   /* n <<= 32 */
    mpz_add_ui(n, n, (unsigned int)ull);      /* n += (unsigned int)ull */
}

/**
 * \brief Fonctions de conversion mpz_t <-> ULL
 *
 * Elles n'existent pas par défaut dans gmp :(
 * source: stackoverflow.
 */
static inline unsigned long long mpz_get_ull(mpz_t n)
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

#endif // __NSA_TOOLS_H
