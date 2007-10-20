#ifndef __roster_h__
#define __roster_h__

#include <pthread.h>
#include "jabber_connection.h"
#include "xml_wrap.h"
#include "history.h"

using namespace std;

enum enumSubscription{
	S_FROM,
	S_TO,
	S_BOTH,
	S_REQUEST,
	S_NONE
};

typedef struct rosterItem_{
	string jid;
	string name;
	string group;
	History backlog;
	enumSubscription sub;
	enumStav stav;
	bool alert;
	
	rosterItem_();
	~rosterItem_();
	void sendMessage(jabberConnection *, string text);
	const bool operator<(rosterItem_);

private:
	string escapeSpecialChars(string text);
};
typedef std::list<rosterItem_> rosterItem;
typedef rosterItem::iterator rosterItemIter;

class Roster{
	rosterItem roster;
	rosterItemIter aktBuddy;
	jabberConnection *jc;
	readThread *thr;
	pthread_mutex_t mutLock;
	void(*rosterChangeCallback)(void*);
	void(*messageReceivedCallback)(void*);
	void *callbackObj;

	void loadRoster(void);
	void pushToRoster(void);
	bool findByJid(string, rosterItemIter &);
public:
	Roster(jabberConnection *);
	~Roster();
	/*void lock(void){cout << "locking\n";pthread_mutex_lock(&mutLock);cout << "locked\n";}
	void unlock(void){cout << "unlocking\n";pthread_mutex_unlock(&mutLock);cout << "unlocked\n";}*/
	bool isEmpty(void){return roster.empty();}
	void lock(void){pthread_mutex_lock(&mutLock);}
	void unlock(void){pthread_mutex_unlock(&mutLock);}
	bool findByJid(string);
	void print(void);
	bool addBuddy(string jid, string name, string group);
	bool delBuddy(string jid);
	void changeBuddySubscription(string jid, string type);
	static void presenceChange(void *,int id);
	static void messageReceived(void *);
	void setRosterChangeCallback(void *obj, void( *cb)(void*));
	void setMessageReceivedCallback(void *obj, void( *cb)(void*));

	void reset(void){aktBuddy = roster.begin();}
	bool next(void);
	int size(void){return roster.size();}
	string getBuddyJID(void){return (*aktBuddy).jid;}
	string getBuddyName(void){return (*aktBuddy).name;}
	string getBuddyGroup(void){return (*aktBuddy).group;}
	bool getBuddyAlert(void){return (*aktBuddy).alert;}
	enumStav getBuddyStatus(void){return (*aktBuddy).stav;}
	enumSubscription getBuddySubscription(void){return (*aktBuddy).sub;}
	History *getBuddyHistory(void){return &((*aktBuddy).backlog);}
	void setBuddyEasy(void){(*aktBuddy).alert = false;roster.sort();}
	void sendMessage(string text){aktBuddy->sendMessage(jc, text);}
};

#endif

