#!/bin/bash

cd src
rm -r build
mkdir -p build && cd build
cmake ..
make -j8