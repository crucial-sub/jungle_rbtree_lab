// src/driver.c
#include "rbtree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// ─────────────────────────────────────────────────────────────
// 테스트 ON/OFF: 빌드 시 -DLOCAL_TEST를 주면 아래 테스트들이 컴파일됨
#ifdef LOCAL_TEST

// 간단한 마이크로 테스트 프레임워크
static int tests_run = 0, tests_passed = 0;

#define TEST(fn) do {                               \
  printf("RUN  %s\n", #fn);                         \
  ++tests_run;                                      \
  fn();                                             \
  ++tests_passed;                                   \
  printf("PASS %s\n\n", #fn);                       \
} while (0)

// (옵션) 가독성 좋은 체크 매크로 (assert 대용)
#define CHECK(cond) do {                            \
  if (!(cond)) {                                    \
    fprintf(stderr,                                 \
      "CHECK failed: %s (at %s:%d)\n",              \
      #cond, __FILE__, __LINE__);                   \
    exit(1);                                        \
  }                                                 \
} while (0)

// ─────────────────────────────────────────────────────────────
// [테스트 1] new_rbtree 기본
static void test_new_rbtree_basic(void) {
  rbtree *t = new_rbtree();
  CHECK(t != NULL);
#ifdef SENTINEL
  CHECK(t->nil != NULL);
  CHECK(t->root == t->nil);
#else
  CHECK(t->root == NULL);
#endif
  delete_rbtree(t);
}

// [테스트 2] 한 개 삽입
static void test_insert_one(void) {
  rbtree *t = new_rbtree();
  node_t *p = rbtree_insert(t, 7);
  CHECK(p != NULL);
  CHECK(t->root == p);
  CHECK(p->key == 7);
  // 루트는 보통 BLACK이어야 함(원하면 켜기)
  // CHECK(p->color == RBTREE_BLACK);
#ifdef SENTINEL
  CHECK(p->left == t->nil && p->right == t->nil);
  CHECK(p->parent == t->nil);
#else
  CHECK(p->left == NULL && p->right == NULL);
  CHECK(p->parent == NULL);
#endif
  delete_rbtree(t);
}

// [도우미] 중위순회로 수집
static void inorder_collect(const rbtree *t, const node_t *p,
                            key_t *buf, size_t *idx) {
#ifdef SENTINEL
  if (p == t->nil) return;
#else
  if (p == NULL) return;
#endif
  inorder_collect(t, p->left, buf, idx);
  buf[(*idx)++] = p->key;
  inorder_collect(t, p->right, buf, idx);
}

// [테스트 3] 여러 개 삽입 → 오름차순 검증
static void test_inorder_sorted(void) {
  rbtree *t = new_rbtree();
  const int arr[] = {7, 5, 10, 3, 6, 8, 12};
  const size_t n = sizeof(arr)/sizeof(arr[0]);
  for (size_t i = 0; i < n; ++i) {
    CHECK(rbtree_insert(t, arr[i]) != NULL);
  }
#ifdef SENTINEL
  CHECK(t->root != t->nil);
#else
  CHECK(t->root != NULL);
#endif

  key_t out[32] = {0};
  size_t m = 0;
  inorder_collect(t, t->root, out, &m);
  CHECK(m == n);
  for (size_t i = 1; i < m; ++i) {
    CHECK(out[i-1] <= out[i]);  // 오름차순
  }
  delete_rbtree(t);
}

// [테스트 4] 간단 RB 성질(루트 블랙)만 확인
static void test_root_black(void) {
  rbtree *t = new_rbtree();
  int arr[] = {10, 5, 20, 1, 7, 15, 30};
  for (int i = 0; i < 7; ++i) rbtree_insert(t, arr[i]);
#ifdef SENTINEL
  CHECK(t->root == t->nil || t->root->color == RBTREE_BLACK);
#else
  // sentinel을 안 쓰면 빈 트리(NULL) 또는 root->color 검사를 스스로 정의해야 함
  if (t->root) CHECK(t->root->color == RBTREE_BLACK);
#endif
  delete_rbtree(t);
}

// [TC3-RB] 7,5,3 삽입 → LL 회전 결과 점검 (RB 버전)
static void test_rb_left_chain_rotates(void) {
  rbtree *t = new_rbtree();
  rbtree_insert(t, 7);
  rbtree_insert(t, 5);
  rbtree_insert(t, 3);

#ifdef SENTINEL
  node_t *nil = t->nil;
  // 1) 루트는 5여야 함 (LL → g=7 우회전)
  CHECK(t->root != nil);
  CHECK(t->root->key == 5);

  // 2) 루트의 양쪽 자식은 3,7
  CHECK(t->root->left  != nil && t->root->left->key  == 3);
  CHECK(t->root->right != nil && t->root->right->key == 7);

  // 3) 루트는 BLACK
  CHECK(t->root->color == RBTREE_BLACK);
#else
  CHECK(t->root != NULL);
  CHECK(t->root->key == 5);
  CHECK(t->root->left  && t->root->left->key  == 3);
  CHECK(t->root->right && t->root->right->key == 7);
  CHECK(t->root->color == RBTREE_BLACK);
#endif

  // 4) (선택) 중위순회 정렬 확인
  key_t out[3] = {0}; size_t m = 0;
  inorder_collect(t, t->root, out, &m);
  CHECK(m == 3);
  CHECK(out[0] == 3 && out[1] == 5 && out[2] == 7);

  delete_rbtree(t);
}

#endif // LOCAL_TEST

int main(void) {
#ifdef LOCAL_TEST
  TEST(test_new_rbtree_basic);
  TEST(test_insert_one);
  TEST(test_inorder_sorted);
  TEST(test_root_black);
  TEST(test_rb_left_chain_rotates);
  printf("Summary: %d/%d tests passed.\n", tests_passed, tests_run);
#else
  // 공식 테스트에선 driver가 링크되지 않지만,
  // 혹시를 대비해 빈 main 유지
  (void)0;
#endif
  return 0;
}
