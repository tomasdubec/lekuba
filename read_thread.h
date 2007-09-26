#ifndef __read_thread_h__
#define __read_thread_h__

#include "connection.h"
#include "xml_wrap.h"
#include <signal.h>
#include <list>

typedef struct rcvStanza_{
	string id;
	string from;
	//string to;
	stanzaType type;
	string data;
};

typedef std::list<rcvStanza_> rcvStanza;
typedef rcvStanza::iterator rcvStanzaIter;

class readThread{
	konexe *conn;
	pthread_t thr;
	rcvStanza inStanza;

	void(*presenceCallback)(void*,const int);
	void(*messageCallback)(void*);
	void *callbackObj;

	static void *thrLoop(void*);
	static void thrTerm(int signal);
	string stripResource(string);
public:
	readThread(konexe *);
	~readThread();
	bool getStanzaById(int id, rcvStanza_ &ret);
	bool getStanzaByType(stanzaType type, rcvStanza_ &ret);
	//bool getPresenceStanza(rcvStanza_ &ret);
	void setPresenceCallback(void *, void(*)(void*, const int));
	void setMessageCallback(void *, void(*)(void*));
	void print(void);
};

#endif

