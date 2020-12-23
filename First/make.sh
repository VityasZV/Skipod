#!/bin/bash -x
sudo rm -rf build/
mkdir build
cd build
cmake .. && make 
touch test.txt
cd ..
python3 script.py
