TARGET=test
INCLUDE =../include
LIBS =../libs

COMP_FLAG =-Wall -g -I$(INCLUDE) -std=c99
LD_FLAG =-L$(LIBS) -ltrace

#源文件
CFILES := $(wildcard *.c)
DEP_FILES := $(patsubst %.c,%.d,$(CFILES))
OBJ_FILES := $(patsubst %.c,%.o,$(CFILES))

%.d:%.c
	gcc -MM $^ -MF $@ -I$(INCLUDE) -I.

# 编译当前测试程序的规则 需要执行 make 命令
%.o:%.c %.d
	gcc -c $< $(COMP_FLAG) -o $@

ifeq ($(MAKECMDGOALS),)
$(TARGET): $(OBJ_FILES)
	gcc -o $@ $^ $(LD_FLAG)
-include $(DEP_FILES)
endif

.PHONY: clean
clean:
	rm -f *.d *.o test
