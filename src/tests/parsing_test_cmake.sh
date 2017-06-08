# Fichier : parsing_test.sh
# Date de dernière édition : 07/03/2017
# Auteur : PARPAITE Thibault
# Description : Script permettant d'effectuer les tests du lexer/parser

#!/bin/bash

#############
# Fonctions #
#############

usage() {
    echo "usage: $0 sapoX ok|err path"
    echo "X is the number of test file\nok to test a valid file, err an unvalid file\npath is the path to the parser examples folder."
    exit 1
}


############################
# Boucle principale (main) #
############################

TMP_FILE="/tmp/sapo.tmp"

if [ $# -ne 3 ]
then
    usage
else
    if [ $2 = "ok" ]
    then
        # sapoX.ok.txt
        ./parsing_test $1.ok.txt > ${TMP_FILE}
        if [ -z $(diff $3/sapo$1.ok.exp ${TMP_FILE}) ]
        then
            exit 0
        else
            exit 1
        fi
    else
        # sapoX.err.txt
        if [ ! $(./parsing_test $3/sapo$1.err.txt >/dev/null 2>&1 || false) ]
        then
            exit 0
        else
            exit 1
        fi
    fi
fi
