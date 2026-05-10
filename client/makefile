#运行前export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:`pwd`/INSTALL/libs/
# 关键：指定Bash支持大括号扩展
SHELL := /bin/bash

CC = gcc
TARGET = testclient
INST_PATH = ./INSTALL
# 静态库名（后续链接使用）
STATIC_LIB = $(INST_PATH)/libs/libmyfunction.a

# 源文件列表
SRCS =  transferdata.c client.c gettime.c mysqlite.c ds18b20.c logger.c
# 目标文件列表（所有.o输出到INST_PATH/bin）
OBJS = $(patsubst %.c, $(INST_PATH)/bin/%.o, $(SRCS))

# 编译选项：头文件路径（优先找INSTALL/include）
CFLAGS = -Wall -I$(INST_PATH)/include/
# 链接选项：库路径（优先找INSTALL/libs）
LDFLAGS = -L$(INST_PATH)/libs
# 链接的库：自定义静态库 + SQLite3系统库
LIBS = -lmyfunction -lsqlite3

# 默认目标：先准备目录→编译.o→生成静态库→生成可执行文件
all: prepare $(STATIC_LIB) $(TARGET)

# 1. 准备工作：创建目录+复制头文件/库文件（仅执行一次）
prepare:
	
	mkdir -p $(INST_PATH)/bin $(INST_PATH)/etc $(INST_PATH)/include $(INST_PATH)/libs
	
	cp -f *.h $(INST_PATH)/include/ 2>/dev/null || true        # 复制当前目录头文件到INSTALL/include（-f避免无文件时报错）
	
	cp -f ../install/include/sqlite3.h $(INST_PATH)/include/ 2>/dev/null || true     # 复制外部SQLite头文件（-f+错误重定向，无文件时不中断）
	
	cp -f ../install/lib/lib* $(INST_PATH)/libs/ 2>/dev/null || true   # 复制外部库文件（仅复制存在的lib文件，避免报错）

# 2. 生成静态库：所有.o文件生成后，一次性打包（依赖所有OBJS）
$(STATIC_LIB): $(OBJS)
	ar -rcs $@ $(OBJS)                            # $@ = $(STATIC_LIB)
	@echo "静态库生成完成：$@"

# 3. 生成可执行文件：链接所有.o文件+静态库+SQLite库（依赖静态库和OBJS）
$(TARGET): $(STATIC_LIB) $(OBJS)
	
	$(CC) $(CFLAGS) main.c -o $(INST_PATH)/bin/$(TARGET) $(LDFLAGS) $(LIBS)      # 链接命令：用所有OBJS，按"编译选项→目标文件→链接选项→库"顺序
	@echo "编译完成！可执行文件：$(INST_PATH)/bin/$(TARGET)"

# 4. 编译.o文件：仅处理编译逻辑（无冗余操作）
$(INST_PATH)/bin/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@  # $< = 源文件（如main.c），$@ = 目标文件（如INSTALL/bin/main.o）
	@echo "编译完成：$< → $@"

	

# 6. 清理目标：删除所有编译产物（-f避免无文件时报错）
clean:
	rm -f $(OBJS)                      # 删除所有.o文件
	rm -f $(INST_PATH)/bin/$(TARGET)   # 删除可执行文件
	rm -f $(STATIC_LIB)                # 删除静态库
	rm -f $(INST_PATH)/etc/*.db $(INST_PATH)/include/*.h $(INST_PATH)/libs/*  # 删除头文件/库文件/数据库
	rm -rf $(INST_PATH)                # 可选：删除整个INSTALL目录（按需保留）
	rm -f *.o *.db                     # 删除源码目录下的临时文件
	@echo "清理完成！"

# 声明伪目标：避免与同名文件冲突
.PHONY: all prepare run clean