#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#define N 1000000
#define MAX_SOL 159139
#define BUFF_SIZE 16384*2*2

#define SUM_ITER(__i) {\
    sum[__i] = sum[__i-1] + mem[__i];\
}\

unsigned char mem[N+1]; // Use char instead to improve locality
char to_string[MAX_SOL][8] __attribute__ ((aligned (8)));
char sum[N+1][8] __attribute__ ((aligned (8)));

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

int main() {
    unsigned int i, bytes_read, n, written;
    char buffer[BUFF_SIZE];

    memset(mem, 0, sizeof(mem));

    // Memoize the results
    compute_triples_from(3, 4, 5);

    // The memset has been eliminated because it was unnecessary

    // And compute partial sum
    sum[1][0] = sum[2][0] = sum[3][0] = sum[4][0] = 2;
    sum[1][1] = sum[2][1] = sum[3][1] = sum[4][1] = '0';
    sum[1][2] = sum[2][2] = sum[3][2] = sum[4][2] = '\n';
    // N - 5 + 1 is already multiple of 6, so we perform unrolling of that degree
    // Also, the compiler didn't perform unrolling before

    for (i = 0; i < 10; ++i) {
        to_string[i][0] = 2;
        to_string[i][1] = i%10 + '0';
        to_string[i][2] = '\n';
    }

    for (i = 10; i < 100; ++i) {
        to_string[i][0] = 3;
        to_string[i][1] = i/10 + '0';
        to_string[i][2] = i%10 + '0';
        to_string[i][3] = '\n';
    }

    for (i = 100; i < 1000; ++i) {
        to_string[i][0] = 4;
        to_string[i][1] = i/(100) + '0';
        to_string[i][2] = (i/10)%10 + '0';
        to_string[i][3] = i%10 + '0';
        to_string[i][4] = '\n';
    }

    for (i = 1000; i < 10000; ++i) {
        to_string[i][0] = 5;
        to_string[i][1] = i/(1000) + '0';
        to_string[i][2] = (i/100)%10 + '0';
        to_string[i][3] = (i/10)%10 + '0';
        to_string[i][4] = i%10 + '0';
        to_string[i][5] = '\n';
    }

    for (i = 10000; i < 100000; ++i){
        to_string[i][0] = 6;
        to_string[i][1] = i/(10000) + '0';
        to_string[i][2] = (i/1000)%10 + '0';
        to_string[i][3] = (i/100)%10 + '0';
        to_string[i][4] = (i/10)%10 + '0';
        to_string[i][5] = i%10 + '0';
        to_string[i][6] = '\n';
    }

    for (i = 100000; i <= MAX_SOL; ++i) {
        to_string[i][0] = 7;
        to_string[i][1] = i/(100000) + '0';
        to_string[i][2] = (i/10000)%10 + '0';
        to_string[i][3] = (i/1000)%10 + '0';
        to_string[i][4] = (i/100)%10 + '0';
        to_string[i][5] = (i/10)%10 + '0';
        to_string[i][6] = i%10 + '0';
        to_string[i][7] = '\n';
    }

    unsigned int sumprev = 0;
    for (i = 5; i <= N; i++) {
        sumprev += mem[i];
        unsigned long long* d = (unsigned long long*) &sum[i];
        unsigned long long* s = (unsigned long long*) &to_string[sumprev];
        *d = *s;
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

                memcpy(&buffer[written], &sum[n][1], sum[n][0]);
                written += sum[n][0];

                ++i; // Eliminamos el final de línea
                n = 0;
            }
            // else dejamos en n la parte parseada que se acabará de parsear
            //      en la siguiente iteración del bucle externo
        }

        write(1, buffer, written);
    }
}
