#! /bin/bash

cd /home/data_science

# standard version (hard)
timeout 30 ./data_science-O2 0.  ||  echo "Timeout"
