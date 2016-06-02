CC=gcc
MAKE=make

#编译选项
LIB=-lpthread -lm -lodbc
#-lodbc
PUBLIC_DEPEND=$(PUBDIR)/*.o

SOURCE=main.c sql.c logs.c ini.c net.c server_mutual.c radius.c portal.c cJSON.c
DEPEND=main.o sql.o logs.o ini.o net.o server_mutual.o radius.o portal.o cJSON.o
TARGET=cp.out


all:clean out mv
	@ctags -R *
	@echo "-- ok!-- "

out:
	@$(CC) -c $(SOURCE)
	@$(CC) $(DEPEND) -o $(TARGET) $(LIB)

mv:
	@mv $(TARGET) ~/a.out/
	@cp config.ini ~/a.out/
	-@rm -f *.o

GDB:
	@echo "-- ok!-- "

clean:
	-@rm -f *.out *.o
