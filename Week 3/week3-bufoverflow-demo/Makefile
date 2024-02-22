CC = gcc
CFLAGS= -Og
FNOSP=-fno-stack-protector
FNOPIE=-fno-pie -fno-pic -no-pie
MCMODEL=-mcmodel=large

bug: bug.c
	$(CC) $(CFLAGS) bug.c -o bug

bug-nsp: bug.c
	$(CC) $(CFLAGS) $(FNOSP) bug.c -o bug-nsp

bufdemo-sp: bufdemo.c
	$(CC) $(CFLAGS) $(FNOPIE) bufdemo.c -o bufdemo-sp

bufdemo-nsp: bufdemo.c
	$(CC) $(CFLAGS) $(FNOPIE) $(FNOSP) bufdemo.c -o bufdemo-nsp

echo-nsp: echo.c
	$(CC) $(CFLAGS) $(FNOPIE) $(FNOSP) echo.c -o echo-nsp

stack: stack.c
	$(CC) $(CFLAGS) $(FNOPIE) stack.c -o stack

clean:
	rm -rf bug bug-nsp bufdemo-sp bufdemo-nsp echo-nsp stack
