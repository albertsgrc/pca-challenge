#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <unistd.h>
#include <time.h>

#define N 1000000
#define BUFF_SIZE 16384*2*2

#define SUM_ITER(__i) {\
    sum[__i] = sum[__i-1] + mem[__i];\
}\

typedef long double LD;
typedef unsigned char uchar;
typedef unsigned int uint;

uchar mem[N+1]; // Use char instead to improve locality
uint sum[N+1];

void compute_triples_from(uint x, uint y, uint z) {
    // Use different variables to help compiler detect that there are no dependencies
    uint zp1, zp2, zp3;

    ++mem[z]; // There are a lot of cache misses here because accesses are not sequential

    zp1 = 2*x - 2*y + 3*z;
    zp2 = -2*x + 2*y + 3*z;
    zp3 = 2*x + 2*y + 3*z;

    // The order of the recursive calls has been changed to try to maximize locality
    // of the previous mem[z] access
    if (zp3 <= N) compute_triples_from(x + 2*y + 2*z, 2*x + y + 2*z, zp3);
    if (zp1 <= N) compute_triples_from(x - 2*y + 2*z, 2*x - y + 2*z, zp1);
    if (zp2 <= N) compute_triples_from(-x + 2*y + 2*z, -2*x + y + 2*z, zp2);
}

inline uint decimal_digits(uint n) {
    return
    n < 100000 && n >= 10000 ? 5 :
    n < 10 ? 1 :
    n < 100 ? 2 :
    n < 1000 ? 3 :
    n < 10000 ? 4 : 6;
}


int main() {
    uint i, bytes_read, n, written;

    char buffer[BUFF_SIZE];

    memset(mem, 0, sizeof(mem));

    // Memoize the results
    compute_triples_from(3, 4, 5);

    // And compute partial sum
    // N - 5 + 1 is already multiple of 6, so we perform unrolling of that degree
    // Also, the compiler didn't perform unrolling before
    for (i = 5; i <= N; i += 6) {
        SUM_ITER(i)
        SUM_ITER(i+1)
        SUM_ITER(i+2)
        SUM_ITER(i+3)
        SUM_ITER(i+4)
        SUM_ITER(i+5)
    }

    // scanf & printf specialization
    n = 0;
    // We leave 8 bytes free at the beggining, because we're going to be writting
    // the results from the beggining of the buffer as we compute them,
    // and if we didn't leave some space it could happen that some non-yet processed
    // inputs would be overwritten in case there was some leftover from the previous
    // iteration (a number which happens to be cut along two consecutive reads)
    while ((bytes_read = read(0, &buffer[8], sizeof(buffer) - 8)) > 0) {
        i = 0;
        written = 0;
        while (i < bytes_read) {
            // Parseamos un valor del buffer
            while (i < bytes_read && buffer[i+8] != '\n') {
                n = n*10 + buffer[i+8] - '0';
                ++i;
            }

            if (i != bytes_read) { // We managed to parse an input number
                if (n == 0) {
                    write(1, buffer, written);
                    return 0;
                }

                uint value = sum[n];

                // Convert the value to its corresponding string (utoa)
                uint n_digits = decimal_digits(value);
                uint n_digits_cp = n_digits;
                do {
                    buffer[written + n_digits - 1] = value%10 + '0';
                    value /= 10;
                    --n_digits;
                } while (value > 0);

                written += n_digits_cp;
                buffer[written++] = '\n';

                ++i; // Erase the endlien from the buffer
                n = 0;
            }
            // Otherwise we leave in n the parsed leftover which will be
            // parsed to the end in the next iteration of the outer loop
        }

        write(1, buffer, written);
    }
}
