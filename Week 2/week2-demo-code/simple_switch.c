#include <stdio.h>

long switch_eg
   (long x, long y, long z)
{
    long w = 1;
    switch(x) {
    case 1:
        w = y*z;
        break;
    case 2:
        w = y/z;
        /* Fall Through */
    case 3:
        w += z;
        break;
    case 5:
    case 6:
        w -= z;
        break;
    case 8:
        w -= 1;
        break;
    case 9:
        w -= 2;
        break;
    default:
        w = 2;
    }
    return w;
}


int main() {

	int result = switch_eg(10,11,12);

	return 0;
}
