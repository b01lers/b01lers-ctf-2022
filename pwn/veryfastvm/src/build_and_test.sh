#!/bin/bash

rm cpu.py
rm __pycache__

python3 ./build.py
python3 ./test.py

rm ../solve/exploit
cp exploit ../solve/

rm ../dist/cpu.py
cp cpu.py ../dist

rm ../solve/flag.txt
cp flag.txt ../solve/
