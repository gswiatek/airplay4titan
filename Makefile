C_SRC=src/mongoose.c

CPP_SRC=src/PropList.cpp\
	src/Mutex.cpp\
	src/Log.cpp\
	src/Util.cpp\
	src/Control.cpp\
	src/AirPlay.cpp\
	src/TitanRemote.cpp\
	src/Server.cpp
	
LIBS=

EXEC=airplay

#CFLAGS=-static-libstdc++ -c -Wall -O2 -DNO_CGI -DNO_SSL -DNDEBUG -Iinclude $(CROSS_CFLAGS)
CFLAGS=-c -Wall -O2 -DNO_CGI -DNO_SSL -DNDEBUG -Iinclude $(CROSS_CFLAGS)

LDFLAGS=$(CROSS_LDFLAGS) $(LIBS) -lpthread -ldl
OBJ=$(CPP_SRC:.cpp=.o) $(C_SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(LDFLAGS) $(LIBS) $(OBJ) -o $@
	$(STRIP) $(EXEC)
	cp $(EXEC) $(ARCH)

.cpp.o:
	$(CXX) $(CFLAGS) -std=c++0x $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ) $(EXEC)

