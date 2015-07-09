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
#include <libcgc.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "stree.h"

extern void db_free(database_t *db);

static database_t *db = NULL;
static row_t *first_row = NULL;
static row_t *last_row = NULL;
static row_t *highest_row = NULL;

#define DIR_LEFT 0
#define DIR_RIGHT 1

#define MAX_OUTPUT (64*1024)
static char *g_output_buf = NULL;
static size_t g_output_len = 0;

result_set_t* reverse_result(result_set_t *root)
{
    result_set_t *head = NULL, *next;
    while (root)
    {
        next = root->next;
        root->next = head;
        head = root;
        root = next;
    }
    return head;
}

char* dtostr(double value, char *str)
{
    /* Length of str array needs to be at least 15 bytes including null-byte */
    int val_int = (int) value;
    sprintf(str, "%d.", val_int);
    value -= val_int;
    value *= 100;
    if (value < 0) value = -1 * value;
    sprintf(str, "%s%02d", str, (int) value);
    return str;
}

int readuntil(int fd, char *buf, size_t len, char delim)
{
    size_t i;
    char *c = buf;
    for (i = 0; i < len; ++i)
    {
        size_t rx;
        if (receive(fd, c, 1, &rx) != 0 || rx == 0)
            break;
        if (*(c++) == delim)
            break;
    }
    *(c-1) = '\0';
    return c - buf;
}

void handle_create_database(char *input)
{
    char *tok;
    tok = strsep(&input, " ");
    if (input || tok == NULL)
    {
        fdprintf(STDOUT, "Invalid database name.\n");
        return;
    }
    if (db)
        fdprintf(STDOUT, "Database (%s) is already created.\n", db->name);
    else
        db = create_database(tok);
}

void handle_create_table(char *input)
{
    table_t *tbl;
    char *tbl_name, *primary_key, *col_name, *col_type;
    tbl_name = strsep(&input, " ");
    if (tbl_name == NULL)
    {
        fdprintf(STDOUT, "Invalid table name.\n");
        return;
    }
    if (db == NULL)
    {
        fdprintf(STDOUT, "Create a database first!\n");
        return;
    }
    if (db->table)
        fdprintf(STDOUT, "Table (%s) is already created.\n", db->table->name);
    else
    {
        primary_key = strsep(&input, " ");
#ifdef PATCHED
        if (primary_key == NULL || *primary_key == '(' || input == NULL)
#else
        if (primary_key == NULL || *primary_key == '(')
#endif
        {
            fdprintf(STDOUT, "Missing a primary key field!\n");
            return;
        }

#ifdef PATCHED
        if (input == NULL) {
            return;
        }
        if (strlen(input) < 1) {
            return;
        }
#endif

        if (*input == '(' && input[strlen(input) - 1] == ')')
        {
            int found_pk = 0;
            tbl = create_table(tbl_name);

            input++;
            input[strlen(input) - 1] = '\0';

            while (1)
            {
                char *tok;
                tok = strsep(&input, ",");
                if (tok == NULL) break;
                col_name = strsep(&tok," ");
                if (col_name == NULL || tok == NULL)
                {
                    fdprintf(STDOUT, "Invalid syntax.\n");
                    tbl_free(tbl);
                    return;
                }

                int i;
                for (i = 0; i < tbl->num_fields; ++i)
                {
                    if (strcmp(col_name, tbl->fields[i]->name) == 0)
                    {
                        fdprintf(STDOUT, "Duplicate column found!\n");
                        tbl_free(tbl);
                        return;
                    }
                }

                col_type = tok;
                int ret = tbl_add_field(tbl, col_name, col_type);
                if (ret != 0)
                {
                    tbl_free(tbl);
                    return;
                }
                if (strcmp(col_name, primary_key) == 0)
                {
                    tbl->pk_idx = tbl->num_fields - 1;
                    found_pk = 1;
                }
                if (tbl->num_fields > MAX_COLUMNS)
                {
                    fdprintf(STDOUT, "Max number of columns is %d.\n", MAX_COLUMNS);
                    tbl_free(tbl);
                    return;
                }
            }

            if (tbl->num_fields == 0)
            {
                fdprintf(STDOUT, "Needs to have more than 1 column.\n");
                tbl_free(tbl);
                return;
            }

            if (!found_pk)
            {
                fdprintf(STDOUT, "Given primary key field not found.\n");
                tbl_free(tbl);
                return;
            }

            db_set_table(db, tbl);
        }
        else
        {
            fdprintf(STDOUT, "Invalid syntax.\n");
        }
    }
}

