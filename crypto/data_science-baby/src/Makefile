all: data_science-baby-O3 data_science-baby-O2 data_science-baby-O1

%-O3: %.c
	gcc -o $@ -Wall -O3 $< -lm
	strip -s $@

%-O2: %.c
	gcc -o $@ -Wall -O2 $< -lm
	strip -s $@

%-O1: %.c
	gcc -o $@ -Wall -O1 $< -lm
	strip -s $@
