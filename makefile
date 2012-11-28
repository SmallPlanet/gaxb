LUA_LIB_FLAGS = lua-5.2.0/src/liblua.a
XML_LIB_FLAGS = `xml2-config --cflags --libs`
ALL_LIB_FLAGS = $(LUA_LIB_FLAGS) $(XML_LIB_FLAGS)

LUA_INCLUDE_FLAGS = -Ilua-5.2.0/src/
XML_INCLUDE_FLAGS = `xml2-config --cflags`
ALL_INCLUDE_FLAGS = $(LUA_INCLUDE_FLAGS) $(XML_INCLUDE_FLAGS)

all : clean lua main.o
	gcc main.o -Wall $(ALL_LIB_FLAGS) -o gaxb

lua : $(LUA_LIB)
	make -C lua-5.2.0 generic &> /dev/null

main.o : main.c
	gcc $(ALL_INCLUDE_FLAGS) -c main.c

clean:
	rm -f main.o
	rm -f gaxb