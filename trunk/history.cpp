#include <string>
#include <iostream>
#include "history.h"

using namespace std;

History::History(void){
	addReceived("session opened");
	jid = "";
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

void History::addSent(string text, long time){
	zprava_ tmp;

	tmp.sent = true;
	tmp.date = time;
	tmp.text = replaceNewLines(text);
	history.push_front(tmp);
}

void History::addReceived(string text, long time){
	zprava_ tmp;

	tmp.sent = false;
	tmp.date = time;
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

void History::setJid(string j){
	jid = j;
}

void History::load(void){
	ifstream in;
	string filename, stime, type, text;
	long time;
	char buf[5000];

	filename = getenv("HOME");
	filename = filename + "/.lekuba/" + jid;
	in.open(filename.c_str());
	if(in.is_open()){
		while(!in.eof()){
			in.getline(buf, 5);
			type = buf;
			in.getline(buf, 20);
			time = strtod(buf, NULL);
			in.getline(buf, 5000);
			text = buf;

			if(type == "recv"){
				addReceived(text, time);
			}
			else if(type == "sent"){
				addSent(text, time);
			}
			else{ /* this probably means, we are at the end of the file, but it wasnt noticed by eof() */
				break;
			}
		}
	}
	in.close();

	addReceived("session opened");
}

void History::save(void){
	ofstream out;
	zpravaIter zi;
	string filename;
	
	if(jid != ""){
		filename = getenv("HOME");
		filename = filename + "/.lekuba/" + jid;
		//out.open(filename.c_str());
		out.open(filename.c_str());
		if(!out.is_open()){
			cerr << "ERROR when opening history file " << filename << endl;
			return;
		}

		zi = history.end();
		do{
			zi--;
			if(zi->sent)
				out << "sent" << endl;
			else
				out << "recv" << endl;

			out << zi->date << endl;

			out << zi->text << endl;
		}
		while(zi != history.begin());

		out.close();
	}
}

