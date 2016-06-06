#include <stdio.h>
#include <string.h>

#define N 1000000

unsigned int n;

unsigned char mem[N+1]; // Use char instead to improve locality
unsigned int sum[N+1];

void compute_triples_from(unsigned int x, unsigned int y, unsigned int z) {
    // Use different variables to help compiler detect that there are no dependencies
    unsigned int xp1, yp1, zp1, xp2, yp2, zp2, xp3, yp3, zp3;

    ++mem[z]; // There are a lot of cache misses here because accesses are not sequential

    xp1 =   x - 2*y + 2*z;
    yp1 = 2*x -   y + 2*z;
    zp1 = 2*x - 2*y + 3*z;

    xp2 = -  x + 2*y + 2*z;
    yp2 = -2*x +   y + 2*z;
    zp2 = -2*x + 2*y + 3*z;

    xp3 =   x + 2*y + 2*z;
    yp3 = 2*x +   y + 2*z;
    zp3 = 2*x + 2*y + 3*z;

    if (zp1 <= N) compute_triples_from(xp1, yp1, zp1);

    if (zp2 <= N) compute_triples_from(xp2, yp2, zp2);

    if (zp3 <= N) compute_triples_from(xp3, yp3, zp3);
}

int main() {
    unsigned int i;

    memset(mem, 0, sizeof(mem));
    // Memoize the results
    compute_triples_from(3, 4, 5);

    // And compute partial sum
    memset(sum, 0, sizeof(sum));
    for (i = 5; i <= N; ++i) sum[i] = sum[i-1] + mem[i];

    scanf("%d", &n);
    while (n != 0) {
        printf("%d\n", sum[n]);
        scanf("%d", &n);
    }

    return 0;
}
