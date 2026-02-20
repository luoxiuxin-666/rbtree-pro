#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rbtree.h"

// 模拟 kvstore.h 的宏定义，方便直接编译
#define kvstore_malloc malloc
#define kvstore_free free

enum {
    BLACK = 0,
    RED = 1
};


/**
 * 左旋
 */
void Left_Rotate(rbtree *tree, rbtree_node *node) {
    rbtree_node *right = node->right;
    node->right = right->left;
    
    if (right->left != tree->nil) {
        right->left->parent = node;
    }
    
    right->parent = node->parent;
    
    if (right->parent == tree->nil) {
        tree->root = right;
    } else if (node == node->parent->left) {
        node->parent->left = right;
    } else {
        node->parent->right = right;
    }
    
    right->left = node;
    node->parent = right;
}

/**
 * 右旋
 */
void Right_Rotate(rbtree *tree, rbtree_node *node) {
    rbtree_node *left = node->left;
    node->left = left->right;
    
    if (left->right != tree->nil) {
        left->right->parent = node;
    }

    left->parent = node->parent;
    
    if (left->parent == tree->nil) {
        tree->root = left; // [修正] 原代码是 tree->root->left; 这是一个无效语句
    } else if (node == node->parent->left) {
        node->parent->left = left;
    } else {
        node->parent->right = left;
    }

    left->right = node;
    node->parent = left;
}

/**
 * 插入修正
 * [重要修正] 这里必须是 while 循环，因为修正可能向上传递
 */
void rbtree_insert_fix(rbtree *tree, rbtree_node *node) {
    while (node->parent->color == RED) {
        rbtree_node *parent = node->parent;
        rbtree_node *grandfather = parent->parent;
        
        if (parent == grandfather->left) {
            rbtree_node *uncle = grandfather->right;
            // Case 1: 叔叔是红色
            if (uncle->color == RED) {
                parent->color = BLACK;
                uncle->color = BLACK;
                grandfather->color = RED;
                node = grandfather; // 接着检查祖父
            } else {
                // Case 2: 叔叔是黑色，且当前节点是右孩子 (LR型) -> 转为 LL型
                if (node == parent->right) { // [修正] 原代码是 node = parent->right (赋值)
                    node = parent;
                    Left_Rotate(tree, node);
                    parent = node->parent; // 旋转后 parent 变了，需要更新指针以便下一步操作
                }
                // Case 3: 叔叔是黑色，且当前节点是左孩子 (LL型)
                parent->color = BLACK;
                grandfather->color = RED;
                Right_Rotate(tree, grandfather);
            }
        } else {
            // 父节点是右子树
            rbtree_node *uncle = grandfather->left;
            // Case 1
            if (uncle->color == RED) {
                parent->color = BLACK;
                uncle->color = BLACK;
                grandfather->color = RED;
                node = grandfather;
            } else {
                // Case 2: RL型 -> 转为 RR型
                if (node == parent->left) {
                    node = parent;
                    Right_Rotate(tree, node);
                    parent = node->parent;
                }
                // Case 3: RR型
                parent->color = BLACK;
                grandfather->color = RED;
                Left_Rotate(tree, grandfather);
            }
        }
    }
    tree->root->color = BLACK;
}

/**
 * 插入操作
 */
void rbtree_insert_(rbtree *tree, rbtree_node *node) {
    rbtree_node *cmp = tree->root;
    rbtree_node *parent = tree->nil;

    node->left = tree->nil;
    node->right = tree->nil;
    
    while (cmp != tree->nil) {
        parent = cmp;
        // [修正] 使用 strcmp 比较字符串 key
        int res = strcmp(node->key, cmp->key);
        if (res > 0) {
            cmp = cmp->right;
        } else if (res < 0) {
            cmp = cmp->left;
        } else {
            // Key 存在，更新 Value
            // [修正] 需要释放旧 value 内存，重新分配
            if (cmp->value) kvstore_free(cmp->value);
            cmp->value = (char*)kvstore_malloc(strlen(node->value) + 1);
            strcpy(cmp->value, node->value);
            
            // 释放传入的 node，因为我们只用了它的数据更新现有节点
            kvstore_free(node->key);
            kvstore_free(node->value);
            kvstore_free(node);
            return;
        }
    }
    
    node->parent = parent;
    if (parent == tree->nil) {
        tree->root = node;
        node->color = BLACK; // 根总是黑的
    } else {
        if (strcmp(node->key, parent->key) < 0) {
            parent->left = node;
        } else {
            parent->right = node;
        }
        node->color = RED; // 新节点默认红
        rbtree_insert_fix(tree, node); // [修正] 这是一个函数调用，不需要 void 声明
    }
    
    
}

rbtree_node *rbtree_min_(rbtree *tree, rbtree_node *node) {
    while (node->left != tree->nil) {
        node = node->left;
    }
    return node;
}

