#ifndef _HISTORY_H_
#define _HISTORY_H_
#include <string>
#include <list>
#include <fstream>
#include <iostream>

using namespace std;

typedef struct zprava_{
	bool sent;
	long date;
	string text;
};

typedef std::list<zprava_> zprava;
typedef zprava::iterator zpravaIter;

class History{
	zprava history;
	zpravaIter aktMessage;
	string jid;

	string replaceNewLines(string);
public:
	History();
	~History();
	void addSent(string);
	void addReceived(string);
	void addSent(string, long);
	void addReceived(string, long);
	void reset(){aktMessage = history.begin();}
	bool next();
	void getMessage(bool&, double&, string&);
	bool saveToDisk(void);
	bool loadFromDisk(void);
	void setJid(string);
	void load(void);
	void save(void);
	void clear(void){history.clear();}
};

#endif