void handle_insert_into(char *input)
{
    char *tok, *tbl_name, *value;
    tbl_name = strsep(&input, " ");
    if (input == NULL || tbl_name == NULL)
    {
        fdprintf(STDOUT, "Invalid syntax!\n");
        return;
    }
    if (db == NULL || db->table == NULL)
    {
        fdprintf(STDOUT, "Create a database and a table first!\n");
        return;
    }

    if (strcmp(db->table->name, tbl_name) != 0)
    {
        fdprintf(STDOUT, "Table not found!\n");
        return;
    }

    tok = strsep(&input, " ");
    if (tok == NULL || strcmp(tok, "VALUES") != 0 || input == NULL)
    {
        fdprintf(STDOUT, "Invalid syntax!\n");
        return;
    }

    if (*input == '(' && input[strlen(input) - 1] == ')')
    {
        input++;
        input[strlen(input) - 1] = '\0';

        int num_values = 0;
        row_t *row = (row_t *) malloc(sizeof(row_t));
        row->columns = (data_t **) malloc(MAX_COLUMNS * sizeof(data_t *));
        while (row && num_values < db->table->num_fields)
        {
            value = strsep(&input, ",");
            if (value == NULL) break;

            data_t *data = (data_t *) malloc(sizeof(data_t));
            type_t type = db->table->fields[num_values]->type;
            if (type == INTEGER)
            {
                char *tok;
                data->type = INTEGER;
                data->value.i = strtol(value, &tok, 0);
            }
            else if (type == FLOAT)
            {
                char *tok;
                data->type = FLOAT;
                data->value.f = strtod(value, &tok);
            }
            else if (type == VARCHAR)
            {
                data->type = VARCHAR;
                data->value.s = (char *) malloc(strlen(value) + 1);
                strcpy(data->value.s, value);
            }
            row->columns[num_values] = data;
            num_values++;
        }

        row->num_columns = num_values;
        row->pk_idx = db->table->pk_idx;

        if (num_values != db->table->num_fields)
        {
            fdprintf(STDOUT, "Not enough values are given.\n");
            destroy_row(row);
            return;
        }

        db_insert_row(db, row);
        db->num_queries++;
        row_t *high = db_select_max(db);
        highest_row = high;
        fdprintf(STDOUT, "1 row inserted.\n");
    }
    else
    {
        fdprintf(STDOUT, "Invalid syntax.\n");
    }
}

