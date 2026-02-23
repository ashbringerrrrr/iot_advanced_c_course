#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct list {
    char word[20];
    struct list *next;
};

struct list* add_to_list(struct list *head, char *word) {
    struct list *new_node = (struct list*)malloc(sizeof(struct list));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }
    strcpy(new_node->word, word);
    new_node->next = NULL;

    if (head == NULL) {
        return new_node;
    } else {
        struct list *current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
        return head;
    }
}

void swap_elements(struct list *a, struct list *b) {
    char tmp[20];
    strcpy(tmp, a->word);
    strcpy(a->word, b->word);
    strcpy(b->word, tmp);
}

void sort_list(struct list *head) {
    if (head == NULL || head->next == NULL) return;

    int swapped;
    struct list *ptr;
    struct list *last = NULL;

    do {
        swapped = 0;
        ptr = head;
        while (ptr->next != last) {
            if (strcmp(ptr->word, ptr->next->word) > 0) {
                swap_elements(ptr, ptr->next);
                swapped = 1;
            }
            ptr = ptr->next;
        }
        last = ptr;
    } while (swapped);
}

void print_list(struct list *head) {
    if (head == NULL) return;
    struct list *current = head;
    while (current != NULL) {
        printf("%s", current->word);
        if (current->next != NULL) {
            printf(" ");
        }
        current = current->next;
    }
    printf("\n");
}

void delete_list(struct list *head) {
    struct list *current = head;
    while (current != NULL) {
        struct list *next = current->next;
        free(current);
        current = next;
    }
}

int main() {
    char input[1000];

    if (fgets(input, sizeof(input), stdin) == NULL) {
        return 1;
    }
    size_t len = strlen(input);
    if (len > 0 && input[len-1] == '\n') {
        input[len-1] = '\0';
        len--;
    }

    char *dot = strchr(input, '.');
    if (dot != NULL) {
        *dot = '\0';
    }
    struct list *head = NULL;
    char *token = strtok(input, " ");
    while (token != NULL) {
        head = add_to_list(head, token);
        token = strtok(NULL, " ");
    }

    sort_list(head);
    print_list(head);
    delete_list(head);

    return 0;
}