INCLUDE =../include
LIBS =../libs

COMP_FLAG =-Wall -g -fPIC -std=c99 -I$(INCLUDE)
LD_FLAG =-shared -L$(LIBS) -ltrace
CC = gcc

#源文件
CFILES := $(wildcard *.c)
DEP_FILES := $(patsubst %.c,%.d,$(CFILES))
OBJ_FILES := $(patsubst %.c,%.o,$(CFILES))

%.d:%.c
	$(CC) -MM $^ -MF $@ -I$(INCLUDE)

# 编译当前测试程序的规则 需要执行 make 命令
%.o:%.c %.d
	$(CC) -c $< $(COMP_FLAG) -o $@

ifeq ($(MAKECMDGOALS),)
libasynpool.so: $(OBJ_FILES)
	$(CC) -o $(LIBS)/$@ $^ $(LD_FLAG)
-include $(DEP_FILES)
endif

.PHONY: clean
clean:
	rm -f *.d *.o $(LIBS)/*.so
