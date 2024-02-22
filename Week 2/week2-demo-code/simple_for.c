#include <stdio.h>
  
int for_loop(int upto) {

    int sum = 0;	

    for (int i = 0; i < upto; i++)
	    sum = sum + i;

    return sum;
}

int main() {
    
    int a = 3;

    for_loop(a);

    return 0;
}

