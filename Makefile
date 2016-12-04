SRC_DIR?=.
all:bsd 

bsd:
	make -f $(SRC_DIR)/bs.mk

clean:
	make -f $(SRC_DIR)/bs.mk clean
