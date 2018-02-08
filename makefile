MODULE_BIN = Test
MODULE_OBJS += $(patsubst %.c,%.o ,$(wildcard ./*.c))
MODULE_OBJS += $(patsubst %.c,%.o ,$(wildcard commonList/*.c))
MODULE_OBJS += $(patsubst %.c,%.o ,$(wildcard hashTable/*.c))
MODULE_OBJS += $(patsubst %.c,%.o ,$(wildcard maxHeap/*.c))
MODULE_OBJS += $(patsubst %.c,%.o ,$(wildcard rbTree/*.c))
MODULE_OBJS += $(patsubst %.c,%.o ,$(wildcard skipList/*.c))
MODULE_OBJS += $(patsubst %.c,%.o ,$(wildcard unionFindSet/*.c))
LDFLAGS = -lm -lz
EXFLAGS = -g -O0 -Wall
MODULE_INCLUDE += -I. -I./commonList -I./hashTable
MODULE_INCLUDE += -I./maxHeap -I./rbTree -I./skipList
MODULE_INCLUDE += -I./unionFindSet
OUT_DIR = .

%.o: %.c
	gcc $(MODULE_INCLUDE) $(EXFLAGS) -c $< -o $@

all : clean $(MODULE_OBJS)
	gcc $(MODULE_OBJS) $(LDFLAGS) $(EXFLAGS) -o $(OUT_DIR)/$(MODULE_BIN)

clean:
	-rm -f $(MODULE_BIN)
	-rm -f $(MODULE_OBJS)

