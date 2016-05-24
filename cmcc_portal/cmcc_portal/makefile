INCLUDE = -I./ -I../include

FLAGS = -g -Wall $(INCLUDE)
FLAGS += -fPIC

LIBDIR = -L -lltime -L../lib -lktstd -L/usr/lib64 -L/usr/local/lib -lz -lm -lcrypto


GCC = g++

# for C++ language
CODE.cpp = Portal.cpp \
		   UtilFunc.cpp \
		   SocketFunc.cpp

CPP.o = $(CODE.cpp:.cpp=.o)

OBJS.o = $(CPP.o)

# rule for C++ language
#%.o : %.cpp	
#	$(GCC) $(FLAGS) -o $@ -c $<	
#	@echo $*.o build successfully!......

TARGET = libPortal.so
all : h3cPortalSend h3cPortalServer $(TARGET)	
$(TARGET) : $(CODE.cpp) 
	$(GCC) -shared $(FLAGS) $(CODE.cpp) -o $(TARGET) $(LIBDIR)
	@echo $(TARGET) BUILD OK!.........
h3cPortalSend : send.cpp libPortal.so
	$(GCC) send.cpp -o $@ $(FLAGS) -L. -lPortal $(LIBDIR) 
h3cPortalServer : recv.cpp UtilFunc.cpp SocketFunc.cpp
	$(GCC) recv.cpp UtilFunc.cpp SocketFunc.cpp -o $@ $(FLAGS) $(LIBDIR)
.PHONY:
clean:
	rm -rf $(OBJS.o) $(TARGET)
.PHONY:
install:
	cp $(TARGET) ../lib
	cp $(TARGET) /home/auth_wx/lib
