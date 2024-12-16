#!/usr/bin/bash

if [[ $1 = "" || $1 = "normal" ]]; then
	echo compilando em modo normal
	gcc -Wall -Wextra -Wpedantic tests.c -o tests.o -lm
elif [[ $1 = "debug" ]]; then
	echo compilando em modo debug
	gcc -Wall -Wextra -Wpedantic -g tests.c -o tests.o -lm -Dcels_debug=true
else
	echo opção não encontrada
fi