void handle_delete_from(char *input)
{
    char *tok, *tbl_name, *column_name, *op, *value;
    tbl_name = strsep(&input, " ");
    if (input == NULL || tbl_name == NULL)
    {
        fdprintf(STDOUT, "Invalid syntax!\n");
        return;
    }
    if (db == NULL || db->table == NULL)
    {
        fdprintf(STDOUT, "Create a database and a table first!\n");
        return;
    }

    if (strcmp(db->table->name, tbl_name) != 0)
    {
        fdprintf(STDOUT, "Table not found!\n");
        return;
    }

    tok = strsep(&input, " ");
    if (tok == NULL || strcmp(tok, "WHERE") != 0)
    {
        fdprintf(STDOUT, "Invalid syntax!\n");
        return;
    }

    column_name = strsep(&input, " ");
    if (column_name == NULL)
    {
        fdprintf(STDOUT, "Invalid syntax!\n");
        return;
    }

    op = strsep(&input, " ");
    if (op == NULL || (strcmp(op, ">") && strcmp(op, ">=") &&
            strcmp(op, "<") && strcmp(op, "<=") && strcmp(op, "=")))
    {
        fdprintf(STDOUT, "Invalid syntax!\n");
        return;
    }

    value = strsep(&input, " ");
    if (value == NULL || input != NULL)
    {
        fdprintf(STDOUT, "Invalid syntax!\n");
        return;
    }

    type_t type;
    int i, found = 0;
    for (i = 0; i < db->table->num_fields; ++i)
    {
        if (strcmp(db->table->fields[i]->name, column_name) == 0)
        {
            found = 1;
            type = db->table->fields[i]->type;
            break;
        }
    }

    if (!found)
    {
        fdprintf(STDOUT, "Given column name not found.\n");
        return;
    }

    if (i != db->table->pk_idx)
    {
        fdprintf(STDOUT, "You can only select on primary key!\n");
        return;
    }

    data_t *data = (data_t *) malloc(sizeof(data_t));
    data->type = type;
    if (type == INTEGER)
    {
        data->value.i = strtol(value, &tok, 0);
    }
    else if (type == FLOAT)
    {
        data->value.f = strtod(value, &tok);
    }
    else if (type == VARCHAR)
    {
        data->value.s = value;
    }

    result_set_t *cur, *result = (result_set_t *) malloc(sizeof(result_set_t));
    row_t *row, *find;

    find = (row_t *) malloc(sizeof(row_t));
    find->pk_idx = db->table->pk_idx;
    find->columns = (data_t **) malloc((find->pk_idx + 1) * sizeof(data_t *));
    find->columns[find->pk_idx] = data;
    row = db_select_row(db, find);

    if (db->table->rows && db->table->rows->root)
    {
        if (strcmp(op, "=") == 0)
        {
            result->row = row;
            result->next = NULL;
        }
        else if (strcmp(op, ">") == 0)
        {
            if (db->table->rows->size > 0)
            {
                row_t *root_row = db->table->rows->root->elem;
                if (db->table->rows->compare(root_row, find) > 0)
                {
                    result->row = root_row;
                    result->next = NULL;
                }
            }
            append_rows(db->table->rows, &result, DIR_RIGHT);
        }
        else if (strcmp(op, ">=") == 0)
        {
            row_t *root_row = db->table->rows->root->elem;
            if (row || db->table->rows->compare(root_row, find) > 0)
            {
                result->row = root_row;
                result->next = NULL;
            }
            append_rows(db->table->rows, &result, DIR_RIGHT);
        }
        else if (strcmp(op, "<") == 0)
        {
            append_rows(db->table->rows, &result, DIR_LEFT);
            if (db->table->rows->size > 0)
            {
                row_t *root_row = db->table->rows->root->elem;
                if (db->table->rows->compare(root_row, find) < 0)
                {
                    result_set_t *new = (result_set_t *) malloc(sizeof(result_set_t));
                    new->row = root_row;
                    if (result->next == NULL)
                        new->next = NULL;
                    else
                        new->next = result;
                    result = new;
                }
            }
            result = reverse_result(result);
        }
        else if (strcmp(op, "<=") == 0)
        {
            append_rows(db->table->rows, &result, DIR_LEFT);
            row_t *root_row = db->table->rows->root->elem;
            if (row || db->table->rows->compare(root_row, find) < 0)
            {
                result_set_t *new = (result_set_t *) malloc(sizeof(result_set_t));
                new->row = root_row;
                if (result->next == NULL)
                    new->next = NULL;
                else
                    new->next = result;
                result = new;
            }
            result = reverse_result(result);
        }
    }

    int j, num_deleted = 0;
    if (result->row)
    {
        cur = result;
        while (cur)
        {
            row = cur->row;
            db_delete_row(db, row);
#ifdef PATCHED
            if (row == highest_row)
                highest_row = db_select_max(db);
#endif
            for (j = 0; j < row->num_columns; ++j)
            {
                data_t *col = row->columns[j];
                if (col->type == VARCHAR)
                    free(col->value.s);
                free(col);
            }
            result_set_t *tmp = cur;
            cur = cur->next;
            free(row->columns);
            free(tmp);
            free(row);
            num_deleted++;
        }
    }
    db->num_queries++;
    fdprintf(STDOUT, "%d row(s) deleted.\n", num_deleted);

    free(find->columns);
    free(find);
    free(data);
}

