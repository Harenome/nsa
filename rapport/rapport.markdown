% **NSA**: **N**aive **S**emi-prime **A**ccelerator
% Harenome RAZANAJATO <<razanajato@etu.unistra.fr>>;
  Maxime SCHMITT <<maxime.schmitt@etu.unistra.fr>>;
% 4 avril 2015

Utilisation du programme
========================

Le fichier `README.markdown` et le pdf qui en résulte (`README.pdf`) contiennent
les instructions nécessaires pour compiler et éxécuter notre programme.

Implémentation
==============

Notre programme se décompose en deux parties :

1. Un coordinateur
  ~ qui s'occupe de récupérer les messages des esclaves et distribuer le
    ***travail***.
2. Des esclaves
  ~ qui attendent un ***travail*** et l'effectuent.

Un ***travail*** est composé :

1. d'un point de $départ$.
2. d'un $facteur$ multiplicateur (par défaut, $facteur = 10$).

Un ***esclave*** ayant reçu un ***travail*** s'intéressera alors à l'intervalle
suivant :

$$\left[\ départ,\ départ + facteur * taille\_crible\ \right]$$

Pour plus de performances, $taille\_crible$ est un nombre dont la division par
$16$ doit tenir dans le cache L1 de la (ou les) machine(s) cible.

Une fois arrivé aux $\frac{3}{4}$ de l'intervalle considéré, un ***esclave***
demandera un nouveau ***travail*** au ***coordinateur*** afin de combler la
latence dûe aux échanges de messages. Ainsi, dans la plupart des cas, un nouveau
***travail*** sera immédiatement démarré à la fin d'un ***travail*** non
concluant.

#### Exemple

Soit un ***travail*** tel que $départ = 101$ et $facteur = 10$. Alors un
***esclave*** ayant reçu ce ***travail*** s'intéressera à l'intervalle suivant :

$$\left[101,\ 101 + 10 * taille\_crible \right]$$

Un nouveau ***travail*** sera demandé une fois que le sous-intervalle suivant 
sera complété :

$$\left[101,\ 101 + \frac{15 * taille\_cribe}{2} \right]$$

Analyse des résultats
=====================

Les résultats suivants ont été obtenus dans la salle J4. Les étapes que nous
avons suivies pour éxécuter notre programme dans la salle J4 sont détaillées
dans l'[Annexe][].

RSA-80 : $360597895876381766679361$
-----------------------------------

Notre programme trouve bien que
$360597895876381766679361 = 600999999781 * 599996499181$.

![Temps d'éxécution en fonction du nombre de threads pour la clé RSA-80 \label{rsa-80-fig}](rsa-80-graph.pdf)

Threads         Temps (secondes)
-------         ----------------
8               688.429
35              163.066
71              86.453
107             74.987
143             70.151
144             69.673
179             69.253
251             66.756
287             73.110

: Temps d'éxécution en fonction du nombre de threads pour la clé RSA-80 \label{rsa-80-table}

La Figure \ref{rsa-80-fig} et le tableau \ref{rsa-80-table} montrent l'évolution
du temps d'éxécution nécessaire pour trouver une solution en utilisant l'option
`-m 100` sur les machines de la salle J4 (équipées de Core i7-2600).

```bash
$ mpirun --hostfile /mpi/hosts -n $N_THREADS /mpi/nsa_bin 360597895876381766679361 -m 100
```

Nous avons également testé notre programme sur 8 threads sur une seule machine,
ainsi que sur 8 threads distribués sur différentes machines :

```bash
$ mpirun -n 9 /mpi/nsa_bin 360597895876381766679361 -m 100
$ mpirun --hostfile hosts -n 9 /mpi/nsa_bin 360597895876381766679361 -m 100
```

Comme nous l'attendions, nous avons bien observé de meilleures performances
(1312 secondes contre 688 secondes) en distribué : même si les coeurs des
Core i7 disposent de multithreading, les 8 threads ne sont pas réellement
éxécutés en parallèle tandis que lorsque distribués sur des machines différentes,
les 8 threads ont bien tourné sur un vrai coeur.

RSA-90: $725999999974645000000200901$
-------------------------------------

Notre programme a trouvé que
$725999999974645000000200901 = 32999999999599 * 21999999999499$.

\pagebreak

Annexe
======

Après avoir permis le montage nfs de `/mpi` et ajouté les fichiers :

- `/mpi/id_dsa` : la clé privée *dsa*.
- `/mpi/id_dsa.pub` : la clé publique *dsa*.
- `/mpi/scriptMPI` : le script du TP.
- `/mpi/hosts` : la liste des machines à utiliser.
- `/mpi/nsa` : l'éxécutable du programme.

On a éxécuté le script suivant :

`setup_j4`:

```bash
#!/bin/bash

function setup_mpi
{
    if [[ $1 != $2 ]]; then
        echo "Setting up $1..."

        ssh_command="apt-get -y install nfs-common 2>&1 >/dev/null;"
        ssh_command="$ssh_command mkdir -p /mpi;"
        ssh_command="$ssh_command [[ -f '/mpi/id_dsa' ]] || mount $2:/mpi /mpi"
        ssh_command="$ssh_command /mpi/scriptMPI"

        ssh $1 $ssh_command
    fi
}

# seq -w 21 ?!
for i in $(seq 9); do
    setup_mpi pcj40$i $(hostname)
done
for i in $(seq 10 21); do
    setup_mpi pcj4$i $(hostname)
done
```

Le script `/mpi/scriptMPI` correspond plus ou moins au même script que celui
utilisé lors du TP (le script se charge d'installer, si nécessaire,
`libopenmpi-dev`, `openmpi-bin` et `openmpi-doc`, puis de copier les clés *dsa*
depuis `/mpi`).

Une fois les machines de la salle J4 prêtes :

```bash
$ cd /mpi
$ N_THREADS=$(( $(wc -l /mpi/hosts | cut -f 1 -d ' ') * 8 + 1 ))
$ mpirun --hostfile /mpi/hosts -n $N_THREADS /mpi/nsa_bin <nombre>
```

Pour le nombre de threads : les Core i7 de la salle J4 ont 4 coeurs en
multithread (donc 8 threads) et on peut rajouter un thread car le
***coordinateur*** ne calcule pas.
