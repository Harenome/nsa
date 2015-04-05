Naive Semi-prime Accelerator
============================

Projet de l'UE « Parallélisme », Master 1 RISE,

[Université de Strasbourg][], [UFR Mathématique-Informatique][] 2014-2015.

Que fait ce programme ?
-----------------------

Ce programme prend un nombre semi-premier en entrée, c'est à dire un nombre qui
est le produit de deux nombres premiers, puis trouve ces deux nombres premiers
(sauf si le nombre fourni est lui même premier).

Dans le cas où le nombre fourni n'est pas semi-premier, il est possible
d'obtenir sa factorisation en nombre premiers en utilisant le programme sur les
résultats obtenus.

Les détails d'implémentation imposent la limite suivante : la racine carré du
nombre fourni doit être inférieure au maximum des entiers 64 bits non signés.

Prérequis
---------

### Librairies et outils

- [CMake][]
- [GMP][]
- [MPI][] (Votre implémentation préférée)

Compiler nsa
------------

```bash
$ mkdir build && cd build
$ cmake ..
$ make
```

Sans `cmake` :

```bash
$ ./configure
$ make
```

Note : Le script `configure` n'a pas été généré avec les autotools, il se
contente de générer des fichiers nécéssaires à la compilation et ne vérifie pas
la présence ou la version des bibliothèques et outils...

### Documentation

#### Prérequis

- [Doxygen][]

#### Optionnel

- [Graphviz][] Pour les graphes dans la documentation.

#### Générer la documentation

```bash
$ make doc
```

Installer
---------

```bash
$ sudo make install
```

Lancer nsa
----------

Si vous avez sauté la phase d'installation, le binaire se trouve dans le dossier
`<Dossier_de_build>/bin` (si compilé avec `cmake`) ou `bin` (si compilé avec
le `Makefile` à la racine).

```bash
$ mpirun <options_mpirun> nsa <options_nsa> <nombre_à_factoriser>
```

### Options nsa

- `-h` affiche l'aide et quitte.
- `-v` affiche la version et quitte.
- `-m` change le facteur multiplicatif du nombre d'intervalles à distribuer
  pour chaque job.
- `-p` affiche le travail donné pour chaque threads.


License
-------
Copyright © 2015 SCHMITT Maxime, RAZANAJATO RANAIVOARIVONY Harenome

Ce projet est libre. Vous pouvez le redistribuer ou le modifier selon les termes
de la license « Do What The Fuck You Want To Public License », Version 2, comme
publiée par Sam Hocevar. Pour de plus amples informations, veuillez vous référer
au fichier COPYING, ou bien http://www.wtfpl.net/.

[Université de Strasbourg]: https://www.unistra.fr
[UFR Mathématique-Informatique]: https://mathinfo.unistra.fr/
[CMake]: http://www.cmake.org/
[GMP]: https://gmplib.org/
[MPI]: http://www.open-mpi.org/
[Doxygen]: http://www.doxygen.org/
[Graphviz]: http://graphviz.org/
