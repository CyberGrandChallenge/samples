/* Splay Tree - stree.h */

#ifndef STREE_H
#define STREE_H

typedef int (*st_cmp_t) (void *e1, void *e2);

typedef struct node {
    struct node *left;
    struct node *right;
    void  *elem;
} node_t;

typedef struct splay_tree {
    node_t      *root;
    int         size;
    st_cmp_t    compare;
} splay_tree_t;

splay_tree_t* splay_tree_new(st_cmp_t cmp);
void splay_tree_destroy(splay_tree_t *tree);
node_t* st_insert(splay_tree_t *t, void *x);
node_t* st_remove(splay_tree_t *t, void *x);
node_t* st_find(splay_tree_t *t, void *x);
node_t* st_max(splay_tree_t *t);

/* Used for debugging */
void print_tree(node_t *node);

#endif  /* STREE_H */
