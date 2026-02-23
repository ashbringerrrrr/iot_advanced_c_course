void btUpView(tree *root) {
    if (root == NULL) return;
    typedef struct queue_node {
        tree *node;
        int hd;
        struct queue_node *next;
    } queue_node;

    queue_node *front = NULL, *rear = NULL;


    void enqueue(tree *node, int hd) {
        queue_node *new = (queue_node*)malloc(sizeof(queue_node));
        new->node = node;
        new->hd = hd;
        new->next = NULL;
        if (rear == NULL) {
            front = rear = new;
        } else {
            rear->next = new;
            rear = new;
        }
    }

    queue_node* dequeue() {
        if (front == NULL) return NULL;
        queue_node *temp = front;
        front = front->next;
        if (front == NULL) rear = NULL;
        return temp;
    }

    typedef struct {
        int hd;
        int key;
    } pair;

    pair *result = NULL;
    int capacity = 0;
    int count = 0;

    enqueue(root, 0);

    while (front != NULL) {
        queue_node *cur = dequeue();
        tree *node = cur->node;
        int hd = cur->hd;
        int found = 0;
        for (int i = 0; i < count; ++i) {
            if (result[i].hd == hd) {
                found = 1;
                break;
            }
        }

        if (!found) {
            if (count >= capacity) {
                capacity = (capacity == 0) ? 10 : capacity * 2;
                result = (pair*)realloc(result, capacity * sizeof(pair));
            }
            result[count].hd = hd;
            result[count].key = node->key;
            ++count;
        }

        if (node->left != NULL)
            enqueue(node->left, hd - 1);
        if (node->right != NULL)
            enqueue(node->right, hd + 1);

        free(cur);
    }
    for (int i = 0; i < count - 1; ++i) {
        for (int j = 0; j < count - i - 1; ++j) {
            if (result[j].hd > result[j + 1].hd) {
                pair tmp = result[j];
                result[j] = result[j + 1];
                result[j + 1] = tmp;
            }
        }
    }
    for (int i = 0; i < count; ++i) {
        printf("%d", result[i].key);
        if (i < count - 1) printf(" ");
    }

    free(result);
}