CC=gcc
MAKE=make

#编译选项
LIB=-lpthread -lm -lodbc -lcrypto
#-lodbc
PUBLIC_DEPEND=$(PUBDIR)/*.o

SOURCE=main.c user_mp_list.c sql.c sql_fun.c logs.c net.c server_mutual.c radius12.c radius13.c portal_send.c portal_recv.c cJSON.c
DEPEND=main.o user_mp_list.o sql.o sql_fun.o logs.o net.o server_mutual.o radius12.o radius13.o portal_send.o portal_recv.o cJSON.o
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
