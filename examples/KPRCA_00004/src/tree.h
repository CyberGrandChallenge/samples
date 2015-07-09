typedef struct tree_node
{
    struct tree_node *parent;
    struct tree_node *left, *right;
    void *key;
    size_t keylen;
} tree_node_t;

typedef int (*tree_compare_t) (tree_node_t *a, tree_node_t *b);

typedef struct
{
    tree_node_t *root;
    unsigned int count; // current number of nodes
    unsigned int max_size;
    tree_compare_t compare;
} tree_t;

void tree_init(tree_t *tree, tree_compare_t compare_fn);
void tree_insert(tree_t *tree, tree_node_t *node);
tree_node_t *tree_lookup(tree_t *tree, void *key, size_t keylen);
void tree_remove(tree_t *tree, tree_node_t *node);
void tree_test(tree_t *tree);

