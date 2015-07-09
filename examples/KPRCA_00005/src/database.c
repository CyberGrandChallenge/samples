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
/* Database - database.c */

#include <libcgc.h>
#include "stdlib.h"
#include "string.h"
#include "database.h"

extern char* dtostr(double value, char *str);

/* Utility functions & etc. */
int compare_data(void *e1, void *e2)
{
    row_t *r1 = (row_t *)e1;
    row_t *r2 = (row_t *)e2;

    data_t *d1 = r1->columns[r1->pk_idx];
    data_t *d2 = r2->columns[r2->pk_idx];

    if (d1->type == d2->type)
    {
        type_t type = d1->type;
        if (type == INTEGER)
        {
            if (d1->value.i > d2->value.i)
                return 1;
            else if (d1->value.i < d2->value.i)
                return -1;
            else
                return 0;
        }
        else if (type == FLOAT)
        {
            if (d1->value.f > d2->value.f)
                return 1;
            else if (d1->value.f < d2->value.f)
                return -1;
            else
                return 0;
        }
        else if (type == VARCHAR)
        {
            return strcmp(d2->value.s, d1->value.s);
        }
    }
    return 0;
}

/* Table functions */
table_t* create_table(char *name)
{
    table_t *tbl = (table_t *) malloc(sizeof(table_t));
    if (tbl == NULL)
        return NULL;
    tbl->fields = NULL;
    tbl->rows = NULL;
    tbl->name = (char *) malloc(strlen(name) + 1);
    strcpy(tbl->name, name);
    tbl->num_fields = 0;
    return tbl;
}

void tbl_insert_row(table_t *tbl, row_t *row)
{
    if (tbl && row)
    {
        if (tbl->rows == NULL)
            tbl->rows = splay_tree_new(compare_data);

        if (st_insert(tbl->rows, row) == NULL)
        {
            return;
        }
    }
}

row_t* tbl_select_row(table_t *tbl, row_t *row)
{
    row_t *ret = NULL;
    if (tbl && row)
    {
        if (tbl->rows)
        {
            node_t *node = st_find(tbl->rows, row);
            if (node)
                ret = (row_t *) node->elem;
        }
    }
    return ret;
}

row_t* tbl_select_max(table_t *tbl)
{
    row_t *ret = NULL;
    if (tbl)
    {
        if (tbl->rows)
        {
            node_t *node = st_max(tbl->rows);
            if (node)
                ret = (row_t *) node->elem;
        }
    }
    return ret;
}

row_t* tbl_delete_row(table_t *tbl, row_t *row)
{
    row_t *ret = NULL;
    if (tbl && row)
    {
        if (tbl->rows)
        {
            node_t *node = st_remove(tbl->rows, row);
            if (node)
            {
                ret = (row_t *) node->elem;
                free(node);
            }
        }
    }
    return ret;
}

void tbl_free(table_t *tbl)
{
    if (tbl)
    {
        if (tbl->name)
        {
            free(tbl->name);
            tbl->name = NULL;
        }
        if (tbl->rows)
        {
            splay_tree_destroy(tbl->rows);
        }
        if (tbl->fields)
        {
            int i;
            for (i = 0; i < tbl->num_fields; i++)
            {
                free(tbl->fields[i]->name);
                free(tbl->fields[i]);
            }
            free(tbl->fields);
            tbl->fields = NULL;
        }
        free(tbl);
        tbl = NULL;
    }
}

/* Database functions */
database_t* create_database(char *name)
{
    database_t *db = (database_t *) malloc(sizeof(database_t));
    if (db == NULL)
        return NULL;
    db->num_queries = 0;
    db->table = NULL;
    db->name = (char *) malloc(strlen(name) + 1);
    strcpy(db->name, name);
    return db;
}

void db_set_table(database_t *db, table_t *tbl)
{
    if (db && tbl)
    {
        if (db->table)
            tbl_free(db->table);
        db->table = tbl;
    }
}

void db_free(database_t *db)
{
    if (db)
    {
        if (db->name)
            free(db->name);
        if (db->table)
            tbl_free(db->table);
        free(db);
        db = NULL;
    }
}

void db_insert_row(database_t *db, row_t *row)
{
    tbl_insert_row(db->table, row);
}

row_t* db_select_row(database_t *db, row_t *row)
{
    return tbl_select_row(db->table, row);
}

row_t* db_select_max(database_t *db)
{
    return tbl_select_max(db->table);
}

row_t* db_delete_row(database_t *db, row_t *row)
{
    return tbl_delete_row(db->table, row);
}

int tbl_add_field(table_t *tbl, char *name, char *type)
{
    if (tbl && name && type)
    {
        type_t t;
        if (strcmp(type, "INTEGER") == 0) t = INTEGER;
        else if (strcmp(type, "FLOAT") == 0) t = FLOAT;
        else if (strcmp(type, "VARCHAR") == 0) t = VARCHAR;
        else
        {
            fdprintf(STDOUT, "Unknown type!\n");
            return -1;
        }

        field_t *field = (field_t *) malloc(sizeof(field_t));
        if (field == NULL) return -1;
        field->name = (char *) malloc(strlen(name) + 1);
        if (field->name == NULL)
        {
            free(field);
            return -1;
        }
        strcpy(field->name, name);
        field->type = t;
        if (tbl->fields == NULL)
            tbl->fields = (field_t **) malloc(MAX_COLUMNS * sizeof(field_t *));
        tbl->fields[tbl->num_fields++] = field;
        return 0;
    }
    return -1;
}

void _append_rows_helper(node_t *node, result_set_t **result)
{
    if (node && node != node->left)
    {
        _append_rows_helper(node->left, result);
        row_t *r = (row_t *) node->elem;
        data_t *d = r->columns[r->pk_idx];
        if ((*result)->row == NULL)
        {
            (*result)->row = r;
            (*result)->next = NULL;
        }
        else
        {
            result_set_t *new = (result_set_t *) malloc(sizeof(result_set_t));
            new->row = r;
            new->next = *result;
            *result = new;
        }
        _append_rows_helper(node->right, result);
    }
}

void append_rows(splay_tree_t *tree, result_set_t **result, int direction)
{
    if (tree)
    {
        if (direction == 0)
        {
            _append_rows_helper(tree->root->left, result);
        }
        else if (direction == 1)
        {
            _append_rows_helper(tree->root->right, result);
        }
    }
}

void destroy_row(row_t *row)
{
    if (row)
    {
        if (row->columns)
        {
            int i;
            for (i = 0; i < row->num_columns; ++i)
            {
                data_t *d = row->columns[i];
                if (d)
                {
                    if (d->type == VARCHAR && d->value.s)
                        free(d->value.s);
                    free(d);
                }
            }
            free(row->columns);
        }
        free(row);
    }
}

void print_row(row_t *row)
{
    if (row)
    {
        int i;
        char tmpbuf[1024];
        for (i = 0; i < row->num_columns; ++i)
        {
            data_t *col = row->columns[i];
            if (col->type == INTEGER)
                fdprintf(STDOUT, "|%d\t\t", col->value.i);
            else if (col->type == FLOAT)
                fdprintf(STDOUT, "|%s\t\t", dtostr(col->value.f, tmpbuf));
            else if (col->type == VARCHAR)
                fdprintf(STDOUT, "|%s\t\t", col->value.s);
        }
        if (row->num_columns > 0)
            fdprintf(STDOUT, "|\n");
    }
}
