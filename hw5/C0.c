#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int main() {
    char s[10001];
    scanf("%s", s);
    int len = strlen(s);
    
    bool found[1000] = {false};
    
    for (int num = 100; num <= 999; num++) {
        char d1 = '0' + num / 100;
        char d2 = '0' + (num / 10) % 10;
        char d3 = '0' + num % 10;
        
        int pos = 0;
        
        while (pos < len && s[pos] != d1) pos++;
        if (pos >= len) continue;
        pos++;
        
        while (pos < len && s[pos] != d2) pos++;
        if (pos >= len) continue;
        pos++;
        
        while (pos < len && s[pos] != d3) pos++;
        if (pos >= len) continue;
        
        found[num] = true;
    }
    
    int count = 0;
    for (int i = 100; i <= 999; i++) {
        if (found[i]) count++;
    }
    
    printf("%d\n", count);
    return 0;
}