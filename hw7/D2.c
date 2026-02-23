tree * findBrother(tree *root, int key) {
    if (root == NULL) return NULL;
    

    tree *current = root;
    while (current != NULL) {
        if (key == current->key) {
            break;
        } else if (key < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    if (current == NULL || current->parent == NULL) {
        return NULL;
    }

    tree *parent = current->parent;

    if (parent->left == current) {
        return parent->right;
    }
    else if (parent->right == current) {
        return parent->left;
    }
    
    return NULL;
}