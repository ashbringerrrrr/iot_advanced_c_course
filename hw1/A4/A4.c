#include <stdio.h>
#include <stdint.h>

int main() {
    uint32_t N;
    int K;
    
    scanf("%u %d", &N, &K);
    if (K <= 0 || K > 31) {
        printf("Неверное значение K\n");
        return 1;
    }
    
    uint32_t max_value = 0;
    for (int i = 0; i <= 32 - K; i++) {
        uint32_t mask = ((1 << K) - 1) << i;
        uint32_t value = (N & mask) >> i;
        
        if (value > max_value) {
            max_value = value;
        }
    }
    
    printf("%u\n", max_value);
    
    return 0;
}