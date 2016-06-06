#include <stdio.h>
#include <string.h>

#define N 1000000

unsigned int n;

unsigned char mem[N+1]; // Use char instead to improve locality
unsigned int sum[N+1];

void compute_triples_from(unsigned int x, unsigned int y, unsigned int z) {
    unsigned int xp, yp, zp;

    ++mem[z]; // There are a lot of cache misses here because accesses are not sequential

    xp =   x - 2*y + 2*z;
    yp = 2*x -   y + 2*z;
    zp = 2*x - 2*y + 3*z;

    if (zp <= N) compute_triples_from(xp, yp, zp);

    xp = -  x + 2*y + 2*z;
    yp = -2*x +   y + 2*z;
    zp = -2*x + 2*y + 3*z;

    if (zp <= N) compute_triples_from(xp, yp, zp);

    xp =   x + 2*y + 2*z;
    yp = 2*x +   y + 2*z;
    zp = 2*x + 2*y + 3*z;

    if (zp <= N) compute_triples_from(xp, yp, zp);
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
