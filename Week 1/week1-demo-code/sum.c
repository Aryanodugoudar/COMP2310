#include <stdio.h>

char char_sum(char, char);
short short_sum(short, short);
int int_sum(int, int);
long long_sum(long, long);

int main() {    

    char  c = char_sum(10, 19);
    short s = short_sum(10, 19);
    int   i = int_sum(10, 19);
    long  l = long_sum(10, 19);
    
    return 0;
}

char char_sum(char a, char b) {
	return a + b;
}


short short_sum(short a, short b) {
	return a + b;
}


int int_sum(int a, int b) {
	return a + b;
}

long long_sum(long a, long b) {
	return a + b;
}
