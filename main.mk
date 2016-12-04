CROSS_COMPILE ?= 
PWD= $(shell pwd)
SRC_DIR       ?= $(PWD)
BUILD_DIR     ?= $(PWD)
INSTALL_DIR   ?= $(PWD)
TARGET = $(INSTALL_DIR)/${PRJ_TARGET}

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

OBJS = $(patsubst $(notdir %.c), $(BUILD_DIR)/%.o, $(PRJ_SRC))


LDFLAGS += $(PRJ_LDFLAG)
ifeq "$(PRJ_DEBUG)" "yes"
CFLAGS  += -ggdb -g3 -O0
else
CFLAGS  += -o2
endif
CFLAGS  += -Wall -Werror -Wno-unused -Wno-unused-variable $(PRJ_CFLAG)

all:$(TARGET)
$(TARGET):$(PRJ_TARGET_TYPE)
ifeq "$(PRJ_TARGET_TYPE)" "shared"
CFLAGS += -fPIC
endif

exe:$(OBJS)
	$(CC) -o $(TARGET) $^ $(LDFLAGS) 

static:$(OBJS)
	$(AR) -crv $(TARGET).a $^

shared:$(OBJS)
	$(CC) -shared -fPIC $(LDFLAGS) -o $(TARGET).so $^


$(OBJS):$(BUILD_DIR)/%.o:$(SRC_DIR)/%.c
	@if [ ! -e $(@D) ]; then mkdir -p $(@D); fi
	$(CC) $(CFLAGS) -o $@  -c $<

.c.o:
	$(CC) $(CFLAGS) -o $@  -c $<

clean:
	rm -rf  $(TARGET) $(OBJS)
