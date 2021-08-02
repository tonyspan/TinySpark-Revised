#!/bin/bash

mkdir -p GenProtos/cpp
mkdir -p GenProtos/py

mkdir -p build
cd build
cmake ..
make