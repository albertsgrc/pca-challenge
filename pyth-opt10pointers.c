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

#define push(__stack, __x, __y, __z) {\
    __stack[++stackIndex][0] = __x;\
    __stack[stackIndex][1] = __y;\
    __stack[stackIndex][2] = __z;\
}\

typedef long double LD;
typedef unsigned char uchar;
typedef unsigned int uint;

uint stack[447][3];
uchar mem[N+1]; // Use char instead to improve locality
uint sum[N+1];

// I've replaced the recursion by a stack of the exact needed size
// Now I've got a little bit more control on the iteration for future optimizations
inline void compute_triples() {
    uint x, y, z;
    uint zp1, zp2, zp3;
    int stackIndex = 0;
    stack[0][0] = 3; stack[0][1] = 4; stack[0][2] = 5;
    z = 0;
    do {
        ++mem[z];
        x = stack[stackIndex][0]; y = stack[stackIndex][1]; z = stack[stackIndex][2];
        zp1 = 2*x -2*y + 3*z;
        zp2 = -2*x + 2*y + 3*z;
        zp3 = 2*x + 2*y + 3*z;

        --stackIndex;

        if (zp3 <= N) push(stack, x + 2*y + 2*z, 2*x + y + 2*z, zp3);
        if (zp1 <= N) push(stack, x - 2*y + 2*z, 2*x - y + 2*z, zp1);
        if (zp2 <= N) push(stack, -x + 2*y + 2*z, -2*x + y + 2*z, zp2);
    } while (stackIndex >= 0);

    ++mem[z];
    // Use different variables to help compiler detect that there are no dependenciea
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
    uint i, bytes_read, n;

    char buffer[BUFF_SIZE];

    memset(mem, 0, sizeof(mem));

    // Memoize the results
    compute_triples();

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
    bytes_read = read(0, &buffer[8], sizeof(buffer) - 8);
    do {
        // Replace indices by pointers, and while loops by do-while loops
        char* pread = &buffer[8];
        char* last = &buffer[8+bytes_read];
        char* pwrite = &buffer[0];
        do {
            // Try to parse a value from the buffer
            if (*pread != '\n') {
                do {
                    n = n*10 + *pread - '0';
                } while (++pread < last && *pread != '\n');
            }

            if (pread < last) { // We managed to parse an input number
                ++pread; // Erase the endline from the buffer

                if (n == 0) {
                    write(1, buffer, pwrite - &buffer[0]);
                    return 0;
                }

                uint value = sum[n];

                // Convert the value to its corresponding string (utoa)
                uint n_digits = decimal_digits(value);
                pwrite += n_digits + 1;
                char* number_end = pwrite -1;
                *number_end-- = '\n';

                do {
                    *number_end = value%10 + '0';
                    --number_end;
                    value /= 10;
                } while (value > 0);

                n = 0;
            }
            // Otherwise we leave in n the parsed leftover which will be
            // parsed to the end in the next iteration of the outer loop
        } while (pread < last);

        write(1, buffer, pwrite - &buffer[0]);
        bytes_read = read(0, &buffer[8], sizeof(buffer) - 8);
    } while (bytes_read > 0);
}
