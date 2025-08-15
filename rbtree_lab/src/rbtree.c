#include "rbtree.h"

#include <stdlib.h>

rbtree *new_rbtree(void) {
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  // TODO: initialize struct if needed
  p->root = NULL;
  
  return p;
}

void delete_rbtree(rbtree *t) {
  // TODO: reclaim the tree nodes's memory
  free(t);
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  // TODO: implement insert
  if (!t) return NULL;

  // node 초기 설정
  node_t *node = (node_t *)calloc(1, sizeof(node_t));
  node->parent = node->left = node->right = NULL;
  node->color = (!(t->root) ? RBTREE_BLACK : RBTREE_RED);
  node->key = key;

  // 트리가 비어있으면 바로 루트로 삼고 함수 종료
  if (!(t->root)) 
  {
    t->root = node;
    return node;
  }

  // BST 삽입 먼저 구현
  node_t *parent = NULL;
  node_t *tmp = t->root;
  while (tmp) {
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
    insert_fixup(t, node);
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
