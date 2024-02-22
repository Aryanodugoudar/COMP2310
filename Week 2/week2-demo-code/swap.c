#include <stdio.h>
  
void swap_v2(int *a, int *b) {

    int t1 = *a;
    int t2 = *b;

    *a = t2;
    *b = t1;

    printf("(swap_v2) a = %i, b = %i\n", *a, *b);
    return;
}

int main() {
    int a = 2;
    int b = 3;

    printf("(main) a = %i, b = %i\n", a, b);

    swap_v2(&a, &b);

    return 0;
}