rbtree_node *rbtree_get_successor(rbtree *tree, rbtree_node *node) {
    if (node->right != tree->nil) {
        return rbtree_min_(tree, node->right);
    }
    rbtree_node *parent = node->parent;
    while (parent != tree->nil && node == parent->right) {
        node = parent;
        parent = parent->parent; // [修正] 应该是 parent->parent
    }
    return parent;
}

/**
 * 删除修正
 */
void rbtree_delete_fixup(rbtree *tree, rbtree_node *node) {
    while ((node != tree->root) && (node->color == BLACK)) {
        if (node == node->parent->left) {
            rbtree_node *bro = node->parent->right; 
            
            // Case 1: 兄弟是红色
            if (bro->color == RED) {
                bro->color = BLACK;
                node->parent->color = RED;
                Left_Rotate(tree, node->parent);
                bro = node->parent->right; 
            }
            
            // Case 2: 兄弟的两个孩子都是黑色
            if ((bro->left->color == BLACK) && (bro->right->color == BLACK)) {
                bro->color = RED;
                node = node->parent;
            } else {
                // Case 3: 兄弟右孩子是黑色 (即左红右黑，RL型)
                if (bro->right->color == BLACK) {
                    bro->left->color = BLACK;
                    bro->color = RED;
                    Right_Rotate(tree, bro);
                    bro = node->parent->right; 
                }
                // Case 4: 兄弟右孩子是红色 (RR型)
                bro->color = node->parent->color;
                node->parent->color = BLACK; // [修正] 原代码: node->parent = BLACK (类型错误)
                bro->right->color = BLACK;
                Left_Rotate(tree, node->parent);
                node = tree->root;
            }
        } else {
            // 对称情况
            rbtree_node *bro = node->parent->left;
            if (bro->color == RED) {
                bro->color = BLACK;
                node->parent->color = RED;
                Right_Rotate(tree, node->parent);
                bro = node->parent->left;
            }
            if ((bro->left->color == BLACK) && (bro->right->color == BLACK)) {
                bro->color = RED;
                node = node->parent;
            } else {
                if (bro->left->color == BLACK) {
                    bro->right->color = BLACK;
                    bro->color = RED;
                    Left_Rotate(tree, bro); // [修正] 修复语法错误 : -> ;
                    bro = node->parent->left;
                }
                bro->color = node->parent->color;
                node->parent->color = BLACK;
                bro->left->color = BLACK;
                Right_Rotate(tree, node->parent);
                node = tree->root;
            }
        }
    }
    node->color = BLACK;
}

/**
 * 删除核心逻辑
 * 注意：为了简化内存管理，如果进行了交换，我们在内部处理数据的移动。
 * 返回需要被释放（物理删除）的那个节点结构体。
 */
rbtree_node *rbtree_delete_(rbtree *tree, rbtree_node *node) {
    rbtree_node *delete_node = node; // 默认删除自己
    rbtree_node *replace_node = tree->nil;

    // 1. 确定要物理删除的节点 (delete_node)
    // 如果有两个孩子，实际上删除的是后继节点
    if (node->left != tree->nil && node->right != tree->nil) {
        delete_node = rbtree_get_successor(tree, node);
    }

    // 2. 确定顶替位置的节点 (replace_node)
    if (delete_node->left != tree->nil) {
        replace_node = delete_node->left;
    } else {
        replace_node = delete_node->right;
    }

    // 3. 链接 replace_node 和 delete_node 的父节点
    replace_node->parent = delete_node->parent;

    if (delete_node->parent == tree->nil) {
        tree->root = replace_node;
    } else if (delete_node == delete_node->parent->left) {
        delete_node->parent->left = replace_node;
    } else {
        delete_node->parent->right = replace_node;
    }

    // 4. 如果物理删除的节点不是目标节点，说明进行了交换。
    // 我们需要把 delete_node 的 Key/Value 移动到 node 中。
    // 同时，我们需要先释放 node 原本指向的 Key/Value 内存，防止泄漏。
    if (delete_node != node) {
        kvstore_free(node->key);
        kvstore_free(node->value);
        
        // 移动指针所有权
        node->key = delete_node->key;
        node->value = delete_node->value;
        
        // 将 delete_node 的指针置空，防止外部再次释放
        delete_node->key = NULL;
        delete_node->value = NULL;
    }

    // 5. 修正颜色
    if (delete_node->color == BLACK) {
        rbtree_delete_fixup(tree, replace_node);
    }

    return delete_node; // 返回该节点，以便外部释放 rbtree_node 结构体本身的内存
}

rbtree_node *rbtree_search(rbtree *tree, char *key) {
    rbtree_node *node = tree->root;
    while (node != tree->nil) {
        int res = strcmp(key, node->key);
        if (res > 0) {
            node = node->right;
        } else if (res < 0) {
            node = node->left;
        } else {
            return node;
        }
    }
    return tree->nil;
}

