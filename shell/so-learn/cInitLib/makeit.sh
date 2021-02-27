#!/bin/sh

gcc -fPIC -shared -nostdlib -o libso_exmp.so -c so_exmp.c

gcc main.c -I. -L. -lso_exmp -o so_test

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/roman/workspace/so-learn/cInitLib ./so_test
