% NSA
% Harenome RAZANAJATO <<razanajato@etu.unistra.fr>>;
  Maxime SCHMITT <<maxime.schmitt@etu.unistra.fr>>;
% 4 avril 2015

Implémentation
==============

Notre programme se décompose en deux parties,

- un coordinateur qui s'occupe de récupérer les messages des esclaves et
  distribuer le travail
- des esclaves qui attendent un travail et l'effectue

Un ``travail`` est composé

1. d'un point de départ
2. d'un facteur multiplicateur

Par exemple, si un travail est donné avec un départ de 101 et un facteur
multiplicateur de 10, l'esclave vas effectuer le travail entre
``[101, 101 + facteur * TAILLE_CRIBLE]`` avec TAILLE_CRIBLE étant un nombre dont la
division par 16 doit tenir dans le cache L1 pour plus de performances.
Par défaut ``facteur`` vaut 10.

Chaque esclave exécutera donc son travail et au trois quart du travail en cours
vas demander au coordinateur un nouveau travail pour combler la latence due aux
échanges de messages. Ainsi dans la plus part des cas à la fin d'un travail
non concluant, un nouveau travail est immédiatement lancé.

Analyse des résultats
=====================

Graphe speedup ordonnée et nombre de cœurs abscisse.
