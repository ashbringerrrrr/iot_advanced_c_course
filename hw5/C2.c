#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_STACK_SIZE 1000

typedef struct {
    int data[MAX_STACK_SIZE];
    int top;
} Stack;

void init(Stack* s) {
    s->top = -1;
}

void push(Stack* s, int value) {
    s->data[++s->top] = value;
}

int pop(Stack* s) {
    return s->data[s->top--];
}

int is_empty(Stack* s) {
    return s->top == -1;
}

int evaluate_rpn(char* expression) {
    Stack stack;
    init(&stack);
    
    char* token = strtok(expression, " ");
    
    while (token != NULL) {
        size_t len = strlen(token);
        if (token[len - 1] == '.') {
            token[len - 1] = '\0';
        }
        
        if (isdigit(token[0])) {
            push(&stack, atoi(token));
        } else {
            int b = pop(&stack);
            int a = pop(&stack);
            
            switch (token[0]) {
                case '+':
                    push(&stack, a + b);
                    break;
                case '-':
                    push(&stack, a - b);
                    break;
                case '*':
                case '×':
                    push(&stack, a * b);
                    break;
                case '/':
                    push(&stack, a / b);
                    break;
                default:
                    printf("Неизвестный оператор: %s\n", token);
                    exit(1);
            }
        }
        
        token = strtok(NULL, " ");
    }
    
    return pop(&stack);
}

int main() {
    char expression[1000];
    
    fgets(expression, sizeof(expression), stdin);
    
    expression[strcspn(expression, "\n")] = 0;
    
    int result = evaluate_rpn(expression);
    printf("%d\n", result);
    
    return 0;
}