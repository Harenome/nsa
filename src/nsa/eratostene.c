#include "nsa/eratostene.h"
#include "nsa/tools.h"

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
    MET1(tab,0);    // 1 n'est pas premier :)

    /* le tableau tab contient des booléens pour les numéros 1, 3, 5, 7, 9, ...
     * dont les valeurs sont 1 pour les nombres non-premiers,
     * 0 pour les nombres premiers */

    // parcourt tous les entiers du tableau jusqu'à racine(n)
    unsigned long long ractaille = sqrt(n/2);
    for( i=1 ; i<=ractaille ; i++ )
    {
        if( PREMIER(tab,i) )    // i*2+1 est premier
        {
            // donc on supprime ses multiples : 3i+1, 5i+2, 7i+3, ...
            unsigned int ii;
            for( ii=3*i+1 ; ii<n/2 ; ii+=2*i+1 )
            {
                MET1(tab,ii);   // ce nombre n'est pas premier
            }
        }
    }

    return( tab );
}
