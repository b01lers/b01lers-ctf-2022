CC=gcc
CFLAGS= -no-pie 
CXXFLAGS=
LDFLAGS= -Wl,-z,relro,-z,now

all: gambler_supreme

gambler_supreme: gambler_supreme.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
