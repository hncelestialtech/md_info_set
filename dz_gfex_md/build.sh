#!/bin/bash

#scl enable devtoolset-11 bash


rm build -rf

mkdir build
cd build;
cmake ..
make -j32
cd ../
rm build -rf

ls -l ./lib


