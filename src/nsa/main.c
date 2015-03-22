/**
 * \file main.c
 * \brief Main.
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

#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>
#include <mpi.h>
#include <gmp.h>
#include <unistd.h>

#include "nsa/version.h"
#include "nsa/eratostene.h"

static inline void fprintf_help(FILE *outstream, const char* const executable_path){
    fprintf(outstream,
            "\tNaive Semi-prime Accelerator\n"
            "Usage: %s [<options>] <number>\n"
            "Options:\n"
            "\t-h : print this help and exit\n"
            "\t-v : print version and exit\n",
            executable_path);
}

/**
 * \brief Main function.
 * \param argc Command line argument count.
 * \param argv Command line arguments.
 * \retval EXIT_SUCCESS on success.
 * \retval EX_USAGE on user misbehaviour.
 */
int main (int argc, char **argv)
{
    int rank, size, opt;
    while((opt=getopt(argc, argv, "hv")) != -1){
        switch(opt){
            case 'h':
                fprintf_help(stdout, argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'v':
                fprintf_version(stdout, argv[0]);
                exit(EXIT_SUCCESS);
                break;
            default:
                fprintf_help(stderr, argv[0]);
                exit(EX_USAGE);
                break;
        }
    }
    MPI_Init(&argc, &argv);
    --argc; argv++;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (argc != 1)
    {
        fprintf_help(stderr, argv[-1]);
        MPI_Finalize();
        exit (EX_USAGE);
    }

    /* lecture param en gmp */
    mpz_t x;
    mpz_init_set_str(x, argv[0], 10);

    init_eratostene();

    if(rank == COORDINATOR_ID)
        eratostene_coordinator(size, x);
    else
        eratostene_slaves( x );

    mpz_clear( x );

    MPI_Finalize();
    exit (EXIT_SUCCESS);
}
