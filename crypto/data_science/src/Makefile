all: data_science-O3 data_science-O2 data_science-O1 data_science-O0

%-O3: %.c
	gcc -o $@ -Wall -O3 $< -lm
	strip -s $@

%-O2: %.c
	gcc -o $@ -Wall -O2 $< -lm
	strip -s $@

%-O1: %.c
	gcc -o $@ -Wall -O1 $< -lm
	strip -s $@

%-O0: %.c
	gcc -o $@ -Wall -O1 $< -lm
	strip -s $@
