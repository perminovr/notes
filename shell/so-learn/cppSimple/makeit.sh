#!/bin/sh

# С -nostdlib не комплит
g++ -fPIC -shared -o libso_exmp.so so_exmp.cpp

g++ main.cpp -I. -L. -lso_exmp -o so_test

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/roman/workspace/so-learn/cppSimple ./so_test
