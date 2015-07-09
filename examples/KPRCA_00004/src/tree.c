/*
 * Author: Andrew Wesie <andrew.wesie@kapricasecurity.com>
 * 
 * Copyright (c) 2014 Kaprica Security, Inc.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */
#include <stdlib.h>

#include "tree.h"

double __builtin_log(double x);
#define log __builtin_log

static const double alpha = (double)2/3;

static unsigned int count_nodes(tree_node_t *node)
{
    if (node == NULL)
        return 0;
    return 1 + count_nodes(node->left) + count_nodes(node->right);
}

// returns end of list
static tree_node_t *convert_to_list(tree_node_t **node)
{
    tree_node_t *n = *node, *end_right = n;
    if (n->left)
    {
        tree_node_t *end_left = convert_to_list(&n->left);
        end_left->right = n;
        *node = n->left;
        n->left = NULL;
    }

    if (n->right)
    {
        end_right = convert_to_list(&n->right);
    }

    n->parent = (void *)0x0badbeef;
    return end_right;
}

static tree_node_t *convert_from_list(tree_node_t **node, unsigned int count)
{
    tree_node_t *list, *parent;
    unsigned int left_count, right_count;
    if (count == 0)
    {
        list = *node;
        *node = NULL;
        return list;
    }

    count--;
    left_count = count / 2;
    right_count = count - left_count;

    parent = list = convert_from_list(node, left_count);
    parent->left = *node;
    *node = parent;
    list = convert_from_list(&parent->right, right_count);

    if (parent->left)
        parent->left->parent = parent;
    if (parent->right)
        parent->right->parent = parent;
    return list;
}

static void rebalance(tree_node_t **node, unsigned int count)
{
    if (count == 0)
        return;

    tree_node_t *parent = (*node)->parent;
    convert_to_list(node);
    convert_from_list(node, count);
    (*node)->parent = parent;
}

void tree_init(tree_t *tree, tree_compare_t compare_fn)
{
    tree->compare = compare_fn;
    tree->root = NULL;
    tree->max_size = tree->count = 0;
}

void tree_insert(tree_t *tree, tree_node_t *node)
{
    int depth;
    tree_node_t **n, *parent;
    for (n = &tree->root, parent = NULL, depth = 0; *n != NULL;)
    {
        parent = *n;
        depth++;
        if (tree->compare(node, *n) < 0)
            n = &parent->left;
        else
            n = &parent->right;
    }

    *n = node;
    node->parent = parent;
    node->left = NULL;
    node->right = NULL;

    if (++tree->count > tree->max_size)
        tree->max_size = tree->count;

    if (depth > log(tree->count)/log(1/alpha))
    {
        unsigned int count, parent_count;
        tree_node_t *tmp;
        for (tmp = node, count = 1; tmp->parent != NULL; count = parent_count, tmp = tmp->parent)
        {
            tree_node_t *sibling;
            if (tmp == tmp->parent->left)
                sibling = tmp->parent->right;
            else
                sibling = tmp->parent->left;
            parent_count = 1 + count + count_nodes(sibling);
            if (count > parent_count * alpha + 1)
                break;
        }
        // we either found our scapegoat or we hit the top of the tree
        if (tmp->parent == NULL)
            n = &tree->root;
        else if (tmp == tmp->parent->right)
            n = &tmp->parent->right;
        else
            n = &tmp->parent->left;
        rebalance(n, count);
    }
}

void tree_remove(tree_t *tree, tree_node_t *node)
{
    // remove node
    tree_node_t **n;
    if (node->parent == NULL)
        n = &tree->root;
    else if (node == node->parent->right)
        n = &node->parent->right;
    else
        n = &node->parent->left;

    if (node->left == NULL && node->right == NULL)
    {
        *n = NULL;
    }
    else if (node->left == NULL)
    {
        *n = node->right;
        if (*n)
            (*n)->parent = node->parent;
    }
    else if (node->right == NULL)
    {
        *n = node->left;
        if (*n)
            (*n)->parent = node->parent;
    }
    else
    {
        // swap node with min of right child
        tree_node_t *min;
        for (min = node->right; min->left != NULL; min = min->left) ;

        // unlink min from tree
        // NB min->parent may == node
        if (min->parent->right == min)
            min->parent->right = min->right;
        else
            min->parent->left = min->right;

        if (min->right)
            min->right->parent = min->parent;

        // replace node with min
        *n = min;
        min->left = node->left;
        min->right = node->right;
        min->parent = node->parent;

        // update parents
        min->left->parent = min;
        if (min->right)
            min->right->parent = min;
    }

    if (--tree->count < tree->max_size * alpha)
    {
        rebalance(&tree->root, tree->count);
        tree->max_size = tree->count;
    }
}

tree_node_t *tree_lookup(tree_t *tree, void *key, size_t keylen)
{
    tree_node_t *n, tmp;
    tmp.key = key;
    tmp.keylen = keylen;
    for (n = tree->root; n != NULL;)
    {
        int res = tree->compare(&tmp, n);
        if (res < 0)
            n = n->left;
        else if (res > 0)
            n = n->right;
        else
            break;
    }

    return n;
}

#ifdef DEBUG
static unsigned int tree_test_node(tree_t *tree, tree_node_t *node)
{
    if (node == NULL)
        return 0;

    unsigned int max_depth = 0;
    if (node->left)
    {
        max_depth = tree_test_node(tree, node->left);
        if (tree->compare(node->left, node) > 0)
            debug_printf("Invalid node (left %08x, %08x)\n",
                (unsigned int)node->left->key,
                (unsigned int)node->key);
        if (node->left->parent != node)
            debug_printf("Invalid node parent\n");
    }
    if (node->right)
    {
        unsigned int max_depth2 = tree_test_node(tree, node->right);
        if (max_depth2 > max_depth)
            max_depth = max_depth2;
        if (tree->compare(node, node->right) > 0)
            debug_printf("Invalid node (%08x, right %08x)\n",
                (unsigned int)node->key,
                (unsigned int)node->right->key);
        if (node->right->parent != node)
            debug_printf("Invalid node parent\n");
    }
    return max_depth + 1;
}

void tree_test(tree_t *tree)
{
    unsigned int depth = tree_test_node(tree, tree->root);
    debug_printf("Current depth = %d\n", depth);
    if (depth > log(tree->count)/log(1/alpha)+1)
        debug_printf("Invalid depth (%d, expected max %d)\n", depth,
            (int)(log(tree->count)/log(1/alpha)+1));
}
#endif
