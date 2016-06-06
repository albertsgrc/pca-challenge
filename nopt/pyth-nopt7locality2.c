#include <stdio.h>
#include <string.h>

#define N 1000000

#define SUM_ITER(__s) {\
    sum[i+__s] = sum[i+__s-1] + ((v << (60 - __s*4)) >> 60);\
}\

unsigned int n;

unsigned char mem[N/2] __attribute__((aligned(8))); // Use only 4 bits for each value to further increase locality
unsigned int sum[N+1];

void compute_triples_from(unsigned int x, unsigned int y, unsigned int z) {
    // Use different variables to help compiler detect that there are no dependencies
    unsigned int zp1, zp2, zp3;

    // Increment hex value at position __z
    // First, compute the index to of the hex to incr
    // If value is odd, then saturate the rightmost 4 bits to be able to add
    // 1 correctly to the left 4 bits, otherwise we're already adding correctly
    // The new value is a combination of the incremented value and the previous value
    // based on mask. If the value was odd, we only need to change the left 4 bits
    // so the mask picks those bits (0 bits) from __v and the rest (1 bits) from mem[__index]
    // Otherwise, we should pick all bits from __v because adding to the right 4 bits
    // does never modify the left bits (0 <= mem[z] <= 8 < 15), so a mask with all 0's
    // is fine
    unsigned int index = z/2;
    unsigned char mask = (z&1) ? 0x0F : 0x00;
    unsigned char v = ((mem[index] | mask) + 1);
    mem[index] = v ^ ((v ^ mem[index]) & mask);

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

    memset(mem, 0, sizeof(mem));

    // Memoize the results
    compute_triples_from(3, 4, 5);
    // The memset has been eliminated because it was unnecessary

    // And compute partial sum
    sum[1] = sum[2] = sum[3] = sum[4] = 0;
    sum[5] = sum[6] = sum[7] = sum[8] = sum[9] = sum[10] = sum[11] = sum[12] = 1;
    sum[13] = sum[14] = sum[15] = 2;

    // N - 5 + 1 is already multiple of 6, so we perform unrolling of that degree
    // Also, the compiler didn't perform unrolling before
    unsigned long long* memp = (unsigned long long*) &mem[0];
    for (i = 16; i < N; i += 16) {
        unsigned int ind = i/16;
        unsigned long long v = memp[ind];

        SUM_ITER(0);
        SUM_ITER(1);
        SUM_ITER(2);
        SUM_ITER(3);
        SUM_ITER(4);
        SUM_ITER(5);
        SUM_ITER(6);
        SUM_ITER(7);
        SUM_ITER(8);
        SUM_ITER(9);
        SUM_ITER(10);
        SUM_ITER(11);
        SUM_ITER(12);
        SUM_ITER(13);
        SUM_ITER(14);
        SUM_ITER(15);
    }

    sum[1000000] = sum[999999];

    scanf("%d", &n);
    while (n != 0) {
        printf("%d\n", sum[n]);
        scanf("%d", &n);
    }

    return 0;
}
