#include <stdio.h>
#include <string.h>

#define N 1000000

#define SUM_ITER(__i, __mem) {\
    sum[__i] = sum[__i-1] + __mem[(__i) >> 1];\
}\

unsigned int n;

unsigned char mem1[N >> 1]; // Use char instead to improve locality
unsigned char mem2[N >> 1];
unsigned int sum[N+1];

void compute_triples_from(unsigned int x, unsigned int y, unsigned int z) {
    // Use different variables to help compiler detect that there are no dependencies
    unsigned int zp1, zp2, zp3;

    unsigned char* arr = (z&1) ? mem2 : mem1;
    ++arr[z >> 1]; // There are a lot of cache misses here because accesses are not sequential

    zp1 = 2*x - 2*y + 3*z;
    zp2 = -2*x + 2*y + 3*z;
    zp3 = 2*x + 2*y + 3*z;

    // The order of the recursive calls has been changed to try to maximize locality
    // of the previous mem[z] access
    if (zp3 <= N) compute_triples_from(x + 2*y + 2*z, 2*x + y + 2*z, zp3);
    if (zp1 <= N) compute_triples_from(x - 2*y + 2*z, 2*x - y + 2*z, zp1);
    if (zp2 <= N) compute_triples_from(-x + 2*y + 2*z, -2*x + y + 2*z, zp2);
}

int main() {
    unsigned int i;

    memset(mem1, 0, sizeof(mem1));
    memset(mem2, 0, sizeof(mem2));

    // Memoize the results
    compute_triples_from(3, 4, 5);

    // The memset has been eliminated because it was unnecessary

    // And compute partial sum
    sum[1] = sum[2] = sum[3] = sum[4] = 0;
    // N - 5 + 1 is already multiple of 6, so we perform unrolling of that degree
    // Also, the compiler didn't perform unrolling before
    for (i = 5; i <= N; i += 6) {
        SUM_ITER(i, mem2)
        SUM_ITER(i+1, mem1)
        SUM_ITER(i+2, mem2)
        SUM_ITER(i+3, mem1)
        SUM_ITER(i+4, mem2)
        SUM_ITER(i+5, mem1)
    }

    scanf("%d", &n);
    while (n != 0) {
        printf("%d\n", sum[n]);
        scanf("%d", &n);
    }

    return 0;
}
