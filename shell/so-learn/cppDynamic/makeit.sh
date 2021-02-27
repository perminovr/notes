#!/bin/sh

g++ -fPIC -shared -o libso_exmp.so so_exmp.cpp

g++ main.cpp -I. -L. -ldl -o so_test

./so_test
