Naive Semi-prime Accelerator
============================

Projet de l'UE « Parallélisme », Master 1 RISE,

[Université de Strasbourg][], [UFR Mathématique-Informatique][] 2014-2015.

Que fait ce programme ?
-----------------------

Ce programme prends un nombre semi-premier en entrée, c'est à dire qu'il
est un produit entre deux nombres premiers, puis trouve les deux nombres
premiers qui le factorise à moins qu'il soit lui même premier.

Si le nombre donné n'est pas semi-premier vous pouvez réitérer sur le
résultat jusqu'à l'obtention de sa factorisation en nombre premier.

La limite due à l'implémentation est que la racine du nombre recherché soit
inférieur au maximum contenu dans un entier 64bits non signé.

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

Installer
---------

```bash
$ sudo make install
```

Lancer nsa
----------

Si vous avez sauté la phase d'installation, le binaire se trouve dans le dossier
``<Dossier_de_build>/bin``.

```bash
$ mpirun <options_mpirun> nsa <options_nsa> <nombre_à_factoriser>
```

### Options nsa

- ``-h`` affiche l'aide et quitte.
- ``-v`` affiche la version et quitte.
- ``-m`` change le facteur multiplicatif du nombre d'intervalles à distribuer
  pour chaque job.
- ``-p`` affiche le travail donné pour chaque threads.

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
