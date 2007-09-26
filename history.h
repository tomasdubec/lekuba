#ifndef _HISTORY_H_
#define _HISTORY_H_
#include <string>
#include <list>

using namespace std;

typedef struct zprava_{
	bool sent;
	double date;
	string text;
};

typedef std::list<zprava_> zprava;
typedef zprava::iterator zpravaIter;

class History{
	zprava history;
	zpravaIter aktMessage;

	string replaceNewLines(string);
public:
	History(void);
	~History();
	void addSent(string);
	void addReceived(string);
	void reset(){aktMessage = history.begin();}
	bool next();
	void getMessage(bool&, double&, string&);
};

#endif

