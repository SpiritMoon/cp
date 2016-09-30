CC=gcc
MAKE=make

#编译选项
LIB=-lpthread -lm -lpq -lcrypto

SOURCE=exec_sql.c postgresql.c main.c utils.c user_mp_list.c logs.c net.c server_mutual.c radius.c portal.c cJSON.c
DEPEND=exec_sql.o postgresql.o main.o utils.o user_mp_list.o logs.o net.o server_mutual.o radius.o portal.o cJSON.o
TARGET=cp.out


all:clean out mv
	@ctags -R *
	@echo "-- ok!-- "

out:
	@$(CC) -c $(SOURCE)
	@$(CC) $(DEPEND) -o $(TARGET) $(LIB)

mv:
	-@rm -f *.o

GDB:
	@echo "-- ok!-- "

clean:
	-@rm -f *.out *.o
