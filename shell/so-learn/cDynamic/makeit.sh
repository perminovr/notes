#!/bin/sh

gcc -fPIC -shared -o libso_exmp.so -c so_exmp.c

gcc main.c -I. -ldl -o so_test

./so_test
