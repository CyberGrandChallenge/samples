/* Database - database.h */

#ifndef DATABASE_H
#define DATABASE_H

#include "stree.h"

#define MAX_COLUMNS 10

typedef enum {
    INTEGER,
    FLOAT,
    VARCHAR
} type_t;

typedef struct data {
    type_t type;
    union {
        int i;
        double f;
        char *s;
    } value;
} data_t;

typedef struct field {
    char    *name;
    type_t   type;
} field_t;

typedef struct row {
    int     pk_idx;
    data_t  **columns;
    int     num_columns;
} row_t;

typedef struct table {
    char    *name;
    field_t **fields;
    splay_tree_t *rows;
    int     num_fields;
    int     pk_idx;
} table_t;

typedef struct database {
    char    *name;
    int     num_queries;
    table_t *table;
} database_t;

typedef struct result_set {
    row_t *row;
    struct result_set *next;
} result_set_t;

database_t* create_database(char *name);
table_t* create_table(char *name);
void db_set_table(database_t *db, table_t *tbl);
void db_insert_row(database_t *db, row_t *row);
row_t* db_select_row(database_t *db, row_t *row);
row_t* db_select_max(database_t *db);
row_t* db_delete_row(database_t *db, row_t *row);

int tbl_add_field(table_t *tbl, char *name, char *type);
void tbl_free(table_t *tbl);

void append_rows(splay_tree_t *tree, result_set_t **result, int direction);

void destroy_row(row_t *row);
void print_row(row_t *row);

#endif  /* DATABSE_H */
