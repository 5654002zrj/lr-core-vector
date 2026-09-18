/* vector.h —— 练习用动态数组（简化版 std::vector）的接口声明 */

#pragma once

#include <stddef.h>
#include <stdint.h>

/* data 指向缓冲区起点，end 指向最后一个元素的后一位，cap
 * 指向缓冲区末尾的后一位。 有效元素位于 [data, end)，剩余空间位于 [end, cap)。
 * 零容量状态下三个指针均为 NULL；此时 size() 和 capacity() 返回
 * 0，不能做空指针减法。 所有函数要求 v 非 NULL；除 vector_init 外，v
 * 必须已初始化或处于全 NULL 状态。
 * get、front、back、pop_back 的 out 必须指向可写的 int。
 */
typedef struct {
    int *data;
    int *end;
    int *cap;
} vector;

/* ===================== 生命周期 ===================== */

/* 初始化：申请一块能装 capacity 个 int 的缓冲区，size() 为 0
 * 调用前 v 不得持有尚未释放的缓冲区；字段无需预先清零，初始化不读取旧值
 * capacity 为 0 时不分配内存，三个指针置为 NULL，返回 0
 * capacity 最大为 SIZE_MAX / sizeof(int)
 * 超过上限或内存分配失败时返回 -1，三个指针置为 NULL，否则返回 0
 * 时间复杂度：O(1)
 */
int vector_init(vector *v, size_t capacity);

/* 释放 v 占用的内存并重置为空 vector
 * 释放后三个指针均为 NULL，可以重复调用
 * 时间复杂度：O(1)
 */
void vector_destroy(vector *v);

/* ===================== 容量与大小 ===================== */

/* 当前元素个数
 * 时间复杂度：O(1)
 */
size_t size(const vector *v);

/* 当前容量
 * 时间复杂度：O(1)
 */
size_t capacity(const vector *v);

/* size() 为 0 时返回 1，否则返回 0
 * 时间复杂度：O(1)
 */
int empty(const vector *v);

/* ===================== 元素访问 ===================== */

/* 读取下标 index 处的元素，写入 *out
 * index >= size() 时返回 -1 且不写入 *out，成功返回 0
 * 时间复杂度：O(1)
 */
int get(const vector *v, size_t index, int *out);

/* 把下标 index 处的元素改成 value
 * index >= size() 时返回 -1 且不修改任何元素，成功返回 0
 * 时间复杂度：O(1)
 */
int set(vector *v, size_t index, int value);

/* 读取首元素，写入 *out
 * empty(v) 时返回 -1 且不写入 *out，成功返回 0
 * 时间复杂度：O(1)
 */
int front(const vector *v, int *out);

/* 读取末元素，写入 *out
 * empty(v) 时返回 -1 且不写入 *out，成功返回 0
 * 时间复杂度：O(1)
 */
int back(const vector *v, int *out);

/* ===================== 修改器 ===================== */

/* 在尾部追加一个元素，size() 加一，容量不够时自动扩容
 * 容量足够时容量不变；容量为 0 时增至 1，否则严格增至原容量的 2 倍
 * 两倍容量超过 SIZE_MAX / sizeof(int) 或分配失败时返回 -1，不修改 vector
 * 成功返回 0
 * 时间复杂度：不扩容时 O(1)，扩容时 O(size())
 */
int push_back(vector *v, int value);

/* 删除尾部元素并写入 *out，size() 减一，容量不变
 * empty(v) 时返回 -1，不写入 *out 且不修改 vector；成功返回 0
 * 时间复杂度：O(1)
 */
int pop_back(vector *v, int *out);

/* 扩容到至少能装 capacity 个元素，size() 和已有元素保持不变；容量只增不减
 * capacity 最大为 SIZE_MAX / sizeof(int)
 * 超过上限或扩容失败时返回 -1，容量和元素都不变
 * 成功（含 capacity 不大于当前容量、无需扩容）时返回 0
 * 时间复杂度：O(size())，不需要扩容时 O(1)
 */
int reserve(vector *v, size_t capacity);

/* 收缩容量，使 capacity() == size()，元素和 size() 不变
 * size() 为 0 时释放内存并把三个指针置为 NULL，返回 0
 * 失败时返回 -1，容量和元素都不变；成功返回 0
 * 时间复杂度：O(size())
 */
int shrink_to_fit(vector *v);

/* 清空所有元素：size() 变成 0，容量和缓冲区保持不变
 * 时间复杂度：O(1)
 */
void clear(vector *v);
