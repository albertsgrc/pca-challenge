#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define N 1000000
#define BUFF_SIZE 16384*2*2
#define MAX_SOL 159139

#define SUM_ITER(__i) {\
    sum[__i] = sum[__i-1] + mem[__i];\
}\

typedef long double LD;
typedef unsigned char uchar;
typedef unsigned int uint;

uint stack[447][3];

uchar mem[N+1];
//uint padd[2048];
uint sum[N+1];

#if _TIME_

unsigned long cpu_time_usecs(struct rusage* r) {
    return
    r->ru_utime.tv_sec*1e6 + r->ru_utime.tv_usec +
    r->ru_stime.tv_sec*1e6 + r->ru_stime.tv_usec;
}

#endif

#define push(__stack, __x, __y, __z) {\
    __stack[++stackIndex][0] = __x;\
    __stack[stackIndex][1] = __y;\
    __stack[stackIndex][2] = __z;\
}\

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

inline int compare( const void* a, const void* b)
{
     return *(int*)a - *(int*)b;
}


int main() {
    uint i, bytes_read, n;
    #if _TIME_
    unsigned long ta, ts, te;
    struct rusage rb, ra;
    struct timeval tvb, tva;
    #endif

    /*volatile int k = 0;
    for (i = 0; i <= 100000000; ++i)
        k++;
*/
    char buffer[BUFF_SIZE];

    memset(mem, 0, sizeof(mem));

    // Memoize the results
    #if _TIME_
    getrusage(RUSAGE_SELF, &rb);
    #endif
    compute_triples();
    #if _TIME_
    getrusage(RUSAGE_SELF, &ra);

    ta = cpu_time_usecs(&ra) - cpu_time_usecs(&rb);
    #endif
    // The memset has been eliminated because it was unnecessary

    // And compute partial sum
    // N - 5 + 1 is already multiple of 6, so we perform unrolling of that degree
    // Also, the compiler didn't perform unrolling before
    #if _TIME_
    getrusage(RUSAGE_SELF, &rb);
    #endif

    sum[1] = sum[2] = sum[3] = sum[4] = 0;
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
    #if _TIME_
    getrusage(RUSAGE_SELF, &ra);

    ts = cpu_time_usecs(&ra) - cpu_time_usecs(&rb);
    #endif


    // Especialización del scanf y printf
    n = 0;
    // Dejamos 8 bytes libres al principio, ya que vamos a escribir los resultados
    // en el mismo buffer, y se podrían sobreescribir valores leídos si ha
    // quedado algún parseo a medias de la iteración anterior
    #if _TIME_
    gettimeofday(&tvb);
    #endif
    while ((bytes_read = read(0, &buffer[8], sizeof(buffer) - 8)) > 0) {
        char* pointer = &buffer[8];
        char* last = &buffer[bytes_read+8];
        char* pwrite = &buffer[0];
        do {
            // Parseamos un valor del buffer
            if (*pointer != '\n') {
                do {
                    n = n*10 + *pointer - '0';
                    ++pointer;
                } while (pointer < last && *pointer != '\n');
                                            __builtin_prefetch(&sum[n], 0, 3);
            }

            if (pointer == last) break;

            ++pointer;

                // n es válido
            if (n == 0) {
                write(1, buffer, pwrite - &buffer[0]);

                #if _TIME_
                gettimeofday(&tva);

                te = (tva.tv_sec - tvb.tv_sec)*1e6 + (tva.tv_usec - tvb.tv_usec);

                fprintf(stderr, "Algoritmo: %lu us\nSuma: %lu us\nEscritura: %lu us\n", ta, ts, te);
                fflush(stderr);
                #endif

                return 0;
            }

            uint value = sum[n];

            // Convertimos el valor a su string correspondiente

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
            n = 0;
            // else dejamos en n la parte parseada que se acabará de parsear
            //      en la siguiente iteración del bucle externo
        } while(pointer < last);

        write(1, buffer, pwrite - &buffer[0]);
    }
}
