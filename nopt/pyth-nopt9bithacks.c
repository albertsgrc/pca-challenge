#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#define N 1000000
#define BUFF_SIZE 16384*2*2

#define SUM_ITER(__i) {\
    sum[__i] = sum[__i-1] + mem[__i];\
}\

unsigned char mem[N+1]; // Use char instead to improve locality
unsigned int sum[N+1];

static char table[16] = {0, 1, 2, 2, 3, 3, 4, 5, 5, 6, 7, 7, 8, 8, 9, 0};

void compute_triples_from(unsigned int x, unsigned int y, unsigned int z) {
    // Use different variables to help compiler detect that there are no dependencies
    unsigned int zp1, zp2, zp3;

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

inline unsigned int decimal_digits(unsigned int n) {
    return
    n < 100000 && n >= 10000 ? 5 :
    n < 10 ? 1 :
    n < 100 ? 2 :
    n < 1000 ? 3 :
    n < 10000 ? 4 : 6;
}

inline unsigned int rem10(unsigned int n) {
    n = (0x19999999*n + (n >> 1) + (n >> 3)) >> 28;
    return table[n];
}


int main() {
    unsigned int i, bytes_read, n, written;
    char buffer[BUFF_SIZE];


    memset(mem, 0, sizeof(mem));

    // Memoize the results
    compute_triples_from(3, 4, 5);

    // The memset has been eliminated because it was unnecessary

    // And compute partial sum
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

    // Especialización del scanf y printf
    n = 0;
    // Dejamos 8 bytes libres al principio, ya que vamos a escribir los resultados
    // en el mismo buffer, y se podrían sobreescribir valores leídos si ha
    // quedado algún parseo a medias de la iteración anterior
    while ((bytes_read = read(0, &buffer[8], sizeof(buffer) - 8)) > 0) {
        i = 0;
        written = 0;
        while (i < bytes_read) {
            // Parseamos un valor del buffer
            while (i < bytes_read && buffer[i+8] != '\n') {
                n = n*10 + buffer[i+8] - '0';
                ++i;
            }

            if (i != bytes_read) { // Lo hemos conseguido parsear
                // n es válido
                if (n == 0) {
                    write(1, buffer, written);
                    return 0;
                }

                unsigned int value = sum[n];

                // Convertimos el valor a su string correspondiente
                unsigned int n_digits = decimal_digits(value);
                unsigned int n_digits_cp = n_digits;
                do {
                    buffer[written + n_digits - 1] = rem10(value) + '0';
                    value = (unsigned int)(((unsigned long long) 0x1999999A)*value >> 32);
                    --n_digits;
                } while (value > 0);

                written += n_digits_cp;
                buffer[written++] = '\n';

                ++i; // Eliminamos el final de línea
                n = 0;
            }
            // else dejamos en n la parte parseada que se acabará de parsear
            //      en la siguiente iteración del bucle externo
        }

        write(1, buffer, written);
    }
}
