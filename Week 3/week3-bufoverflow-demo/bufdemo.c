#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 16
#define KEY "_anu_0123456789_canberra_"

void touch() {
	printf("The secret key is: %s \n", KEY);
	return;
}

unsigned getbuf()
{
    char buf[BUFFER_SIZE];
    gets(buf);
    return 1;
}

int main() {
	printf("main() executed just now \n");
	getbuf();
	return 0;
}
