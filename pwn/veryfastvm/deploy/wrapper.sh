#! /bin/bash

(
    cd /home/veryfastvm/
    timeout -k 1 10 stdbuf -e0 -i0 -o0 python3 -u ./cpu.py 2>/dev/null
)

