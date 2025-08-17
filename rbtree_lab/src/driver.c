// src/driver.c
#include "rbtree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// ─────────────────────────────────────────────────────────────
// 터미널 CLI
// # 1. sentinel 안 쓰는 경우
// make -C src clean && make -C src CFLAGS='-Wall -g -DLOCAL_TEST'
// ./src/driver
// valgrind ./src/driver   # 추후 delete 함수 작성 후에 입력하면 메모리 검사 가능

// # 2. sentinel 쓰는 경우
// make -C src clean && make -C src CFLAGS='-Wall -g -DLOCAL_TEST -DSENTINEL'
// ./src/driver
// valgrind ./src/driver

// 테스트 ON/OFF: 빌드 시 -DLOCAL_TEST를 주면 아래 테스트들이 컴파일됨
#ifdef LOCAL_TEST // LOCAL_TEST ON

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
// 여기서부터 테스트 함수 작성

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

// 중위순회로 노드 포인터 수집
static void inorder_collect_nodes_s(const rbtree *t, const node_t *p,
                                    node_t **buf, size_t *idx) {
  if (p == t->nil) return;
  inorder_collect_nodes_s(t, p->left, buf, idx);
  buf[(*idx)++] = (node_t *)p;
  inorder_collect_nodes_s(t, p->right, buf, idx);
}

// 현재 트리에서 '비-nil' 자식 수가 deg(0/1/2)인 노드 하나 선택
static node_t *pick_node_with_degree_s(const rbtree *t, int deg) {
  node_t *nodes[512] = {0};
  size_t n = 0;
  inorder_collect_nodes_s(t, t->root, nodes, &n);
  for (size_t i = 0; i < n; ++i) {
    node_t *p = nodes[i];
    int c = 0;
    if (p->left  != t->nil) ++c;
    if (p->right != t->nil) ++c;
    if (c == deg) return p;
  }
  return NULL;
}

static int contains_key_s(const key_t *arr, size_t n, key_t k) {
  for (size_t i = 0; i < n; ++i) if (arr[i] == k) return 1;
  return 0;
}

static void assert_sorted_non_decreasing_s(const key_t *arr, size_t n) {
  for (size_t i = 1; i < n; ++i) CHECK(arr[i-1] <= arr[i]);
}

// [테스트] SENTINEL 전용: BST 삭제만 검증(리프 → (가능하면)1자식 → 2자식)
static void test_bst_erase_only_sentinel(void) {
  rbtree *t = new_rbtree();

  // 케이스 다양하게 나오는 키 세트
  const int arr[] = {
    20,10,30, 5,15,25,35, 3,7,13,17,23,27,33,37
  };
  const size_t n0 = sizeof(arr)/sizeof(arr[0]);

  for (size_t i = 0; i < n0; ++i) CHECK(rbtree_insert(t, arr[i]) != NULL);
  CHECK(t->root != t->nil);

  // 1) 리프 삭제
  node_t *leaf = pick_node_with_degree_s(t, 0);
  CHECK(leaf != NULL);
  key_t del1 = leaf->key;
  (void)rbtree_erase(t, leaf);

  key_t out1[512] = {0}; size_t m1 = 0;
  inorder_collect(t, t->root, out1, &m1);
  CHECK(m1 == n0 - 1);
  assert_sorted_non_decreasing_s(out1, m1);
  CHECK(!contains_key_s(out1, m1, del1));

  // 2) 1자식 노드 삭제 (없으면 스킵)
  node_t *one = pick_node_with_degree_s(t, 1);
  if (one) {
    key_t del2 = one->key;
    (void)rbtree_erase(t, one);

    key_t out2[512] = {0}; size_t m2 = 0;
    inorder_collect(t, t->root, out2, &m2);
    CHECK(m2 == n0 - 2);
    assert_sorted_non_decreasing_s(out2, m2);
    CHECK(!contains_key_s(out2, m2, del1));
    CHECK(!contains_key_s(out2, m2, del2));
  } else {
    printf("[BST ERASE SENTINEL] SKIP: 1-child node not found in current shape\n");
  }

  // 3) 2자식 노드 삭제
  node_t *two = pick_node_with_degree_s(t, 2);
  CHECK(two != NULL);
  key_t del3 = two->key;
  (void)rbtree_erase(t, two);

  key_t out3[512] = {0}; size_t m3 = 0;
  inorder_collect(t, t->root, out3, &m3);
  // one이 없어서 스킵했다면 m3 == n0 - 2, 있었다면 n0 - 3
  CHECK(m3 == n0 - 2 || m3 == n0 - 3);
  assert_sorted_non_decreasing_s(out3, m3);
  CHECK(!contains_key_s(out3, m3, del1));
  CHECK(!contains_key_s(out3, m3, del3));

  delete_rbtree(t);
}

