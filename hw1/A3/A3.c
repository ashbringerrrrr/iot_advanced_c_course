#include <stdio.h>

int main() {
    unsigned int N;
    int K;

    scanf("%u %d", &N, &K);
    unsigned int mask = (1 << K) - 1;
    unsigned int result = N & mask;

    printf("%u\n", result);

    return 0;
}