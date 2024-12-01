#! /usr/bin/env bash

set -o errexit
set -o nounset
set -o pipefail


cd ..
gcc -std=gnu11 -I./include -o bin/msh src/msh.c src/shell.c

cd scripts
#gcc -I./include/ -o ./bin/msh src/*.c
#alias msh=./bin/msh