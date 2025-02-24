#!/usr/bin/bash

if [[ $1 = "" || $1 = "normal" ]]; then
	echo compilando em modo normal
	gcc -Wall -Wextra -Wpedantic \
		-fdce -fdata-sections -ffunction-sections -Wl,--gc-sections \
		tests.c -o tests.o -lm -Dcels_debug=false
elif [[ $1 = "debug" ]]; then
	echo compilando em modo debug
	gcc -Wall -Wextra -Wpedantic \
		-fdce -fdata-sections -ffunction-sections -Wl,--gc-sections \
		-g tests.c -o tests.o -lm -Dcels_debug=true
elif [[ $1 = "assembly" ]]; then
	echo compilando em modo assembly
	gcc -Wall -Wextra -Wpedantic \
		-fdce -fdata-sections -ffunction-sections -Wl,--gc-sections \
		-S tests.c -o tests.s -lm
else
	echo opção não encontrada
fi
