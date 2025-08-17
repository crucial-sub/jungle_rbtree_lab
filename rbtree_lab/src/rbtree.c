#include "rbtree.h"
#include <assert.h>
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

// 서브트리를 후위순회로 모두 해제
static void free_subtree(rbtree *t, node_t *n) {
  if (!t || !n || n == t->nil) return; // sentinel은 free하지 않음
  free_subtree(t, n->left);
  free_subtree(t, n->right);
  free(n);
}

// 트리 전체 해제
void delete_rbtree(rbtree *t) {
  // TODO: reclaim the tree nodes's memory
  if (!t) return;
  free_subtree(t, t->root);
  free(t->nil); // sentinel은 마지막에 1번만 free
  free(t);
}

// 오른쪽에 nil이 아닌 자식 y가 있는 노드 x에 대한 좌회전 함수
// y가 x의 부모로 올라가며 x는 y의 왼쪽 자식이 되고 
// 기존에 있던 y의 왼쪽 자식은 x의 오른쪽 자식으로 붙는다.
static void rotate_left(rbtree *t, node_t *x) {
  // 전제: x에 nil이 아닌 오른쪽 자식 y가 존재해야 함
  assert(x != t->nil);
  assert(x->right != t->nil);

  node_t *y = x->right;

  // 최종 목표는 x를 y의 자식으로 옮기고 y를 x의 부모로 바꾸는 것이지만
  // 실제로 포인터가 가리키는 대상을 바꾸기 전에 y의 기존 자식, x의 기존 부모에 대한 처리를 먼저 해줘야 한다.

  // y의 기존 자식(왼쪽 자식) 처리
  // y의 왼쪽 자식은 이제 x의 오른쪽에 붙는다.
  x->right = y->left;
  if (y->left != t->nil)
  {
    y->left->parent = x;
  }

  // x의 기존 부모(y의 할아버지) 처리
  // x의 부모에게도 이제 x대신 y를 자식으로 등록해주기
  y->parent = x->parent; // 이제 y의 부모 포인터는 x가 아닌 x의 부모를 가리키게 됨
  if (x->parent == t->nil) { // 부모가 없다는건 루트였다는 뜻이므로 y를 새로운 루트로 등록
    t->root = y;
  } 
  else if (x == x->parent->left) { // y를 x가 있던 정확한 위치로 옮김(x가 왼쪽 자식이었는지 오른쪽 자식이었는지)
    x->parent->left = y;
  } 
  else {
    x->parent->right = y;
  }

  // 기존 부모 자식들에 대한 처리가 끝나면 그제서야 x와 y의 부모자식관계 바꾸기
  y->left = x;
  x->parent = y;
}

// 좌회전 함수와 대칭
static void rotate_right(rbtree *t, node_t *x) {
  assert(x != t->nil);
  assert(x->left != t->nil);

  node_t *y = x->left;

  x->left = y->right;
  if (y->right != t->nil)
  {
    y->right->parent = x;
  }

  y->parent = x->parent;
  if (x->parent == t->nil) {
    t->root = y;
  } 
  else if (x == x->parent->right) {
    x->parent->right = y;
  } 
  else {
    x->parent->left = y;
  }

  y->right = x;
  x->parent = y;
}

static void insert_fixup(rbtree *t, node_t *z) {
  // 부모가 최종적으로 검은색이어야 하므로 부모가 빨간색인 동안 fixup 반복
  while (z->parent->color == RBTREE_RED)
  {
    node_t *p = z->parent;
    node_t *g = p->parent;
    if (p == g->left) // z의 부모가 왼쪽 자식일 때
    {
      node_t *u = g->right; // u는 z의 삼촌
      if (u->color == RBTREE_RED) { // case 1: 삼촌 빨간색
        g->color = RBTREE_RED;
        p->color = RBTREE_BLACK;
        u->color = RBTREE_BLACK;
        z = g;
      } else {
        if (z == p->right) {// case 2: g-p-z 꺾임
          z = p;
          rotate_left(t, z);
          // 회전이 끝나면 부모 조부모 관계가 바뀌므로 포인터 갱신 필요
          p = z->parent;
          g = p->parent;
        }
        // case 3: g-p-z 선형
        p->color = RBTREE_BLACK;
        g->color = RBTREE_RED;
        rotate_right(t, g);
      }
    }
    else // z의 부모가 오른쪽 자식일 때
    {
      node_t *u = g->left; // u는 z의 삼촌
      if (u->color == RBTREE_RED) { // case 1: 삼촌 빨간색
        g->color = RBTREE_RED;
        p->color = RBTREE_BLACK;
        u->color = RBTREE_BLACK;
        z = g;
      } else {
        if (z == p->left) {// case 2: g-p-z 꺾임
          z = p;
          rotate_right(t, z);
          p = z->parent;
          g = p->parent;
        }
        // case 3: g-p-z 선형
        p->color = RBTREE_BLACK;
        g->color = RBTREE_RED;
        rotate_left(t, g);
      }
    }
  }

  t->root->color = RBTREE_BLACK;
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  // TODO: implement insert
  if (!t) return NULL;

  // node 초기 설정
  node_t *node = calloc(1, sizeof(*node));
  if (!node) return NULL;
  node->left = node->right = node->parent = t->nil;
  node->color = (t->root == t->nil ? RBTREE_BLACK : RBTREE_RED);
  node->key = key;

  // 트리가 비어있으면 바로 루트로 삼고 함수 종료
  if (t->root == t->nil)
  {
    t->root = node;
    return node;
  }

  // BST 규칙 삽입 먼저 구현
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

  // BST 규칙 삽입이 끝나면 insert_fixup 함수를 실행시켜 색상 규칙 위반안되도록 트리 수정
  insert_fixup(t, node);

  return node;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  // TODO: implement find
  if (!t) return NULL;

  node_t *tmp = t->root;
  while(tmp != t->nil)
  {
    if (tmp->key < key) {
      tmp = tmp->right;
    } else if (tmp->key > key) {
      tmp = tmp->left;
    } else {
      return tmp;
    }
  }
  return NULL;
}

