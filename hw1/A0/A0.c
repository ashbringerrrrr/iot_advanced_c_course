#include <stdio.h>

int count_max_value(int N) {
    int max_count = 0;
    int current_max;
    int count;
    
    for (int i = 0; i < N; i++) {
        int num;
        scanf("%d", &num);
        
        if (i == 0 || num > current_max) {
            current_max = num;
            max_count = 1;
        } else if (num == current_max) {
            max_count++;
        }
    }
    
    return max_count;
}

int main() {
    int N;
    scanf("%d", &N);
    
    if (N <= 0) {
        printf("0\n");
        return 0;
    }
    
    int result = count_max_value(N);
    printf("%d\n", result);
    return 0;
}