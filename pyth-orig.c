#include <stdio.h>

unsigned int n;

unsigned int compute_triples_from(unsigned int x, unsigned int y, unsigned int z) {
    unsigned int xp, yp, zp, cont = 1;

    xp =   x - 2*y + 2*z;
    yp = 2*x -   y + 2*z;
    zp = 2*x - 2*y + 3*z;

    if (zp <= n) cont += compute_triples_from(xp, yp, zp);

    xp = -  x + 2*y + 2*z;
    yp = -2*x +   y + 2*z;
    zp = -2*x + 2*y + 3*z;

    if (zp <= n) cont += compute_triples_from(xp, yp, zp);

    xp =   x + 2*y + 2*z;
    yp = 2*x +   y + 2*z;
    zp = 2*x + 2*y + 3*z;

    if (zp <= n) cont += compute_triples_from(xp, yp, zp);

    return cont;
}

int main() {
    scanf("%d", &n);
    while (n != 0) {
        if (n < 5) printf("%d\n", 0);
        //else printf("%d\n", compute_triples_from(3, 4, 5));
        scanf("%d", &n);
    }

    return 0;
}
