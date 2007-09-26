#ifndef _XMLWRAP_H_
#define _XMLWRAP_H_

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <iostream>

using namespace std;

enum stanzaType{
	STREAM,
	FEATURES,
	PROCEED,
	CHALLENGE,
	IQ,
	MESSAGE,
	ERROR,
	PRESENCE,
	SUBSCRIBE,
	ROSTER_IQ,
	SASL_FAILURE,
	SASL_SUCCESS,
	UNKNOWN
};

/*class xmlStanza{
	enum stanzaType stType;
	string to, from, id, type;
	xmlDocPtr doc;
public:
	xmlStanza(enum stanzaType);
	~xmlStanza(void);
	void setTo(string);
	void setFrom(string);
	void setId(string);
	bool setType(string);
	string renderXML(void);
};*/

//string sha(string );

class xmlStanza{
	static string sha(string);
	//static string atoi(int);
public:
	static string itoa(int);
	static string xmlStream(string to);
	static string xmlSASLauth(string);
	static string xmlSASLresponse(string);
#ifdef MHASH
	static string xmlStanzaAuth(string username, string password, string resource, string hashid, string id);
#endif
	static string xmlStanzaStartTLS(void);
	static string xmlIqRegister(string, string, string &id);
	static string xmlIqChangePassword(string, string, string, string&);
	static string xmlIqBindResource(string);
	static string xmlIqSession(void);
	static string xmlPresence(int prio,string show="");
	static string xmlPresenceSubscription(string to, string type);
	//static string xmlQuery(string, string, int&);
	static string xmlRosterQuery(string from, int &id);
	static string xmlRosterAdd(string from, string jid, string name, string group, int &id);
	static string xmlRosterRemove(string from, string jid, int &id);
	static string xmlMessage(string to, string text, int &id);
};

class xmlParserWrap{
	xmlDocPtr doc;
	xmlNodePtr node;
	string error;
	string buffer;
	enum stanzaType type;
public:
	xmlParserWrap(void);
	xmlParserWrap(string mem);
	~xmlParserWrap(void);
	bool parse(string data);
	bool hasSubTag(string name); //dost mozna nepouzivane
	bool supportsTLS(void);
	bool supportsMD5(void);
	bool supportsBind(void);
	string getArg(string name);
	string getContent(void);
	string getSASLError(void);
	string getError(void){return error;}
	string getErrorString(void);
	string getBuffer(void){return buffer;}
	string getRosterAtr(string);
	string getRosterGroup(void);
	bool nextRosterItem(void);
	stanzaType getType(void);
	string getPresenceShow(void);
	string getMessageBody(void);
	bool next(void);
	xmlNodePtr getRoot(void);
};

#endif
