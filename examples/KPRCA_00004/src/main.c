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
/*
 * Commands (newline delimited):
 *  UPLOAD
 *  DOWNLOAD
 *  ARCHIVE
 *  LIST
 *  SEARCH
 *  REMOVE
 *  QUIT
 *
 * Upload file format:
 *  magic (0xABCD0123)
 *  checksum (ADLER32)
 *  length (uint32_t)
 *  filename (asciz)
 *  data
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "viper32.h"
#include "tree.h"

#define MAX_FILES 1000
#define MAX_FILE_LEN 8192
#define PATH_LEN 256

typedef struct
{
#define HDR_MAGIC 0xABCD0123
    uint32_t magic;
    uint32_t cksum;
    uint32_t length;
} file_hdr_t;

typedef struct
{
    tree_node_t tree_node;
    void *data;
    size_t datalen;
    char filename[PATH_LEN];
} file_node_t;
#define FILE_NODE(x) ((file_node_t*)((intptr_t)x + offsetof(file_node_t, tree_node)))

static tree_t tree;

static void handle_upload();
static void handle_download();
static void handle_archive();
static void handle_list();
static void handle_search();
static void handle_remove();

static int recvall(int fd, void *buf, size_t cnt, size_t *cntout)
{
    size_t tmp;
    *cntout = 0;
    do {
        if (receive(STDIN, buf, cnt - *cntout, &tmp) != 0 || tmp == 0)
            return 1;
        buf += tmp;
        *cntout += tmp;
    } while (*cntout < cnt);
    return 0;
}

static int compare(tree_node_t *a, tree_node_t *b)
{
    int ret = memcmp(a->key, b->key, a->keylen < b->keylen ? a->keylen : b->keylen);
    return ret == 0 ? a->keylen - b->keylen : ret;
}

int main()
{
    tree_init(&tree, compare);

    char cmd[32];
    while (1)
    {
        if (readuntil(STDIN, cmd, sizeof(cmd), '\n') != 0)
            break;
        if (strcmp(cmd, "UPLOAD\n") == 0)
            handle_upload();
        else if (strcmp(cmd, "DOWNLOAD\n") == 0)
            handle_download();
        else if (strcmp(cmd, "ARCHIVE\n") == 0)
            handle_archive();
        else if (strcmp(cmd, "LIST\n") == 0)
            handle_list();
        else if (strcmp(cmd, "SEARCH\n") == 0)
            handle_search();
        else if (strcmp(cmd, "REMOVE\n") == 0)
            handle_remove();
        else if (strcmp(cmd, "QUIT\n") == 0)
            break;
        else
            fdprintf(STDOUT, "Unknown command\n");
    }
}

static void handle_upload()
{
    file_hdr_t hdr;
    size_t bytes, i;
    char *data;
    if (recvall(STDIN, &hdr, sizeof(hdr), &bytes) != 0 || bytes != sizeof(hdr))
        return;
    if (hdr.magic != HDR_MAGIC)
        return;
    if (hdr.length > MAX_FILE_LEN)
        return;
    data = malloc(hdr.length);
    if (data == NULL)
        return;
    if (recvall(STDIN, data, hdr.length, &bytes) != 0 || bytes != hdr.length)
        goto error;
    if (viper32(data, hdr.length) != hdr.cksum)
        goto error;

    if (tree.count > MAX_FILES)
        goto error;

    // verify filename is terminated and get length
    for (i = 0; i < hdr.length; i++)
        if (data[i] == 0)
            break;

    if (i == hdr.length)
        goto error;
#ifdef PATCHED
    if (i > PATH_LEN-1)
        i = PATH_LEN-1;
#endif
    
    file_node_t *node = malloc(sizeof(file_node_t));
    if (node == NULL)
        goto error;
    memcpy(node->filename, data, i + 1);
    node->tree_node.key = node->filename;
    node->tree_node.keylen = i;
    node->datalen = hdr.length - (i + 1);
    node->data = malloc(node->datalen);
    if (node->data == NULL) {
        free(node);
        goto error;
    }
    memcpy(node->data, data + i + 1, node->datalen);

    tree_insert(&tree, &node->tree_node);

error:
    free(data);
    return;
}

static void handle_download()
{
    char filename[PATH_LEN];
    if (readuntil(STDIN, filename, sizeof(filename), '\0') != 0)
        return;
    tree_node_t *node = tree_lookup(&tree, filename, strlen(filename));
    if (node)
    {
        writeall(STDOUT, &FILE_NODE(node)->datalen, 4);
        writeall(STDOUT, FILE_NODE(node)->data, FILE_NODE(node)->datalen);
    }
}
    
static void tar_checksum(char *tar_header)
{
    unsigned int cksum = 0;
    int i;
    for (i = 0; i < 512; i++)
        cksum += (uint8_t)tar_header[i];
    sprintf(&tar_header[148], "%06o", cksum & 0777777);
    tar_header[154] = 0;
}

static void tar_and_iterate(file_node_t *node)
{
    char tar_header[512];
    size_t bytes;

    if (node == NULL)
        return;

    tar_and_iterate(FILE_NODE(node->tree_node.left));

    memset(tar_header, 0, sizeof(tar_header));
    bytes = strlen(node->filename);
    if (bytes > 100)
        bytes = 100;
    memcpy(tar_header, node->filename, bytes);
    sprintf(tar_header + 100, "%07o %07o %07o %011o %011o         0",
        0644, // file mode
        1000, // owner user id
        1000, // group user id
        node->datalen, // file size
        0); // modification time
    tar_checksum(tar_header);
    writeall(STDOUT, tar_header, sizeof(tar_header));
    writeall(STDOUT, node->data, node->datalen);
    // padding to 512-byte
    memset(tar_header, 0, sizeof(tar_header));
    writeall(STDOUT, tar_header, 512 - (node->datalen % 512));

    tar_and_iterate(FILE_NODE(node->tree_node.right));
}

static void handle_archive()
{
    char padding[512];
    tar_and_iterate(FILE_NODE(tree.root));
    memset(padding, 0, sizeof(padding));
    writeall(STDOUT, padding, sizeof(padding));
    writeall(STDOUT, padding, sizeof(padding));
}

static void print_and_iterate(file_node_t *node)
{
    if (node == NULL)
        return;

    print_and_iterate(FILE_NODE(node->tree_node.left));
    writeall(STDOUT, node->filename, strlen(node->filename) + 1);
    print_and_iterate(FILE_NODE(node->tree_node.right));
}

// list all files
static void handle_list()
{
    print_and_iterate(FILE_NODE(tree.root));
    
    char term = 0;
    writeall(STDOUT, &term, 1);
}

// remove file by filename
static void handle_remove()
{
    char filename[PATH_LEN];
    if (readuntil(STDIN, filename, sizeof(filename), '\0') != 0)
        return;
    tree_node_t *node = tree_lookup(&tree, filename, strlen(filename));
    if (node)
    {
        tree_remove(&tree, node);
        free(FILE_NODE(node)->data);
        free(FILE_NODE(node));
    }
}

static char *memstr(char *s1, size_t s1_len,const char *s2, size_t s2_len)
{
    size_t i, j;
    if (s2_len > s1_len)
        return NULL;

    for (i = 0; i <= s1_len-s2_len; i++)
    {
        for (j = 0; j < s2_len; j++)
            if (s1[i + j] != s2[j])
                break;
        if (j == s2_len)
            return &s1[i];
    }

    return NULL;
}

static void search_and_iterate(file_node_t *node, char *query)
{
    if (node == NULL)
        return;

    search_and_iterate(FILE_NODE(node->tree_node.left), query);
    if (memstr(node->data, node->datalen, query, strlen(query)) != NULL)
        writeall(STDOUT, node->filename, strlen(node->filename) + 1);
    search_and_iterate(FILE_NODE(node->tree_node.right), query);
}

// list all files that contain this search string
static void handle_search()
{
    char query[128];
    if (readuntil(STDIN, query, sizeof(query), '\0') != 0)
        return;
    search_and_iterate(FILE_NODE(tree.root), query);

    char term = 0;
    writeall(STDOUT, &term, 1);
}