// [도우미] 키로 노드 찾기 (테스트용)
static node_t *rbtree_find_node(const rbtree *t, key_t key) {
  node_t *p = t->root;
#ifdef SENTINEL
  while (p != t->nil) {
#else
  while (p != NULL) {
#endif
    if (key < p->key)
      p = p->left;
    else if (key > p->key)
      p = p->right;
    else
      return p; // 찾음!
  }
  return NULL; // 못 찾음
}

// [테스트 5] BST 삭제 기본 (3가지 경우)
static void test_bst_delete_cases(void) {
  rbtree *t = new_rbtree();
  // 테스트를 위한 트리 구성
  //       10
  //      /  \'
  //     5    20
  //    / \  /  \'
  //   3  7 15  30
  const int arr[] = {10, 5, 20, 3, 7, 15, 30};
  const size_t n = sizeof(arr) / sizeof(arr[0]);
  for (size_t i = 0; i < n; ++i) {
    rbtree_insert(t, arr[i]);
  }

  // 중위 순회 결과를 확인하기 위한 버퍼 및 변수
  key_t buf[32];
  size_t m;

  // [Case 1] 자식이 없는 노드(리프 노드) 삭제 테스트
  printf("  - BST Delete Case 1: Deleting a leaf node (key: 3)\n");
  node_t *p1 = rbtree_find_node(t, 3);
  CHECK(p1 != NULL);
  rbtree_erase(t, p1);

  key_t out1[] = {5, 7, 10, 15, 20, 30};
  m = 0;
  inorder_collect(t, t->root, buf, &m);
  CHECK(m == n - 1);
  for (size_t i = 0; i < m; ++i) CHECK(buf[i] == out1[i]);


  // [Case 2] 자식이 하나인 노드 삭제 테스트
  // 위에서 3을 지웠으므로, 5는 이제 오른쪽 자식(7)만 가짐
  printf("  - BST Delete Case 2: Deleting a node with one child (key: 5)\n");
  node_t *p2 = rbtree_find_node(t, 5);
  CHECK(p2 != NULL);
  rbtree_erase(t, p2);
  
  key_t out2[] = {7, 10, 15, 20, 30};
  m = 0;
  inorder_collect(t, t->root, buf, &m);
  CHECK(m == n - 2);
  for (size_t i = 0; i < m; ++i) CHECK(buf[i] == out2[i]);


  // [Case 3] 자식이 둘인 노드 삭제 테스트
  // 루트인 10을 삭제. 후임자(successor)인 15가 그 자리를 대체해야 함
  printf("  - BST Delete Case 3: Deleting a node with two children (key: 10)\n");
  node_t *p3 = rbtree_find_node(t, 10);
  CHECK(p3 != NULL);
  rbtree_erase(t, p3);

  key_t out3[] = {7, 15, 20, 30};
  m = 0;
  inorder_collect(t, t->root, buf, &m);
  CHECK(m == n - 3);
  for (size_t i = 0; i < m; ++i) CHECK(buf[i] == out3[i]);
  
  // 루트가 후임자인 15로 잘 바뀌었는지 확인
  CHECK(t->root->key == 15);

  delete_rbtree(t);
}

#endif // LOCAL_TEST OFF

int main(void) {
#ifdef LOCAL_TEST
  // 위에서 작성한 테스트 함수들 여기서 실행
  TEST(test_new_rbtree_basic);
  TEST(test_insert_one);
  TEST(test_inorder_sorted);
  TEST(test_root_black);
  TEST(test_rb_left_chain_rotates);
  TEST(test_bst_erase_only_sentinel);
  TEST(test_bst_delete_cases);
  printf("Summary: %d/%d tests passed.\n", tests_passed, tests_run);
#else
  // 공식 테스트에선 driver가 링크되지 않지만,
  // 혹시를 대비해 빈 main 유지
  (void)0;
#endif
  return 0;
}
