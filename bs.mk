PRJ_TARGET = bsd
PRJ_TARGET_TYPE = exe

ifndef PRJ_DEBUG
PRJ_DEBUG = no
endif
MK_DIR ?= $(PWD)

PRJ_SRC = 	\
	core/dev_heap.c \
	core/dev_event.c \
	core/dev_event_timer.c \
	bs_utils.c \
	bs_packet.c \
	bs_vserver.c \
	bs_main.c \
	

#PRJ_CFLAG 
PRJ_LDFLAG = -lrt 
PRJ_CFLAG = 

include ${MK_DIR}/main.mk
