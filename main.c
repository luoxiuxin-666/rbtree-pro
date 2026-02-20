#include "rbtree.h"
#include<stdio.h>
#include <stdlib.h> 
int main(int argc, char const *argv[]) {
    // 1. 正确分配 tree 结构体的内存
    rbtree *tree = (rbtree*)malloc(sizeof(rbtree));
    kvstore_rbtree_create(tree);

    // 2. 将测试数据改为字符串
    char *keys[] = {"60", "15", "9", "85", "95", "99", "7", "6", "8", "90", "80", "45", "66", "70", "75", "55", "50"};
    char *values[] = {"val60", "val15", "val9", "val85", "val95", "val99", "val7", "val6", "val8", "val90", "val80", "val45", "val66", "val70", "val75", "val55", "val50"};

    // 3. 插入数据
    for (int i = 0; i < 17; i++) {
        kvs_rbtree_set(tree, keys[i], values[i]);
    }

    printf("--- Initial Traversal ---\n");
    rbtree_traversal(tree, tree->root);

    // 4. 修改测试
    printf("\n--- Modify Test ---\n");
    kvs_rbtree_set(tree, "99", "new_val_9999");
    printf("Get '99': %s\n", kvs_rbtree_get(tree, "99"));
    // 检查红黑树性质
    rbtree_verify(tree);
    // 5. 删除测试
    printf("\n--- Delete Test ---\n");
    for (int i = 0; i < 17; i++) {
        printf("Deleting %s...\n", keys[i]);
        kvs_rbtree_delete(tree, keys[i]);
    }
    
    printf("Tree count: %d\n", kvs_rbtree_count(tree));

    // 6. 销毁
    kvstore_rbtree_destroy(tree);
    free(tree); // 别忘了释放 tree 容器本身

    

    return 0;
}