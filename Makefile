CC=gcc
MAKE=make

#编译选项
LIB=-lpthread -lm -lodbc -lcrypto
#-lodbc
PUBLIC_DEPEND=$(PUBDIR)/*.o

SOURCE=main.c utils.c user_mp_list.c sql.c exec_sql.c logs.c net.c server_mutual.c radius.c portal.c cJSON.c
DEPEND=main.o utils.o user_mp_list.o sql.o exec_sql.o logs.o net.o server_mutual.o radius.o portal.o cJSON.o
TARGET=cp.out


all:clean out mv
	@ctags -R *
	@echo "-- ok!-- "

out:
	@$(CC) -c $(SOURCE)
	@$(CC) $(DEPEND) -o $(TARGET) $(LIB)

mv:
	@mv $(TARGET) ~/a.out/
	-@rm -f *.o

GDB:
	@echo "-- ok!-- "

clean:
	-@rm -f *.out *.o
