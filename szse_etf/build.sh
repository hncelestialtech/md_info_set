#!/bin/bash

rm build -rf

mkdir build
cd build;
cmake ..
make
cd ../
rm build -rf

ls -l ./lib


