CC=gcc

all:lib shell

lib:
	$(CC)  -O2 -shared -fPIC -o  libsqlte3.15.02.so sqlite3.c -D SQLITE_THREADSAFE=2 -lpthread -ldl

shell:
	$(CC)  -O2 -o  sqlite_shell shell.c -lpthread -ldl  -L. -l:libsqlte3.15.02.so
