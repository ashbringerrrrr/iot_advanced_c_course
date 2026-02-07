#include <stdio.h>
#include <string.h>

int main() {
    char s1[10001], s2[10001];
    
    scanf("%10000s", s1);
    scanf("%10000s", s2);
    
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    
    int max_prefix_suffix = 0;
    int max_len = (len1 < len2) ? len1 : len2;
    
    for (int l = max_len; l >= 1; l--) {
        if (strncmp(s1, s2 + len2 - l, l) == 0) {
            max_prefix_suffix = l;
            break;
        }
    }
    
    int max_suffix_prefix = 0;
    max_len = (len1 < len2) ? len1 : len2;
    
    for (int l = max_len; l >= 1; l--) {
        if (strncmp(s1 + len1 - l, s2, l) == 0) {
            max_suffix_prefix = l;
            break;
        }
    }
    
    printf("%d %d\n", max_prefix_suffix, max_suffix_prefix);
    return 0;
}