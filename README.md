# s6-sapotache-2824 {#mainpage}

Team : Into the while

- PARPAITE Thibault
- SARRABAYROUSE Alexis
- PONCET Clemence
- CALANDRA Josephine

**Encadrant :** Pascal Desbarats pascal.desbarats@labri.fr

ENSEIRB-MATMECA
Projet réalisé en fevrier-mars-avril-mai 2017 dans le cadre de l'UE Projets de première année d'informatique.
L'objectif de ce projet est d'implémenter le jeu Saboteur avec une partie partie "parsing", un paradigme clients / serveur, ainsi que différentes stratégies de jeu.

~~~
Trello : https://trello.com/b/oty94QpK/projet-sapotache
Sharelatex : https://fr.sharelatex.com/project/58b32f2745b9839a55ffe308
~~~

## Table des matières
<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [s6-sapotache-2824](#s6-sapotache-2824)
	- [Table des matières](#table-des-matieres)
	- [Compilation](#compilation)
	- [Documentation](#documentation)
	- [Exportation README Markdown --> HTML](#exportation-readme-markdown-html)
		- [Pré-requis](#pr-requis)
		- [Utilisation](#utilisation)
	- [Conventions de codage](#conventions-de-codage)
		- [Indentation](#indentation)
		- [Langue](#langue)
		- [Nommage](#nommage)
		- [Blocs](#blocs)
		- [Commentaires](#commentaires)
		- [Les espaces](#les-espaces)
		- [Les fonctions locales](#les-fonctions-locales)

<!-- /TOC -->


## Compilation


Créer un répertoire build à la racine et se placer dedans
~~~
$ mkdir build
$ cd build
~~~

Générer les fichiers Makefile
~~~
$ cmake ..
~~~

Lancer la compilation
~~~
$ make install
~~~

Pour lancer les tests
~~~
$ make test
~~~


## Documentation


Se placer dans le répertoire build.
~~~
$ cd build
~~~

Générer la documentation (~ 30 s)
~~~
$ make doc
~~~

Pour la visualiser, l'ouvrir avec un navigateur web. Par exemple
~~~
$ firefox ./doc/html/index.html
~~~

## Exportation README Markdown --> HTML

### Pré-requis

- pip
~~~{shell}
$ sudo apt-get install python-pip python-dev build-essential
$ sudo pip install --upgrade pip
$ sudo pip install --upgrade virtualenv
~~~

- grip
~~~{shell}
$ sudo pip install grip
~~~

### Utilisation

~~~{shell}
$ grip README.md --export README.html
$ firefox README.html
~~~

## Conventions de codage
### Indentation
4 espaces, pas de tabulation

Pour configurer Emacs pour utiliser 4 espaces pour indenter, il faut ajouter les lignes suivantes au fichiers `~/.emacs`:
~~~{lisp}
(setq-default indent-tabs-mode nil)
(setq-default c-basic-offset 4)
~~~


### Langue
- Commentaires : **anglais**
- Code : **anglais**

### Nommage

- **Macros** :
 - Tout en majuscule
 - Mots séparés par un underscore
 - Exemple : `MY_MACRO`


- **Constantes** :
 - Tout en majuscule
 - Mots séparés par un underscore
 - Exemple : `MY_CONST`


- **Variables** :
 - En minuscule
 - Les mots séparés par un underscore
   - **Variables locales** : `my_variable`
   - **Variables statiques** : `S_my_variable_static`
   - **Variables globales** : `G_my_variable_globale`
   - **Tableau** :
     - Ajouter un "a" à la fin
     - Exemple : `my_variable_tab_a`
   - **Pointeur** :
     - Ajouter un "p" à la fin
     - Exemple : `my_variable_p`


- **Fonctions** :
 - Nom du module en CamelCase lower
 - Nom de la fonction en minuscule
 - les mots séparés par un Underscore
 - Exemple : `circularQueue_push()`
 - **Contructeurs** `module_new(...)`


- **Structures** :
 - Ajouter un "s" à la fin
 - Exemple : `my_structure_s`


- **Abstraction par pointeur**
 - Ajouter un "t" à la fin
 - Pointeur sur structure (type abstrait) : my_structure_t


### Blocs
- **Fonctions** :
~~~{c}
int main(int argc, char **argv) {
    ...
}
~~~


- **Les autres blocs** :
~~~{c}
if (i < n) {
    ...
    ...
} else {
    ...
}

struct ma_structure_s {
    ...
};
~~~


  - Blocs d'une ligne sans accolade autorisés
~~~{c}
if (t[min] > t[i])
    min = i;
return min;
~~~

### Commentaires
- Commentaires dans les fonctions :
~~~{c}
    /* */
~~~

- Commentaire des header :
  - Utilisation de la syntaxe doxygen
  -
~~~{c}
/* *
* Ajoute une contrainte à un ensemble de contraintes
* @function add_pressure
* @param  s            L'ensemble des contraintes
* @param  s1           un pelican
* @param  s2           un autre pelican
* @param  tag          Le tag de la contrainte
* @return void
*/
void add_pressure(struct pressures p, int s1, int s2, int tag){
    ...
}

~~~

### Les espaces
- **Virgules** :
 - Pas d'espace avant
 - Un espace après
- **Opérateurs**
 - Un espace avant
 - Un espace après
 - Exception à ces règles pour un aspect plus visuel des priorités opératoires :
~~~
3/4 + 5
~~~

### Les fonctions locales

- Les fonctions locales sont déclarées explicitement (i.e. mot clé static)
- Leurs déclarations seront regroupées en début de fichier, après les include et autre # (instructions préprocesseur)

~~~{c}
#include "hash.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#define HASH_SIZE 13

static struct Cell hash_new_cell(int data);
~~~
