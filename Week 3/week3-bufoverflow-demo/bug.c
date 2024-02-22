#include <stdio.h>
#include <stdlib.h>

typedef struct {
	int a[2];
	double d;
} struct_t;

double fun(int i) {
	volatile struct_t s;
	s.d = 3.14;
	s.a[i] = 1073741824; /* Possibly out of bounds */
	return s.d;
}

int main() {

	int val;
	double d;

	printf("Enter an integer: ");
	scanf("%d", &val);

	d = fun(val);
	d = fun(val);
	d = fun(val);
	d = fun(val);
	d = fun(val);
	d = fun(val);
	d = fun(val);

	printf("fun(%d) = %1.20f \n", val, d);

	return 0;
}
