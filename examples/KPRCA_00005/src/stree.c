/*
 * Author: Brian Pak <brian.pak@kapricasecurity.com>
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
/* Splay Tree - stree.c */
#include "stdlib.h"
#include "stree.h"
#include "database.h"
#include <libcgc.h>

/* Helper/internal functions */
void _splay( void *x, node_t *node, splay_tree_t *t)
{
    node_t N;
    node_t *left, *right;
    node_t *tmp;

    if (node == NULL)
        return;
    N.left = N.right = NULL;
    left = right = &N;

    while (1)
    {
        int res = (*t->compare)(node->elem, x);
        if (res == 0)
            break;
        else if (res > 0)
        {
            if (node->left == NULL)
                break;
            if ((*t->compare)(node->left->elem, x) > 0)
            {
                tmp = node->left;
                node->left = tmp->right;
                tmp->right = node;
                node = tmp;
                if (node->left == NULL)
                    break;
            }
            right->left = node;
            right = node;
            node = node->left;
        }
        else
        {
            if (node->right == NULL)
                break;
            if ((*t->compare)(node->right->elem, x) < 0)
            {
                tmp = node->right;
                node->right = tmp->left;
                tmp->left = node;
                node = tmp;
                if (node->right == NULL)
                    break;
            }
            left->right = node;
            left = node;
            node = node->right;
        }
    }
    left->right = node->left;
    right->left = node->right;
    node->left = N.right;
    node->right = N.left;
    t->root = node;
}

void splay( void *x, splay_tree_t *t)
{
    _splay(x, t->root, t);
}

/* Exposed functions - tree operation */
splay_tree_t* splay_tree_new(st_cmp_t cmp)
{
    splay_tree_t *tree = (splay_tree_t *) malloc(sizeof(splay_tree_t));
    if (tree == NULL)
        return NULL;

    tree->root = NULL;
    tree->size = 0;
    tree->compare = cmp;
    return tree;
}

void _splay_tree_destroy_helper(node_t *node)
{
    if (node && node != node->left)
    {
        _splay_tree_destroy_helper(node->left);
        row_t *row = (row_t *) node->elem;
        destroy_row(row);
        _splay_tree_destroy_helper(node->right);
    }
}

void splay_tree_destroy(splay_tree_t *tree)
{
    if (tree)
    {
        if (tree->root)
        {
            _splay_tree_destroy_helper(tree->root);
        }
        free(tree);
    }
}

node_t* st_insert(splay_tree_t *t,  void *x)
{
    node_t *node = (node_t *) malloc(sizeof(node_t));
    if (node == NULL)
        return NULL;
    node->elem = x;
    if (t->root == NULL)
    {
        node->left = node->right = NULL;
        t->root = node;
        t->size = 1;
    }
    else
    {
        splay(x, t);
        int res = (*t->compare)(t->root->elem, x);
        if (res == 0)
        {
            free(node);
            //node->left = t->root->left;
            //node->right = t->root->right;
            //node->elem = x;
            //free(t->root);
            //t->root = node;
            return t->root;
        }
        else if (res > 0)
        {
            node->left = t->root->left;
            node->right = t->root;
            t->root->left = NULL;
        }
        else
        {
            node->right = t->root->right;
            node->left = t->root;
            t->root->right = NULL;
        }
        t->root = node;
        t->size++;
    }
    return t->root;
}

node_t* st_remove(splay_tree_t *t,  void *x)
{
    node_t *new_root, *ret = NULL;
    node_t *left, *right;
    if (t->size == 0)
        return NULL;
    splay(x, t);
    if ((*t->compare)(t->root->elem, x) == 0)
    {
        left = t->root->left;
        right = t->root->right;
        if (t->root->left == NULL)
        {
            new_root = right;
        }
        else
        {
            new_root = left;
            if (right)
            {
                while (left->right)
                    left = left->right;
                left->right = right;
            }
        }
        ret = t->root;
        t->root = new_root;
        t->size--;
    }
    return ret;
}

node_t* st_find(splay_tree_t *t,  void *x)
{
    if (t->size == 0)
        return NULL;
    splay(x, t);
    if ((*t->compare)(t->root->elem, x) == 0)
        return t->root;
    else
        return NULL;
}

node_t* st_max(splay_tree_t *t)
{
    if (t->size == 0)
        return NULL;
    node_t *max = NULL;
    node_t *cur = t->root;
    while (cur)
    {
        if (cur)
        {
            max = cur;
        }
        cur = cur->right;
    }
    return max;
}

#ifdef DEBUG
/* For debugging purpose */
void print_tree(node_t *node)
{
    if (node && node != node->left)
    {
        print_tree(node->left);
        row_t *r = (row_t *) node->elem;
        data_t *d = r->columns[r->pk_idx];
        debug_printf("%d\n", d->value.i);
        print_tree(node->right);
    }
}
#endif