void handle_select_from(char *input)
{
    char *tok, *tbl_name, *column_name, *op, *value;
    tbl_name = strsep(&input, " ");
    if (input == NULL || tbl_name == NULL)
    {
        fdprintf(STDOUT, "Invalid syntax!\n");
        return;
    }
    if (db == NULL || db->table == NULL)
    {
        fdprintf(STDOUT, "Create a database and a table first!\n");
        return;
    }

    if (strcmp(db->table->name, tbl_name) != 0)
    {
        fdprintf(STDOUT, "Table not found!\n");
        return;
    }

    tok = strsep(&input, " ");
    if (tok == NULL || strcmp(tok, "WHERE") != 0)
    {
        fdprintf(STDOUT, "Invalid syntax!\n");
        return;
    }

    column_name = strsep(&input, " ");
    if (column_name == NULL)
    {
        fdprintf(STDOUT, "Invalid syntax!\n");
        return;
    }

    op = strsep(&input, " ");
    if (op == NULL || (strcmp(op, ">") && strcmp(op, ">=") &&
            strcmp(op, "<") && strcmp(op, "<=") && strcmp(op, "=")))
    {
        fdprintf(STDOUT, "Invalid syntax!\n");
        return;
    }

    value = strsep(&input, " ");
    if (value == NULL || input != NULL)
    {
        fdprintf(STDOUT, "Invalid syntax!\n");
        return;
    }

    type_t type;
    int i, found = 0;
    for (i = 0; i < db->table->num_fields; ++i)
    {
        if (strcmp(db->table->fields[i]->name, column_name) == 0)
        {
            found = 1;
            type = db->table->fields[i]->type;
            break;
        }
    }

    if (!found)
    {
        fdprintf(STDOUT, "Given column name not found.\n");
        return;
    }

    if (i != db->table->pk_idx)
    {
        fdprintf(STDOUT, "You can only select on primary key!\n");
        return;
    }

    data_t *data = (data_t *) malloc(sizeof(data_t));
    data->type = type;
    if (type == INTEGER)
    {
        data->value.i = strtol(value, &tok, 0);
    }
    else if (type == FLOAT)
    {
        data->value.f = strtod(value, &tok);
    }
    else if (type == VARCHAR)
    {
        data->value.s = value;
    }

    result_set_t *cur, *result = (result_set_t *) malloc(sizeof(result_set_t));
    row_t *row, *find;

    find = (row_t *) malloc(sizeof(row_t));
    find->pk_idx = db->table->pk_idx;
    find->columns = (data_t **) malloc((find->pk_idx + 1) * sizeof(data_t *));
    find->columns[find->pk_idx] = data;
    row = db_select_row(db, find);

    if (db->table->rows && db->table->rows->root)
    {
        if (strcmp(op, "=") == 0)
        {
            result->row = row;
            result->next = NULL;
        }
        else if (strcmp(op, ">") == 0)
        {
            if (db->table->rows->size > 0)
            {
                row_t *root_row = db->table->rows->root->elem;
                if (db->table->rows->compare(root_row, find) > 0)
                {
                    result->row = root_row;
                    result->next = NULL;
                }
            }
            append_rows(db->table->rows, &result, DIR_RIGHT);
        }
        else if (strcmp(op, ">=") == 0)
        {
            row_t *root_row = db->table->rows->root->elem;
            if (row || db->table->rows->compare(root_row, find) > 0)
            {
                result->row = root_row;
                result->next = NULL;
            }
            append_rows(db->table->rows, &result, DIR_RIGHT);
        }
        else if (strcmp(op, "<") == 0)
        {
            append_rows(db->table->rows, &result, DIR_LEFT);
            if (db->table->rows->size > 0)
            {
                row_t *root_row = db->table->rows->root->elem;
                if (db->table->rows->compare(root_row, find) < 0)
                {
                    result_set_t *new = (result_set_t *) malloc(sizeof(result_set_t));
                    new->row = root_row;
                    if (result->next == NULL)
                        new->next = NULL;
                    else
                        new->next = result;
                    result = new;
                }
            }
            result = reverse_result(result);
        }
        else if (strcmp(op, "<=") == 0)
        {
            append_rows(db->table->rows, &result, DIR_LEFT);
            row_t *root_row = db->table->rows->root->elem;
            if (row || db->table->rows->compare(root_row, find) < 0)
            {
                result_set_t *new = (result_set_t *) malloc(sizeof(result_set_t));
                new->row = root_row;
                if (result->next == NULL)
                    new->next = NULL;
                else
                    new->next = result;
                result = new;
            }
            result = reverse_result(result);
        }
    }

    int j, num_selected = 0;
    for (j = 0; j < db->table->num_fields; ++j)
    {
        fdprintf(STDOUT, "|%s\t\t", db->table->fields[j]->name);
    }
    fdprintf(STDOUT, "|\n");

    if (result->row)
    {
        cur = result;
        while (cur)
        {
            row = cur->row;
            print_row(row);
            result_set_t *tmp = cur;
            cur = cur->next;
            free(tmp);
            num_selected++;
        }
    }
    db->num_queries++;
    fdprintf(STDOUT, "%d row(s) selected.\n", num_selected);

    free(find->columns);
    free(find);
    free(data);
}

