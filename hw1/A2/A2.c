#include <stdio.h>

unsigned int shift(unsigned int N, unsigned int K) {
    unsigned int result = (N >> K) | (N << (32 - K));
    return result;
}

int main() {
    unsigned int N, K;
    scanf("%u %u", &N, &K);
    unsigned int shifted_result = shift(N, K);
    printf("%u\n", shifted_result);
    return 0;
}