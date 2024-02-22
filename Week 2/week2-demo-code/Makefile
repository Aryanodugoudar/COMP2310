CC = gcc
CFLAGS= -Og
FNOSP=-fno-stack-protector
FNOPIE=-fno-pie -fno-pic -no-pie
MCMODEL=-mcmodel=large

pop_count_while: pop_count_while.c
	$(CC) $(CFLAGS) $(FNOPIE) $(FNOSP) pop_count_while.c -o pop_count_while

simple_for: simple_for.c
	$(CC) $(CFLAGS) $(FNOPIE) $(FNOSP) simple_for.c -o simple_for

simple_for: simple_switch.c
	$(CC) $(CFLAGS) $(FNOPIE) $(FNOSP) simple_switch.c -o simple_switch

swap: swap.c
	$(CC) $(CFLAGS) $(FNOPIE) $(FNOSP) swap.c -o swap

locate-npie: locate.c
	$(CC) $(CFLAGS) $(FNOPIE) $(FNOSP) locate.c -o locate-npie

locate-pie: locate.c
	$(CC) $(CFLAGS) $(MCMODEL) locate.c -o locate-pie

