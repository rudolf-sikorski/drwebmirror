/*
 * File: avltree.h
 * Description: Simple AVL Tree
 * Author: Rudolf Sikorski <rudolf.sikorski@freenet.de>
 * Revision: Fri, 06 Feb 2015 00:16:00 +0000
 * License: Public Domain
 */
#ifndef AVLTREE_H
#define AVLTREE_H

#include <stdint.h>

/* Key, <name> is comparable value */
typedef struct
{
    const char * name;
    const char * hash;
} avl_key;

/* Node of AVL tree */
typedef struct avl_node_
{
    avl_key key;                /* Key */
    uint8_t height;             /* Subree height */
    struct avl_node_ * left;    /* Left subtree */
    struct avl_node_ * right;   /* Right subtree */
} avl_node;

/* Insert key with name <name> and hash <hash> into subtree with root <root> */
avl_node * avl_insert(avl_node * root, const char * name, const char * hash);

/* Find hash of key with name <name> into subtree with root <root> */
const char * avl_hash(const avl_node * root, const char * name);

/* Delete subtree with root <root> */
void avl_dealloc(avl_node * root);

#endif // AVLTREE_H
