/* tests/test.c —— lr-core-vector 的单元测试 */

#define _POSIX_C_SOURCE 200809L

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vector.h"

/* ===================== 极简测试框架 ===================== */

static int g_pass = 0;
static int g_fail = 0;

/* ===================== 着色 ===================== */

/* 输出接在终端上才上色。`make test | tee log.txt` 或者重定向到文件时
 * isatty 是假，颜色自动退化成空串，日志里不会混进 ANSI 转义码。
 * 结果缓存一下，免得每条断言都去问一次系统。 */
static int use_color(void) {
    static int cached = -1;
    if (cached < 0) {
        cached = isatty(STDOUT_FILENO) ? 1 : 0;
    }
    return cached;
}

/* 颜色码得当成参数传给 printf（%s），不能直接和字符串字面量拼接：
 * 这几个宏展开出来是三元表达式，不是字面量。 */
static const char *red(void) {
    return use_color() ? "\033[31m" : "";
}
static const char *green(void) {
    return use_color() ? "\033[32m" : "";
}
static const char *reset(void) {
    return use_color() ? "\033[0m" : "";
}

/* 只给 [FAIL] / [ OK ] 这几个标签上色，正文保持默认前景色：
 * 失败几十条的时候整屏通红，比黑底正文难读得多。 */
static void check_bool(int ok, const char *what, int line) {
    if (ok) {
        g_pass++;
        return;
    }
    g_fail++;
    printf("  %s[FAIL]%s %s  (%s:%d)\n", red(), reset(), what, __FILE__, line);
}

static void check_int(long long actual, long long expected, const char *what,
                      int line) {
    if (actual == expected) {
        g_pass++;
        return;
    }
    g_fail++;
    printf("  %s[FAIL]%s %s  期望 %lld，实际 %lld  (%s:%d)\n", red(), reset(),
           what, expected, actual, __FILE__, line);
}

#define CHECK_BOOL(cond, what)                                                 \
    do {                                                                       \
        check_bool((cond) ? 1 : 0, (what), __LINE__);                          \
    } while (0)

#define CHECK_EQ(actual, expected, what)                                       \
    do {                                                                       \
        check_int((long long)(actual), (long long)(expected), (what),          \
                  __LINE__);                                                   \
    } while (0)

