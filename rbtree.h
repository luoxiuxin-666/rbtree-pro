#ifndef RBTREE_H
#define RBTREE_H

// 前向声明
typedef struct rbtree_node rbtree_node;

/**
 * 节点形状
 * 修改：key 和 value 统一为字符串，且增加 typedef struct 标签以便内部引用
 */
struct rbtree_node {
    unsigned color;
    struct rbtree_node *left;
    struct rbtree_node *right;
    struct rbtree_node *parent;

    char *key;   // 修改为 char*
    char *value; // 修改为 char*
};

/**
 * 红黑树的形状
 */
typedef struct rbtree {
    rbtree_node *nil;
    rbtree_node *root;
    int count;
} rbtree;

void Left_Rotate(rbtree *tree, rbtree_node *node);
void Right_Rotate(rbtree *tree, rbtree_node *node);
void rbtree_insert_fix(rbtree *tree, rbtree_node *node);
void rbtree_insert_(rbtree *tree, rbtree_node *node);
rbtree_node *rbtree_min_(rbtree *tree, rbtree_node *node);
rbtree_node *rbtree_get_successor(rbtree *tree, rbtree_node *node);
void rbtree_delete_fixup(rbtree *tree, rbtree_node *node);
rbtree_node *rbtree_delete_(rbtree *tree, rbtree_node *node);
rbtree_node *rbtree_search(rbtree *tree, char *key);
void rbtree_traversal(rbtree *tree, rbtree_node *node);
int kvstore_rbtree_create(rbtree *tree);
void _rbtree_free_node(rbtree *tree, rbtree_node *node);
void kvstore_rbtree_destroy(rbtree *tree);
int kvs_rbtree_set(rbtree *tree, char *key, char *value);
char* kvs_rbtree_get(rbtree *tree, char *key);
int kvs_rbtree_delete(rbtree *tree, char *key);
int kvs_rbtree_count(rbtree *tree);

int _check_properties(rbtree *tree, rbtree_node *node);
void rbtree_verify(rbtree *tree);

#endif