# client @author:hjh
# 定义编译器、编译选项、库文件路径等
CC = gcc
TARGET = testserver

# 1. 编译选项（CFLAGS）：头文件路径（-I）+ 警告选项（-Wall 显示所有警告，便于调试）
CFLAGS = -Wall  # 开启所有警告，提前发现代码问题（如隐式声明）
CFLAGS += -I./  # 当前目录的头文件（如 transferdata.h、package.h）
CFLAGS += -I../install/include  # 上级 install/include 的头文件（如 mysqlite.h）

# 2. 链接选项（LDFLAGS）：库路径（-L），用 += 追加，不覆盖
LDFLAGS = -L./  # 第一优先级：当前目录的库（如 libds18b20.so、libtransferdata.so）
LDFLAGS += -L../install/lib  # 第二优先级：上级 install/lib 的库（如 libmysqlite.so）

# 3. 链接的库（LIBS）：按依赖顺序排列（被依赖的库放后面，如 sqlite3 是 mysqlite 的依赖，放最后）

LIBS += -lsqlite3       # 库 5：SQLite 系统库（mysqlite 的依赖，放最后）

# 4. 依赖文件：目标 test 依赖的源码文件（根据实际情况调整，若用动态库则可删除 .c 文件）
# 若 makepackage 在 libtransferdata.so 中，可删除 transferdata.c；若在源码中，必须保留
SRCS = socket-poll.c transferdata.c  gettime.c mysqlite.c ds18b20.c# 源码文件列表
OBJS = $(SRCS:.c=.o)          # 目标文件列表（将 .c 替换为 .o，如 main.o、transferdata.o）

# 5. 默认目标：all（执行 make 时默认编译 all）
all: $(TARGET)

# 6. 生成可执行文件：依赖 OBJS（目标文件），链接 LDFLAGS 和 LIBS
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS) $(LIBS)
	@echo "编译完成！可执行文件：./$(TARGET)"  # @ 表示不显示命令本身，仅显示提示

# 7. 生成目标文件（.o）：每个 .c 文件生成对应的 .o，依赖对应的 .c 和头文件
# $< 表示第一个依赖文件（如 main.c），$@ 表示目标文件（如 main.o）
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "编译 $< 完成"

# 8. 运行目标：执行生成的可执行文件
run: $(TARGET)  # 依赖 TARGET，确保先编译再运行
	./$(TARGET)

# 9. 清理目标：删除可执行文件、目标文件、数据库文件
clean:
	rm -f $(TARGET) $(OBJS)  # 删除可执行文件和 .o 文件
	rm -f *.db               # 删除 SQLite 数据库文件
	@echo "清理完成！"

# 10. 声明伪目标：避免与同名文件冲突（如当前目录有 all、clean 文件，仍执行目标）
.PHONY: all run clean