node_t *rbtree_min(const rbtree *t) {
  if (!t || t->root == t->nil) return NULL;
  node_t *tmp = t->root;
  while(tmp->left != t->nil)
  {
    tmp = tmp->left;
  }
  return tmp;
}

node_t *rbtree_max(const rbtree *t) {
  if (!t || t->root == t->nil) return NULL;
  node_t *tmp = t->root;
  while(tmp->right != t->nil)
  {
    tmp = tmp->right;
  }
  return tmp;
}

// 노드 u 자리에 v 서브트리를 이식 (부모 링크 갱신)
// v가 t->nil이어도 parent를 u->parent로 맞춰둬야 이후 delete-fixup에서 x->parent를 탈 수 있음.
static void rbtree_transplant(rbtree *t, node_t *u, node_t *v) {
  if (u->parent == t->nil) {
    t->root = v;
  } else if (u == u->parent->left) {
    u->parent->left = v;
  } else {
    u->parent->right = v;
  }
  v->parent = u->parent;
}

static void rbtree_erase_fixup(rbtree *t, node_t *x) {

}

/*
rbtree_erase 함수에서는 x,y,z 3가지 포인터가 등장한다.

1. z: 삭제 대상
2. y: 트리에서 실제로 제거되는 노드 (z 또는 z의 successor)
    - z의 자식이 1개 이하일 경우: y는 z 자신.
    - z의 자식이 2개일 경우: y는 z의 후임자(successor).
3. x: y의 유일한 자식(없으면 nil)
    - y가 빠져나간 빈자리를 메꾸기 위해 y의 원래 위치로 이동하게 됨
    

절차
1. BST 규칙에 따라 z를 트리에서 제거한다. 이 과정에서 y와 x 포인터가 결정된다.
2. 제거된 노드 y의 원래 색이 BLACK이었다면, 트리 속성이 깨졌을 수 있다.
   이때 rbtree_erase_fixup(t, x)를 호출하여 속성을 복구한다.
*/ 
int rbtree_erase(rbtree *t, node_t *z) {
  if (!t || z == t->nil) return 0;

  node_t *y = z;
  node_t *x = t->nil;
  color_t y_origin_color = y->color;

  // case 1: z의 왼쪽이 NIL → 오른쪽으로 교체
  if (z->left == t->nil) {
    x = z->right;
    rbtree_transplant(t,z,z->right);
  } 
  // case 2: z의 오른쪽이 NIL → 왼쪽으로 교체
  else if (z->right == t->nil) {
    x = z->left;
    rbtree_transplant(t,z,z->left);
  } 
  // 위 처리 결과 노드 z는 직접 제거되며 x는 z의 위치로 올라온 노드가 된다.(z에 자식이 없었다면 NIL)

  // case 3: 양쪽 자식 존재 → 후임 노드로 대체
  else {
    // 후임 노드 찾기(y가 가리키게 만듬)
    y = z->right;                              
    while (y->left != t->nil) {
      y = y->left;
    }
    y_origin_color = y->color;          // 트리에서 빠져나올 y의 색을 미리 저장
    x = y->right;                       // x는 y의 유일한 자식 (successor는 왼쪽 자식이 없으므로)

    // y를 원래 위치에서 제거하기
    if (y->parent == z) {
      // y가 z의 바로 오른쪽 자식인 경우,
      // x가 NIL 노드일 수 있으므로, 부모 포인터를 명시적으로 설정
      x->parent = y; // x가 NIL일 때도 x의 부모는 y여야 하므로 이를 명시해줘야한다.
    } else {
      // y가 z의 오른쪽 서브트리 깊숙이 있는 경우,
      // 1. y의 원래 위치를 x로 대체하여 y를 트리에서 분리
      rbtree_transplant(t, y, x);
      // 2. y가 z의 자리를 차지할 준비: z의 오른쪽 서브트리를 y에게 넘김
      y->right = z->right;
      y->right->parent = y;
    }
    
    // 공통 후처리: z 자리에 y 올리고, z의 왼쪽 서브트리를 y.left로, 색은 z의 색 유지
    rbtree_transplant(t,z,y);
    y->left = z->left;
    y->left->parent = y;
    y->color = z->color;
  }

  // 제거된 y자리가 원래 흑색이었다면 높이 위반 가능 → fixup
  if (y_origin_color == RBTREE_BLACK) {
    // rbtree_erase_fixup(t,x)
  }

  free(z); // 삭제된 노드 z의 메모리를 해제해야 메모리 누수가 발생하지 않음
  return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  // TODO: implement to_array
  return 0;
}
