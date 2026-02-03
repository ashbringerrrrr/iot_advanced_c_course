#include <stdio.h>

int countBits(unsigned int n) {
    int count = 0;
    while (n > 0) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}

int main() {
    unsigned int N;
    scanf("%u", &N);
    
    int result = countBits(N);
    printf("%d\n", result);
    
    return 0;
}