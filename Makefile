# Makefile —— lr-core-vector 练习仓库
#
# 常用目标：
#   make test     编译并运行测试（第一个目标，所以直接 make 也是它）
#   make all      只编译，产物是 build/test
#   make clean    删掉 build/ 目录
#   make format   用 clang-format 格式化 src/、include/ 和 tests/
#   make help     打印这份帮助
#
# 可覆盖的变量：
#   make test SANITIZE=0    关掉 ASan / UBSan
#   make CC=clang           换编译器

CC       ?= cc
CSTD     ?= c11
CFLAGS   ?= -std=$(CSTD) -Wall -Wextra -Wpedantic -g
LDFLAGS  ?=

# 默认打开 AddressSanitizer + UndefinedBehaviorSanitizer。
# 数组越界、重复 free、use-after-free、内存泄漏、有符号溢出都会直接报出来，
# 学内存管理时这两个工具比 printf 好用得多。
#
# 测试里会故意请求一块不可能分配到的内存，用来验证分配失败时的返回值。
# 默认情况下 ASan 遇到这种请求会直接终止进程（allocation-size-too-big），
# allocator_may_return_null=1 让它改成像真实 malloc 一样返回 NULL。
SANITIZE ?= 1
ifeq ($(SANITIZE),1)
SANFLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer
endif
SANENV := ASAN_OPTIONS=allocator_may_return_null=1

SRCDIR   := src
INCDIR   := include
TESTDIR  := tests
BUILDDIR := build

BIN  := $(BUILDDIR)/test
SRCS := $(SRCDIR)/vector.c $(TESTDIR)/test.c
OBJS := $(SRCS:%.c=$(BUILDDIR)/%.o)
DEPS := $(OBJS:.o=.d)

CPPFLAGS += -I$(INCDIR)

.PHONY: test all clean format help

test: $(BIN)
	@$(SANENV) ./$(BIN)

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(SANFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SANFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILDDIR)

format:
	clang-format -i $(SRCDIR)/vector.c $(INCDIR)/vector.h $(TESTDIR)/test.c

help:
	@echo "make test     编译并运行测试"
	@echo "make all      只编译"
	@echo "make clean    删除 build/ 目录"
	@echo "make format   用 clang-format 格式化源码"

-include $(DEPS)
