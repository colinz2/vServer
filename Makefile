SRC_DIR?=.
all:bsd tool

bsd:
	make -f $(SRC_DIR)/bs.mk
tool:
	gcc -o bs_gen_ip_list gen_ip_list.c

install:
	install bsd bs_gen_ip_list -D /usr/local/bin

clean:
	make -f $(SRC_DIR)/bs.mk clean
	rm -f bs_gen_ip_list