void rbtree_traversal(rbtree *tree, rbtree_node *node) {
    if (node != tree->nil) {
        rbtree_traversal(tree, node->left);
        printf("key:%s, val:%s, color:%s\n", node->key, node->value, node->color == RED ? "RED" : "BLACK");
        rbtree_traversal(tree, node->right);
    }
}

int kvstore_rbtree_create(rbtree *tree) {
    if (!tree) return -1;
    
    // 初始化哨兵
    tree->nil = (rbtree_node*)malloc(sizeof(rbtree_node));
    if (!tree->nil) return -1;

    tree->nil->key = NULL;
    tree->nil->value = NULL;
    tree->nil->color = BLACK;
    tree->nil->left = tree->nil; // 哨兵回环，防止野指针
    tree->nil->right = tree->nil;
    tree->nil->parent = NULL;

    tree->root = tree->nil;
    tree->count = 0;

    return 0;
}

void _rbtree_free_node(rbtree *tree, rbtree_node *node) {
    if (node == tree->nil) return;
    _rbtree_free_node(tree, node->left);
    _rbtree_free_node(tree, node->right);

    if (node->key) kvstore_free(node->key);
    if (node->value) kvstore_free(node->value);
    kvstore_free(node);
}

void kvstore_rbtree_destroy(rbtree *tree) {
    if (!tree) return;
    _rbtree_free_node(tree, tree->root);
    if (tree->nil) kvstore_free(tree->nil);
}

int kvs_rbtree_set(rbtree *tree, char *key, char *value) {
    rbtree_node *node = (rbtree_node*)kvstore_malloc(sizeof(rbtree_node));
    if (!node) return -1;

    node->key = (char*)kvstore_malloc(strlen(key) + 1);
    if (node->key == NULL) {
        kvstore_free(node);
        return -1;
    }
    strcpy(node->key, key);

    node->value = (char*)kvstore_malloc(strlen(value) + 1);
    if (node->value == NULL) {
        kvstore_free(node->key);
        kvstore_free(node);
        return -1;
    }
    strcpy(node->value, value);

    // 调用内部插入逻辑
    rbtree_insert_(tree, node);
    tree->count++;

    return 0;
}

char* kvs_rbtree_get(rbtree *tree, char *key) {
    rbtree_node *node = rbtree_search(tree, key);
    if (node == tree->nil) {
        return NULL;
    }
    return node->value;
}

int kvs_rbtree_delete(rbtree *tree, char *key) {
    rbtree_node *node = rbtree_search(tree, key);
    if (node == tree->nil) return -1;

    // rbtree_delete_ 会处理 key/value 的移动和释放逻辑，返回需要释放的节点壳子
    rbtree_node *cur = rbtree_delete_(tree, node);
    if (cur) {
        // 如果 cur->key 还有值（说明是直接删除，没交换），则释放
        // 如果发生了交换，rbtree_delete_ 内部已经把 cur->key 置 NULL 了
        if (cur->key) kvstore_free(cur->key);
        if (cur->value) kvstore_free(cur->value);
        kvstore_free(cur);
    }

    tree->count--;
    return 0;
}

int kvs_rbtree_count(rbtree *tree) {
    return tree->count;
}


#include <assert.h>

// 辅助函数：计算黑高度并检查性质
int _check_properties(rbtree *tree, rbtree_node *node) {
    if (node == tree->nil) {
        return 1; // 叶子节点（NIL）是黑色，黑高度贡献为 1
    }

    // 性质 4: 红色节点的孩子必须是黑色 (不能出现连续的红色)
    if (node->color == RED) {
        if (node->left->color != BLACK || node->right->color != BLACK) {
            printf("错误：发现连续的红色节点！节点 key: %s\n", node->key);
            assert(0);
        }
    }

    // 性质 5: 递归检查左右子树的黑高度
    int left_black_height = _check_properties(tree, node->left);
    int right_black_height = _check_properties(tree, node->right);

    // 性质 5 核心：从任一节点到其每个叶子的所有简单路径都包含相同数目的黑色节点
    if (left_black_height != right_black_height) {
        printf("错误：路径黑高度不一致！节点 key: %s, 左黑高: %d, 右黑高: %d\n", 
                node->key, left_black_height, right_black_height);
        assert(0);
    }

    // 返回当前节点的黑高度：如果是黑色，高度+1；红色则不加
    return (node->color == BLACK ? left_black_height + 1 : left_black_height);
}

void rbtree_verify(rbtree *tree) {
    if (tree->root == tree->nil) return;

    // 性质 2: 根节点必须是黑色
    if (tree->root->color != BLACK) {
        printf("错误：根节点不是黑色！\n");
        assert(0);
    }

    _check_properties(tree, tree->root);
    printf("恭喜：红黑树性质校验通过！\n");
}