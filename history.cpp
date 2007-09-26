#include <string>
#include <iostream>
#include "history.h"

using namespace std;

History::History(void){
	addReceived("session opened");
}

History::~History(){

}

string History::replaceNewLines(string text){
	int index;

	while((index = text.find('\n', 0)) != string::npos){
		text[index] = ' ';
	}
	return text;
}

void History::addSent(string text){
	zprava_ tmp;

	tmp.sent = true;
	tmp.date = time(NULL);
	tmp.text = replaceNewLines(text);
	history.push_front(tmp);
}

void History::addReceived(string text){
	zprava_ tmp;

	tmp.sent = false;
	tmp.date = time(NULL);
	tmp.text = replaceNewLines(text);
	history.push_front(tmp);
}

void History::getMessage(bool &sent, double &date, string &text){
	sent = (*aktMessage).sent;
	date = (*aktMessage).date;
	text = (*aktMessage).text;
}

bool History::next(){
	if(++aktMessage != history.end())
		return true;
	else
		return false;
}

