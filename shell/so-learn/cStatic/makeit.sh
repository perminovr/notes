#!/bin/sh

gcc -c -o liba_exmp.o a_exmp.c
ar rcs liba_exmp.a liba_exmp.o # archive from objects
gcc -I. -L. -o a_test main.c -la_exmp
