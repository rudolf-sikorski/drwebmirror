/*
 * File: avltree.cpp
 * Description: Simple AVL Tree
 * Author: Rudolf Sikorski <rudolf.sikorski@freenet.de>
 * Revision: Sun, 29 Nov 2015 12:05:00 +0000
 * License: Public Domain
 */
#include <stdlib.h>
#include <string.h>
#include "avltree.h"

/* Create new node with key <key> */
avl_node * new_node(const avl_key * key)
{
    avl_node * new_node = (avl_node *)malloc(sizeof(avl_node));
    new_node->key.name = key->name;
    new_node->key.hash = key->hash;
    new_node->left = new_node->right = NULL;
    new_node->height = 1;
    return new_node;
}

/* Comparison function "less" */
avl_s_t less(const avl_key * key_left, const avl_key * key_right)
{
    return strcmp(key_left->name, key_right->name) < 0 ? 1 : 0;
}

/* Height of subtree with root <root> */
avl_u_t height(const avl_node * root)
{
    return root ? root->height : 0;
}

/* Balance factor of subtree with root <root> */
avl_s_t balance_factor(const avl_node * root)
{
    return (avl_s_t)height(root->right) - (avl_s_t)height(root->left);
}

/* Correction tree height after turns */
void fix_height(avl_node * root)
{
    avl_u_t hl = height(root->left);
    avl_u_t hr = height(root->right);
    root->height = (hl > hr ? hl : hr) + 1;
}

/* Right turn over <a> */
avl_node * rotate_right(avl_node * a)
{
    avl_node * b = a->left;
    a->left = b->right;
    b->right = a;
    fix_height(a);
    fix_height(b);
    return b;
}

/* Big right turn over <a> */
avl_node * rotate_right_big(avl_node * a)
{
    avl_node * b = a->left;
    avl_node * c = b->right;
    a->left = c->right;
    b->right = c->left;
    c->right = a;
    c->left = b;
    fix_height(a);
    fix_height(b);
    fix_height(c);
    return c;
}

/* Left turn over <a> */
avl_node * rotate_left(avl_node * a)
{
    avl_node * b = a->right;
    a->right = b->left;
    b->left = a;
    fix_height(a);
    fix_height(b);
    return b;
}

/* Big left turn over <a> */
avl_node * rotate_left_big(avl_node * a)
{
    avl_node * b = a->right;
    avl_node * c = b->left;
    a->right = c->left;
    b->left = c->right;
    c->left = a;
    c->right = b;
    fix_height(a);
    fix_height(b);
    fix_height(c);
    return c;
}

/* Balancing node <root> */
avl_node * balance(avl_node * root)
{
    fix_height(root);
    if(balance_factor(root) == 2)
    {
        if(balance_factor(root->right) >= 0)
            return rotate_left(root);
        else
            return rotate_left_big(root);
    }
    if(balance_factor(root) == -2)
    {
        if(balance_factor(root->left) <= 0)
            return rotate_right(root);
        else
            return rotate_right_big(root);
    }
    return root;
}

/* Insert key <key> into subtree with root <root> */
avl_node * avl_insert_key(avl_node * root, const avl_key * key)
{
    if(!root)
        return new_node(key);
    if(less(key, & root->key))
        root->left = avl_insert_key(root->left, key);
    else
        root->right = avl_insert_key(root->right, key);
    return balance(root);
}

/* Insert key with name <name> and hash <hash> into subtree with root <root> */
avl_node * avl_insert(avl_node * root, const char * name, const char * hash)
{
    avl_key key;
    key.hash = (char *)malloc((strlen(hash) + 1) * sizeof(char));
    strcpy((char *)(key.hash), hash);
    key.name = (char *)malloc((strlen(name) + 1) * sizeof(char));
    strcpy((char *)(key.name), name);
    return avl_insert_key(root, & key);
}

/* Find key <key> into subtree with root <root> */
const avl_key * avl_find_key(const avl_node * root, const avl_key * key)
{
    if(!root)
        return NULL;
    if(less(key, & root->key))
        return avl_find_key(root->left, key);
    if(less(& root->key, key))
        return avl_find_key(root->right, key);
    return & root->key;
}

/* Find hash of key with name <name> into subtree with root <root> */
const char * avl_hash(const avl_node * root, const char * name)
{
    avl_key key;
    const avl_key * key_finded;
    key.name = name;
    key.hash = NULL;
    key_finded = avl_find_key(root, & key);
    return key_finded ? key_finded->hash : NULL;
}

/* Delete subtree with root <root> */
void avl_dealloc(avl_node * root)
{
    if(!root) return;
    avl_dealloc(root->left);
    avl_dealloc(root->right);
    if(root->key.hash) free((void *)(root->key.hash));
    if(root->key.name) free((void *)(root->key.name));
    free(root);
}
