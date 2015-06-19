#DEFS= -DUSE_TLS
DEFS= -DUSE_TLS -DLOGFILE=\"log.txt\"
CFLAGS=$(shell xml2-config --cflags) $(shell pkg-config --cflags openssl) -g
LIBS=$(shell xml2-config --libs) $(shell pkg-config --libs openssl) -lpthread -lncurses

all: lekuba

lekuba: xml_wrap.o connection.o jabber_connection.o roster.o read_thread.o lekuba.o tui.o history.o resource_parser.o
	g++ $(LIBS) -o lekuba lekuba.o connection.o xml_wrap.o jabber_connection.o roster.o read_thread.o tui.o history.o resource_parser.o

connection.o: connection.h connection.cpp
	g++ -c $(CFLAGS) $(DEFS) -o connection.o connection.cpp

jabber_connection.o: jabber_connection.h jabber_connection.cpp connection.h
	g++ -c $(CFLAGS) $(DEFS) -o jabber_connection.o jabber_connection.cpp

xml_wrap.o: xml_wrap.cpp xml_wrap.h
	g++ -c $(CFLAGS) $(DEFS) -o xml_wrap.o xml_wrap.cpp

roster.o: roster.cpp roster.h jabber_connection.h
	g++ -c $(CFLAGS) $(DEFS) -o roster.o roster.cpp

read_thread.o: read_thread.cpp read_thread.h
	g++ -c $(CFLAGS) $(DEFS) -o read_thread.o read_thread.cpp

tui.o: tui.cpp tui.h
	g++ -c $(CFLAGS) $(DEFS) -o tui.o tui.cpp

history.o: history.cpp history.h
	g++ -c $(CFLAGS) $(DEFS) -o history.o history.cpp

resource_parser.o: resource_parser.cpp resource_parser.h
	g++ -c $(CFLAGS) $(DEFS) -o resource_parser.o resource_parser.cpp

lekuba.o: lekuba.cpp
	g++ -c $(CFLAGS) $(DEFS) -o lekuba.o lekuba.cpp

clean:
	rm -f *.o lekuba


#
# comment
# comment
#


#vdsdfv df dfv df
