#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define N 1000000
#define BUFF_SIZE 16384*2*2*2

#define SUM_ITER(__i) {\
    sum[__i] = sum[__i-1] + mem[__i];\
}\

#define push(__x, __y, __z) {\
    stack_pointer += 3;\
    *stack_pointer = __x;\
    *(stack_pointer + 1) = __y;\
    *(stack_pointer + 2) = __z;\
}\

typedef unsigned char uchar;
typedef unsigned int uint;

uint stack[447][3];
uchar mem[N+1]; // Use char instead to improve locality

const char memdigits[200] = {
    '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
    '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
    '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
    '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
    '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
    '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
    '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
    '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
    '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
    '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
};
uint sum[N+1];

// I've replaced the recursion by a stack of the exact needed size
// Now I've got a little bit more control on the iteration for future optimizations
inline void compute_triples() {
    uint x, y, z;
    uint zp1, zp2, zp3;
    // Replace stackIndex by stack_pointer to avoid extra address computations
    uint* stack_begin = &stack[0][0];
    uint* stack_pointer = stack_begin;
    stack[0][0] = 3; stack[0][1] = 4; stack[0][2] = 5;

    z = 0;
    do {
        ++mem[z];
        x = *stack_pointer; y = *(stack_pointer + 1); z = *(stack_pointer + 2);
        unsigned int x2 = x << 1;
        unsigned int y2 = y << 1;
        unsigned int z2 = (z << 1);
        unsigned int z3 = z2 + z;
        zp1 = x2 -y2 + z3;
        zp2 = -x2 + y2 + z3;
        zp3 = x2 + y2 + z3;
        stack_pointer -= 3;
        if (zp3 <= N) push(x + y2 + z2, x2 + y + z2, zp3);
        if (zp1 <= N) push(x - y2 + z2, x2 - y + z2, zp1);
        if (zp2 <= N) push(-x + y2 + z2, -x2 + y + z2, zp2);
        __builtin_prefetch(&mem[z], 0, 3);
        // Make a prefetch for the access of the next iteration
    } while (stack_pointer >= stack_begin);

    ++mem[z];
    // Use different variables to help compiler detect that there are no dependenciea
}

inline uint count_digits(char* from) {
    if (*(from + 1) == '\n') return 1;
    else if (*(from + 2) == '\n') return 2;
    else if (*(from + 3) == '\n') return 3;
    else if (*(from + 4) == '\n') return 4;
    else if (*(from + 5) == '\n') return 5;
    else if (*(from + 6) == '\n') return 6;
    else return 7;
}

int main() {
    uint i, bytes_read, n, value;

    char buffer[BUFF_SIZE];

    memset(mem, 0, sizeof(mem));

    // Memoize the results
    compute_triples();

    // And compute partial sum
    // N - 5 + 1 is already multiple of 6, so we perform unrolling of that degree
    // Also, the compiler didn't perform unrolling before
    sum[1] = sum[2] = sum[3] = sum[4] = 0;
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
    value = 0;
    int foundBefore = 0; // The write of sum[n] is delayed until next iteration
                         // so that leaves more time space between the prefetch
                         // and the read of sum[n]
    do {
        // Replace indices by pointers, and while loops by do-while loops
        char* pread = &buffer[8];
        char* last = &buffer[8+bytes_read];
        char* pwrite = &buffer[0];
        do {
            // Try to parse a value from the buffer
            if (*pread != '\n') {
                if (pread > &buffer[8] && pread < last - 8) {
                    uint digits = count_digits(pread);
                    switch (digits) {
                        case 1: n = pread[0] - '0'; break;
                        case 2: n = 10*(pread[0] - '0') + pread[1] - '0'; break;
                        case 3: n = 100*(pread[0] - '0') + 10*(pread[1] - '0') + pread[2] - '0'; break;
                        case 4: n = 1000*(pread[0] - '0') + 100*(pread[1] - '0') + 10*(pread[2] - '0') + pread[3] - '0'; break;
                        case 5: n = 10000*(pread[0] - '0') + 1000*(pread[1] - '0') + 100*(pread[2] - '0') + 10*(pread[3] - '0') + pread[4] - '0'; break;
                        case 6: n = 100000*(pread[0] - '0') + 10000*(pread[1] - '0') + 1000*(pread[2] - '0') + 100*(pread[3] - '0') + 10*(pread[4] - '0') + pread[5] - '0'; break;
                        case 7: n = 1000000*(pread[0] - '0') + 100000*(pread[1] - '0') + 10000*(pread[2] - '0') + 1000*(pread[3] - '0') + 100*(pread[4] - '0') + 10*(pread[5] - '0') + pread[6] - '0'; break;
                    }
                    pread += digits;
                }
                else {
                    do {
                        n = n*10 + *pread - '0';
                    } while (++pread < last && *pread != '\n');
                }

                // Prefetch the posterior read to get the memoized result
                __builtin_prefetch(&sum[n], 0, 3);
            }

            if (foundBefore) {
                if (value >= 10000) {
                    const uint b = value / 10000;
                    const uint c = value % 10000;
                    const uint d2 = (b % 100) << 1;
                    const uint d3 = (c / 100) << 1;
                    const uint d4 = (c % 100) << 1;

                    if (value >= 100000)
                        *pwrite++ = memdigits[d2];

                    *pwrite++ = memdigits[d2 + 1];
                    *pwrite++ = memdigits[d3];
                    *pwrite++ = memdigits[d3 + 1];
                    *pwrite++ = memdigits[d4];
                    *pwrite++ = memdigits[d4 + 1];
                }
                else {
                    const uint d1 = (value / 100) << 1;
                    const uint d2 = (value % 100) << 1;

                    if (value >= 1000)
                        *pwrite++ = memdigits[d1];
                    if (value >= 100)
                        *pwrite++ = memdigits[d1 + 1];
                    if (value >= 10)
                        *pwrite++ = memdigits[d2];
                    *pwrite++ = memdigits[d2 + 1];
                }

                *pwrite++ = '\n';
            }

            foundBefore = pread < last;
            if (foundBefore) { // We managed to parse an input number
                value = sum[n];
                ++pread; // Erase the endline from the buffer

                if (n == 0) {
                    write(1, buffer, pwrite - &buffer[0]);
                    return 0;
                }

                n = 0;
            }

            // Otherwise we leave in n the parsed leftover which will be
            // parsed to the end in the next iteration of the outer loop
        } while (pread < last);

        write(1, buffer, pwrite - &buffer[0]);
        bytes_read = read(0, &buffer[8], sizeof(buffer) - 8);
    } while (bytes_read > 0);
}
