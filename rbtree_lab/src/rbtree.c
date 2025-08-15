#include "rbtree.h"

#include <stdlib.h>

rbtree *new_rbtree(void) {
  // TODO: initialize struct if needed
  rbtree *t = calloc(1, sizeof(*t));
  node_t *nil = calloc(1, sizeof(*nil));
  nil->color = RBTREE_BLACK;
  nil->parent = nil->left = nil->right = nil;
  t->nil = nil;
  t->root = nil;
  return t;
}

void delete_rbtree(rbtree *t) {
  // TODO: reclaim the tree nodes's memory
  free(t);
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  // TODO: implement insert
  if (!t) return NULL;

  // node 초기 설정
  node_t *node = calloc(1, sizeof(*node));
  node->left = node->right = node->parent = t->nil;
  node->color = (t->root == t->nil ? RBTREE_BLACK : RBTREE_RED);
  node->key = key;

  // 트리가 비어있으면 바로 루트로 삼고 함수 종료
  if (t->root == t->nil)
  {
    t->root = node;
    return node;
  }

  // BST 삽입 먼저 구현
  node_t *parent = t->nil;
  node_t *tmp = t->root;
  while (tmp != t->nil) {
    parent = tmp;
    tmp = (key < tmp->key) ? tmp->left : tmp->right;
  }

  node->parent = parent;
  if (key < parent->key) {
    parent->left = node;
  } else {
    parent->right = node;
  }

  if (node->parent->color == RBTREE_RED) 
  {
    // insert_fixup(t, node);
  }

  return node;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  // TODO: implement find
  return t->root;
}

node_t *rbtree_min(const rbtree *t) {
  // TODO: implement find
  return t->root;
}

node_t *rbtree_max(const rbtree *t) {
  // TODO: implement find
  return t->root;
}

int rbtree_erase(rbtree *t, node_t *p) {
  // TODO: implement erase
  return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  // TODO: implement to_array
  return 0;
}
