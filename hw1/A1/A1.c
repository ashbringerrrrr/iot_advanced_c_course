#include <stdio.h>
#include <ctype.h>

void cipher_input(int shift) {
    char c;
    
    while ((c = getchar()) != '.') {
        if (c >= 'a' && c <= 'z') {
            putchar('a' + (c - 'a' + shift) % 26);
        }
        else if (c >= 'A' && c <= 'Z') {
            putchar('A' + (c - 'A' + shift) % 26);
        }
        else {
            putchar(c);
        }
    }
}

int main() {
    unsigned int shift;
    scanf("%u", &shift);
    
    char c;
    while ((c = getchar()) == ' ' || c == '\n') {}

    if (c != '.') {
        if (c >= 'a' && c <= 'z') {
            putchar('a' + (c - 'a' + shift) % 26);
        }
        else if (c >= 'A' && c <= 'Z') {
            putchar('A' + (c - 'A' + shift) % 26);
        }
        else {
            putchar(c);
        }
        
        cipher_input(shift);
    } 

    putchar('.');
    putchar('\n');
    
    return 0;
}