/* 断言一次调用成功（返回 0） */
#define CHECK_OK(expr, what)                                                   \
    do {                                                                       \
        int result = (expr);                                                   \
        CHECK_EQ(result, 0, (what));                                           \
        if (result != 0) {                                                     \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    } while (0)

/* ===================== 带 out 参数的读取与弹出 ===================== */

/* vector.h 的约定：读取或弹出元素的函数返回 0 表示成功并写入 *out，
 * 返回 -1 表示失败且不写 *out。
 *
 * 测试里统一先把 out 填成哨兵值，这样「失败却写了 *out」也能被抓出来。
 * 下面四个包装只是让断言读起来还像原来那样，例如 GET(&v, 0) == 1。 */
#define NOT_WRITTEN INT_MIN

static int get_ok(const vector *v, size_t index, int line) {
    int out = NOT_WRITTEN;
    if (get(v, index, &out) != 0) {
        check_bool(0, "get 应该成功", line);
        return NOT_WRITTEN;
    }
    return out;
}

static int front_ok(const vector *v, int line) {
    int out = NOT_WRITTEN;
    if (front(v, &out) != 0) {
        check_bool(0, "front 应该成功", line);
        return NOT_WRITTEN;
    }
    return out;
}

static int back_ok(const vector *v, int line) {
    int out = NOT_WRITTEN;
    if (back(v, &out) != 0) {
        check_bool(0, "back 应该成功", line);
        return NOT_WRITTEN;
    }
    return out;
}

static int pop_ok(vector *v, int line) {
    int out = NOT_WRITTEN;
    if (pop_back(v, &out) != 0) {
        check_bool(0, "pop_back 应该成功", line);
        return NOT_WRITTEN;
    }
    return out;
}

#define GET(v, index) get_ok((v), (index), __LINE__)
#define FRONT(v) front_ok((v), __LINE__)
#define BACK(v) back_ok((v), __LINE__)
#define POP(v) pop_ok((v), __LINE__)

/* 初始化失败或状态无效时立即停止，避免后续测试使用无效指针。 */
static vector make_vector(size_t capacity) {
    int sentinel = 123;
    vector v = {&sentinel, &sentinel, &sentinel};
    CHECK_OK(vector_init(&v, capacity), "vector_init 应该成功");
    int valid = capacity == 0
                    ? v.data == NULL && v.end == NULL && v.cap == NULL
                  : v.data != NULL && v.data != &sentinel &&
                      v.end == v.data && v.cap != NULL;
    CHECK_BOOL(valid, "vector_init 后三个指针必须有效，否则停止测试");
    if (!valid) {
        exit(EXIT_FAILURE);
    }
    return v;
}

/* ===================== 测试用例 ===================== */

static void test_init_destroy(void) {
    vector v = make_vector(4);
    CHECK_EQ(size(&v), 0, "init 之后 size 应该是 0");
    CHECK_EQ(capacity(&v), 4, "init 之后 capacity 应该等于传入的容量");
    CHECK_EQ(empty(&v), 1, "init 之后 empty 应该是 1");

    /* 容量为 0 时不应该分配内存 */
    vector e = make_vector(0);
    CHECK_EQ(size(&e), 0, "capacity 为 0 时 size 应该是 0");
    CHECK_EQ(capacity(&e), 0, "capacity 为 0 时 capacity 应该是 0");
    CHECK_BOOL(e.data == NULL && e.end == NULL && e.cap == NULL,
               "capacity 为 0 时三个指针应该都是 NULL");
    vector_destroy(&e);

    vector_destroy(&v);
    CHECK_BOOL(v.data == NULL && v.end == NULL && v.cap == NULL,
               "destroy 之后三个指针都应该是 NULL");
    vector_destroy(&v); /* 再 destroy 一次也应该安全 */
    CHECK_BOOL(v.data == NULL && v.end == NULL && v.cap == NULL,
               "重复 destroy 后三个指针仍应该都是 NULL");
    CHECK_EQ(size(&v), 0, "destroy 之后 size 应该是 0");
    CHECK_EQ(capacity(&v), 0, "destroy 之后 capacity 应该是 0");
    CHECK_EQ(empty(&v), 1, "destroy 之后 empty 应该是 1");

    clear(&v);
    CHECK_OK(reserve(&v, 0), "零容量 reserve(0) 应该成功");
    CHECK_OK(shrink_to_fit(&v), "零容量 shrink_to_fit 应该成功");
    CHECK_BOOL(v.data == NULL && v.end == NULL && v.cap == NULL,
               "零容量 clear、reserve(0)、shrink 后仍应该全 NULL");
    CHECK_OK(reserve(&v, 3), "零容量 reserve(3) 应该成功");
    CHECK_EQ(size(&v), 0, "零容量 reserve 后 size 仍应该是 0");
    CHECK_BOOL(capacity(&v) >= 3, "零容量 reserve 后应该获得所需容量");
    CHECK_OK(push_back(&v, 42), "reserve 后应该可以追加元素");
    CHECK_EQ(GET(&v, 0), 42, "reserve 后追加的元素应该可以读取");
    vector_destroy(&v);
}

static void test_push_back(void) {
    vector v = make_vector(2);

    /* 前两次不用扩容，第三次开始扩容 */
    for (int i = 0; i < 5; i++) {
        CHECK_OK(push_back(&v, i * 10), "容量足够时 push_back 应该成功");
    }

    CHECK_EQ(size(&v), 5, "push 5 次之后 size 应该是 5");
    CHECK_EQ(empty(&v), 0, "push 之后 empty 应该是 0");
    CHECK_EQ(capacity(&v), 8, "从容量 2 连续 push 5 次之后容量应该是 8");
    int valid = v.data != NULL && v.end != NULL && v.cap != NULL;
    CHECK_BOOL(valid, "push 成功后三个指针必须非 NULL");
    if (!valid) {
        exit(EXIT_FAILURE);
    }
    CHECK_EQ(v.end - v.data, (ptrdiff_t)size(&v),
             "三个指针要自洽：end - data 应该等于 size()");

    int order_ok = 1;
    for (size_t i = 0; i < 5; i++) {
        int value = NOT_WRITTEN;
        if (get(&v, i, &value) != 0 || value != (int)(i * 10)) {
            order_ok = 0;
        }
    }
    CHECK_BOOL(order_ok, "push 进去的元素应该原封不动、按顺序待在原位");

    CHECK_EQ(GET(&v, 0), 0, "第 0 个元素应该是第一次 push 的值");
    CHECK_EQ(GET(&v, 4), 40, "第 4 个元素应该是最后一次 push 的值");

    vector_destroy(&v);
}

static void test_auto_grow(void) {
    const size_t initial_capacities[] = {0, 1, 3};
    for (size_t scenario = 0; scenario < 3; scenario++) {
        vector v = make_vector(initial_capacities[scenario]);
        size_t expected_capacity = initial_capacities[scenario];
        int growth_ok = 1;
        for (size_t index = 0; index < 1000; index++) {
            if (index == expected_capacity) {
                expected_capacity =
                    expected_capacity ? expected_capacity * 2 : 1;
            }
            CHECK_OK(push_back(&v, (int)(index + 1)), "push_back 应该成功");
            if (capacity(&v) != expected_capacity || size(&v) != index + 1) {
                growth_ok = 0;
            }
        }
        CHECK_BOOL(growth_ok, "容量为 0 时增至 1，满时严格翻倍，未满时不扩容");
        CHECK_EQ(capacity(&v), expected_capacity,
                 "最终容量应该符合两倍增长序列");
        CHECK_EQ(size(&v), 1000, "连续 push 1000 次之后 size 应该是 1000");

        long long sum = 0;
        int order_ok = 1;
        for (size_t index = 0; index < 1000; index++) {
            int value = NOT_WRITTEN;
            if (get(&v, index, &value) != 0 || value != (int)(index + 1)) {
                order_ok = 0;
            }
            sum += value;
        }
        CHECK_BOOL(order_ok, "多次扩容搬移之后，元素顺序应该仍然是 1..1000");
        CHECK_EQ(sum, 500500, "1 到 1000 的和应该是 500500");
        vector_destroy(&v);
    }
}

static void test_access(void) {
    vector v = make_vector(3);
    CHECK_OK(push_back(&v, 1), "push_back 应该成功");
    CHECK_OK(push_back(&v, 2), "push_back 应该成功");
    CHECK_OK(push_back(&v, 3), "push_back 应该成功");

    CHECK_EQ(GET(&v, 0), 1, "get(0) 应该是首元素");
    CHECK_EQ(GET(&v, 2), 3, "get(2) 应该是末元素");
    CHECK_EQ(FRONT(&v), 1, "front 应该是 1");
    CHECK_EQ(BACK(&v), 3, "back 应该是 3");

    CHECK_OK(set(&v, 1, 42), "下标合法时 set 应该成功");
    CHECK_EQ(GET(&v, 1), 42, "set 之后 get 应该返回新值");
    CHECK_EQ(FRONT(&v), 1, "set 不应该影响前面的元素");
    CHECK_EQ(BACK(&v), 3, "set 不应该影响后面的元素");
    CHECK_EQ(size(&v), 3, "set 不应该改变 size");

    vector_destroy(&v);
}

static void test_pop_back(void) {
    vector v = make_vector(8);
    for (int i = 1; i <= 3; i++) {
        CHECK_OK(push_back(&v, i), "容量足够时 push_back 应该成功");
    }

    CHECK_EQ(POP(&v), 3, "pop_back 应该通过 out 返回原来的末元素");
    CHECK_EQ(size(&v), 2, "pop_back 之后 size 应该减一");
    CHECK_EQ(BACK(&v), 2, "pop_back 之后 back 应该是新的末元素");
    CHECK_EQ(capacity(&v), 8, "pop_back 不应该改变 capacity");

    CHECK_EQ(POP(&v), 2, "第二次 pop_back 应该通过 out 返回 2");
    CHECK_EQ(POP(&v), 1, "第三次 pop_back 应该通过 out 返回 1");
    CHECK_EQ(size(&v), 0, "全部 pop 完之后 size 应该是 0");
    CHECK_EQ(empty(&v), 1, "全部 pop 完之后 empty 应该是 1");

    int out = 234;
    vector before = v;
    CHECK_EQ(pop_back(&v, &out), -1, "全部 pop 完后再次 pop 应该返回 -1");
    CHECK_EQ(out, 234, "pop_back 失败时不应该写入 *out");
    CHECK_BOOL(v.data == before.data && v.end == before.end &&
                   v.cap == before.cap,
               "pop_back 失败时三个指针应该保持不变");
    CHECK_EQ(size(&v), 0, "pop_back 失败时 size 应该保持不变");
    CHECK_EQ(capacity(&v), 8, "pop_back 失败时 capacity 应该保持不变");

    /* 元素被 pop 掉之后容量还在，可以继续 push */
    CHECK_OK(push_back(&v, 99), "还有剩余容量时 push_back 应该成功");
    CHECK_EQ(GET(&v, 0), 99, "pop 完再 push 应该能正常工作");
    CHECK_EQ(capacity(&v), 8, "还有剩余容量时 push_back 不应该扩容");

    vector_destroy(&v);
}

static void test_reserve(void) {
    vector v = make_vector(2);
    CHECK_OK(push_back(&v, 7), "push_back 应该成功");

    CHECK_OK(reserve(&v, 100), "reserve 应该成功");
    CHECK_BOOL(capacity(&v) >= 100,
               "reserve(100) 之后 capacity 应该至少是 100");
    CHECK_EQ(size(&v), 1, "reserve 不应该改变 size");
    CHECK_EQ(GET(&v, 0), 7, "reserve 换内存之后元素应该保持不变");

    CHECK_OK(reserve(&v, 1), "reserve 一个更小的值也应该返回成功");
    CHECK_BOOL(capacity(&v) >= 100, "reserve 一个更小的值不应该把容量缩小");
    CHECK_EQ(size(&v), 1, "reserve 一个更小的值不应该改变 size");
    CHECK_EQ(GET(&v, 0), 7, "reserve 一个更小的值不应该破坏元素");

    CHECK_OK(reserve(&v, 100), "容量已经够了，reserve 同样的大小也应该成功");
    CHECK_BOOL(capacity(&v) >= 100,
               "容量已经够了，reserve 同样的大小也应该有效");

    vector_destroy(&v);
}

static void test_shrink_to_fit(void) {
    vector v = make_vector(64);
    for (int i = 0; i < 5; i++) {
        CHECK_OK(push_back(&v, i), "push_back 应该成功");
    }

    CHECK_OK(shrink_to_fit(&v), "shrink_to_fit 应该成功");
    CHECK_EQ(capacity(&v), 5, "shrink_to_fit 之后 capacity 应该刚好等于 size");
    CHECK_EQ(size(&v), 5, "shrink_to_fit 不应该改变 size");
    for (int i = 0; i < 5; i++) {
        CHECK_EQ(GET(&v, i), i, "shrink_to_fit 搬移之后元素应该保持不变");
    }

    CHECK_OK(shrink_to_fit(&v), "容量等于大小时 shrink_to_fit 应该成功");
    CHECK_EQ(capacity(&v), 5, "重复收缩不应该改变容量");
    CHECK_EQ(size(&v), 5, "重复收缩不应该改变大小");
    for (int index = 0; index < 5; index++) {
        CHECK_EQ(GET(&v, index), index, "重复收缩不应该破坏已有元素");
    }

    clear(&v);
    CHECK_OK(shrink_to_fit(&v), "空 vector 的 shrink_to_fit 应该成功");
    CHECK_EQ(capacity(&v), 0, "空 vector 收缩之后 capacity 应该是 0");
    CHECK_EQ(size(&v), 0, "空 vector 收缩之后 size 应该是 0");
    CHECK_BOOL(v.data == NULL && v.end == NULL && v.cap == NULL,
               "空 vector 收缩之后三个指针应该都是 NULL");

    CHECK_OK(push_back(&v, 1), "收缩到 0 之后 push_back 应该成功");
    CHECK_EQ(GET(&v, 0), 1, "收缩到 0 之后还能继续 push_back");

    vector_destroy(&v);
}

static void test_clear(void) {
    vector v = make_vector(8);
    for (int i = 1; i <= 3; i++) {
        CHECK_OK(push_back(&v, i), "push_back 应该成功");
    }

    int *data = v.data;
    int *cap = v.cap;
    clear(&v);
    CHECK_EQ(size(&v), 0, "clear 之后 size 应该是 0");
    CHECK_EQ(empty(&v), 1, "clear 之后 empty 应该是 1");
    CHECK_EQ(capacity(&v), 8, "clear 只清元素不释放内存，capacity 应该不变");
    CHECK_BOOL(v.data == data && v.cap == cap && v.end == data,
               "clear 应该保留缓冲区并重置 end");

    CHECK_OK(push_back(&v, 99), "clear 之后 push_back 应该成功");
    CHECK_EQ(size(&v), 1, "clear 之后可以重新 push_back");
    CHECK_EQ(GET(&v, 0), 99, "clear 之后重新 push 的元素应该能读到");

    vector_destroy(&v);
}

/* ===================== 错误处理 ===================== */

static void test_index_errors(void) {
    vector v = make_vector(4);
    CHECK_OK(push_back(&v, 10), "push_back 应该成功");
    CHECK_OK(push_back(&v, 20), "push_back 应该成功");

    const size_t indices[] = {2, 3, 100, SIZE_MAX - 1, SIZE_MAX};
    const int sentinels[] = {INT_MIN, 234, INT_MAX};
    for (size_t index = 0; index < sizeof(indices) / sizeof(indices[0]);
         index++) {
        for (size_t sentinel = 0; sentinel < 3; sentinel++) {
            int out = sentinels[sentinel];
            CHECK_EQ(get(&v, indices[index], &out), -1, "越界 get 应该返回 -1");
            CHECK_EQ(out, sentinels[sentinel], "get 失败时不应该写入 *out");
        }
        CHECK_EQ(set(&v, indices[index], 999), -1, "越界 set 应该返回 -1");
        CHECK_EQ(size(&v), 2, "set 越界不应该改变 size");
        CHECK_EQ(capacity(&v), 4, "set 越界不应该改变 capacity");
        CHECK_EQ(GET(&v, 0), 10, "set 越界不应该影响已有元素");
        CHECK_EQ(GET(&v, 1), 20, "set 越界不应该影响已有元素");
    }

    /* 边界值：最后一个合法下标必须成功 */
    int out = NOT_WRITTEN;
    CHECK_EQ(get(&v, 1, &out), 0, "get(size - 1) 应该成功");
    CHECK_EQ(out, 20, "get(size - 1) 应该读到末元素");
    CHECK_EQ(set(&v, 1, 21), 0, "set(size - 1) 应该成功");
    CHECK_EQ(GET(&v, 1), 21, "set(size - 1) 应该改掉末元素");

    vector_destroy(&v);
}

static void test_empty_errors(void) {
    const size_t capacities[] = {0, 2};
    const size_t indices[] = {0, 1, SIZE_MAX - 1, SIZE_MAX};
    const int sentinels[] = {INT_MIN, 234, INT_MAX};
    for (size_t scenario = 0; scenario < 2; scenario++) {
        vector v = make_vector(capacities[scenario]);
        for (size_t sentinel = 0; sentinel < 3; sentinel++) {
            int out = sentinels[sentinel];
            CHECK_EQ(front(&v, &out), -1, "空 vector 的 front 应该返回 -1");
            CHECK_EQ(out, sentinels[sentinel], "front 失败时不应该写入 *out");
            out = sentinels[sentinel];
            CHECK_EQ(back(&v, &out), -1, "空 vector 的 back 应该返回 -1");
            CHECK_EQ(out, sentinels[sentinel], "back 失败时不应该写入 *out");
            out = sentinels[sentinel];
            vector before = v;
            CHECK_EQ(pop_back(&v, &out), -1,
                     "空 vector 的 pop_back 应该返回 -1");
            CHECK_EQ(out, sentinels[sentinel],
                     "pop_back 失败时不应该写入 *out");
            CHECK_BOOL(v.data == before.data && v.end == before.end &&
                           v.cap == before.cap,
                       "pop_back 失败时三个指针应该保持不变");
            CHECK_EQ(size(&v), 0, "pop_back 失败时 size 应该保持不变");
            CHECK_EQ(capacity(&v), capacities[scenario],
                     "pop_back 失败时 capacity 应该保持不变");
            for (size_t index = 0; index < 4; index++) {
                out = sentinels[sentinel];
                CHECK_EQ(get(&v, indices[index], &out), -1,
                         "空 vector 的 get 应该返回 -1");
                CHECK_EQ(out, sentinels[sentinel], "get 失败时不应该写入 *out");
            }
        }
        for (size_t index = 0; index < 4; index++) {
            CHECK_EQ(set(&v, indices[index], 1), -1,
                     "空 vector 的 set 应该返回 -1");
        }
        CHECK_EQ(size(&v), 0, "失败的 set 不应该让 size 变大");
        CHECK_EQ(capacity(&v), capacities[scenario],
                 "失败的 set 不应该改变容量");

        CHECK_OK(push_back(&v, 5), "push_back 应该成功");
        CHECK_EQ(FRONT(&v), 5, "front 应该读到 5");
        CHECK_EQ(BACK(&v), 5, "back 应该读到 5");
        vector_destroy(&v);
    }
}

static void test_int_values(void) {
    const int values[] = {INT_MIN, -1, 0, INT_MAX};
    vector v = make_vector(0);
    for (size_t index = 0; index < 4; index++) {
        CHECK_OK(push_back(&v, values[index]), "所有 int 值都应该可以存入");
    }
    for (size_t index = 0; index < 4; index++) {
        int out = 234;
        CHECK_OK(get(&v, index, &out), "极值元素应该可以读取");
        CHECK_EQ(out, values[index], "读取结果应该保留完整 int 值");
        CHECK_OK(set(&v, index, values[3 - index]), "极值元素应该可以设置");
        out = 234;
        CHECK_OK(get(&v, index, &out), "设置后应该可以读取");
        CHECK_EQ(out, values[3 - index], "set 应该保留完整 int 值");
    }
    int out = 234;
    CHECK_OK(front(&v, &out), "front 应该成功");
    CHECK_EQ(out, INT_MAX, "front 应该保留 INT_MAX");
    out = 234;
    CHECK_OK(back(&v, &out), "back 应该成功");
    CHECK_EQ(out, INT_MIN, "back 应该保留 INT_MIN");
    for (size_t index = 0; index < 4; index++) {
        CHECK_EQ(POP(&v), values[index], "pop_back 应该保留完整 int 值");
    }
    CHECK_EQ(size(&v), 0, "弹出全部极值元素后应该为空");
    vector_destroy(&v);
}

static void test_capacity_limit(void) {
    /* 超过上限：必须在动手分配之前就拒绝 */
    int sentinel = 123;
    vector a = {&sentinel, &sentinel, &sentinel};
    CHECK_EQ(vector_init(&a, SIZE_MAX), -1, "capacity 超过上限时应该返回 -1");
    CHECK_BOOL(a.data == NULL && a.end == NULL && a.cap == NULL,
               "vector_init 失败后三个指针应该都是 NULL");

    /* 刚好越过上限：capacity * sizeof(int) 在这里会回绕，
     * 漏掉上限检查的实现会以为分配成功了，然后返回 0 */
    vector b = {&sentinel, &sentinel, &sentinel};
    CHECK_EQ(vector_init(&b, SIZE_MAX / sizeof(int) + 1), -1,
             "capacity 刚好超过上限时应该返回 -1");
    CHECK_BOOL(b.data == NULL && b.end == NULL && b.cap == NULL,
               "vector_init 失败后三个指针应该都是 NULL");

    /* 上限之内但系统给不出来：这就是真的分配失败，同样要干净地返回 -1 */
    vector c = {&sentinel, &sentinel, &sentinel};
    CHECK_EQ(vector_init(&c, SIZE_MAX / sizeof(int)), -1,
             "系统给不出这么多内存时应该返回 -1");
    CHECK_BOOL(c.data == NULL && c.end == NULL && c.cap == NULL,
               "分配失败后三个指针应该都是 NULL");

    /* reserve 失败：容量和元素必须原封不动 */
    vector v = make_vector(2);
    CHECK_OK(push_back(&v, 7), "push_back 应该成功");

    CHECK_EQ(reserve(&v, SIZE_MAX), -1,
             "capacity 超过上限时 reserve 应该返回 -1");
    CHECK_EQ(capacity(&v), 2, "reserve 失败后 capacity 不应该变");
    CHECK_EQ(size(&v), 1, "reserve 失败后 size 不应该变");
    CHECK_EQ(GET(&v, 0), 7, "reserve 失败后元素应该还在");

    CHECK_EQ(reserve(&v, SIZE_MAX / sizeof(int)), -1,
             "扩容到系统给不出的容量应该返回 -1");
    CHECK_EQ(capacity(&v), 2, "reserve 失败后 capacity 不应该变");
    CHECK_EQ(size(&v), 1, "reserve 失败后 size 不应该变");
    CHECK_EQ(GET(&v, 0), 7, "reserve 失败后元素应该还在");

    /* 上面这一组会直接踩到「realloc 的返回值覆盖了 data」那个坑：
     * 漏掉上限检查的实现到这里会把 data 变成悬垂指针，ASan 会当场报
     * heap-use-after-free。放在最后，是为了让前面几组的结论先打印出来。 */
    CHECK_EQ(reserve(&v, SIZE_MAX / sizeof(int) + 1), -1,
             "capacity 刚好超过上限时 reserve 应该返回 -1");
    CHECK_EQ(capacity(&v), 2, "reserve 失败后 capacity 不应该变");
    CHECK_EQ(size(&v), 1, "reserve 失败后 size 不应该变");
    CHECK_EQ(GET(&v, 0), 7, "reserve 失败后元素应该还在");

    /* 失败之后这个 vector 还得能继续用 */
    CHECK_OK(push_back(&v, 8), "reserve 失败之后 push_back 应该仍然能成功");
    CHECK_EQ(GET(&v, 1), 8, "reserve 失败之后新元素应该能读到");

    vector_destroy(&v);
}

/* ===================== 入口 ===================== */

/* 跑一个用例，按「这个用例里有没有失败」给结果行上色。
 * 断言成功时是安静的，所以每个用例只在结尾留一行结果 —— 这样全绿的
 * 时候也能一眼看出用例都跑过了，而不是什么都没有。 */
static void run_case(const char *name, void (*case_fn)(void)) {
    int fails_before = g_fail;

    case_fn();

    int failed = g_fail - fails_before;
    if (failed == 0) {
        printf("%s[ OK ]%s %s\n", green(), reset(), name);
    } else {
        printf("%s[FAIL]%s %s（%d 项失败）\n", red(), reset(), name, failed);
    }
}

int main(void) {
    printf("== lr-core-vector 测试 ==\n\n");

    run_case("vector_init / vector_destroy", test_init_destroy);
    run_case("push_back / size", test_push_back);
    run_case("自动扩容", test_auto_grow);
    run_case("get / set / front / back", test_access);
    run_case("int 极值与负数", test_int_values);
    run_case("pop_back", test_pop_back);
    run_case("reserve", test_reserve);
    run_case("shrink_to_fit", test_shrink_to_fit);
    run_case("clear", test_clear);
    run_case("错误处理：下标越界", test_index_errors);
    run_case("错误处理：空 vector", test_empty_errors);
    run_case("错误处理：capacity 上限与分配失败", test_capacity_limit);

    printf("\n== %s通过 %d 项，失败 %d 项%s ==\n", g_fail ? red() : green(),
           g_pass, g_fail, reset());
    if (g_fail != 0) {
        printf("还没全绿，继续改 src/vector.c\n");
        return 1;
    }
    printf("全部通过，可以 commit & push 了\n");
    return 0;
}