void handle_stats(char *input)
{
    if (db && db->table)
    {
        fdprintf(STDOUT, "\nStatistics\n");
        fdprintf(STDOUT, "---------------------------------------\n");
        fdprintf(STDOUT, "Database name: %s\n", db->name);
        fdprintf(STDOUT, "Table name: %s\n", db->table->name);
        if (db->table->rows)
            fdprintf(STDOUT, "# rows: %d\n", db->table->rows->size);
        fdprintf(STDOUT, "# queries: %d\n", db->num_queries);
        fdprintf(STDOUT, "Columns:\n");
        int i;
        for (i = 0; i < db->table->num_fields; ++i)
        {
            fdprintf(STDOUT, "|%s\t", db->table->fields[i]->name);
        }
        if (db->table->num_fields > 0)
            fdprintf(STDOUT, "|\n");
        fdprintf(STDOUT, "Primary Key Column Index: %d\n", db->table->pk_idx);
        fdprintf(STDOUT, "Highest Row:\n");
        if (db->table->rows && db->table->rows->size >= 5 && highest_row)
            print_row(highest_row);
        else
            fdprintf(STDOUT, "N/A\n");
        db->num_queries++;
        fdprintf(STDOUT, "---------------------------------------\n\n");
    }
    else
    {
        fdprintf(STDOUT, "Create a database and a table first!\n");
    }
}

int main()
{
    char buf[8192];
    g_output_buf = malloc(MAX_OUTPUT);
    if (g_output_buf == NULL)
    {
        fdprintf(STDOUT, "Failed to allocate output buffer.\n");
        return -1;
    }

    while (readuntil(STDIN, buf, sizeof(buf), '\n') > 0)
    {
        char *tok, *input = buf;
        tok = strsep(&input, " ");
        if (tok == NULL)
            break;
        if (strcmp(tok, "CREATE") == 0)
        {
            tok = strsep(&input, " ");
            if (tok == NULL)
                break;
            if (strcmp(tok, "DATABASE") == 0)
                handle_create_database(input);
            else if (strcmp(tok, "TABLE") == 0)
                handle_create_table(input);
            else
                break;
        }
        else if (strcmp(tok, "INSERT") == 0)
        {
            tok = strsep(&input, " ");
            if (tok == NULL)
                break;
            if (strcmp(tok, "INTO") == 0)
                handle_insert_into(input);
        }
        else if (strcmp(tok, "DELETE") == 0)
        {
            tok = strsep(&input, " ");
            if (tok == NULL)
                break;
            if (strcmp(tok, "FROM") == 0)
                handle_delete_from(input);
        }
        else if (strcmp(tok, "SELECT") == 0)
        {
            tok = strsep(&input, " ");
            if (tok == NULL)
                break;
            if (strcmp(tok, "FROM") == 0)
                handle_select_from(input);
        }
        else if (strcmp(tok, "STATS") == 0)
        {
            handle_stats(input);
        }
        else
            fdprintf(STDOUT, "Invalid query!\n");
    }

    db_free(db);
    return 0;
}
