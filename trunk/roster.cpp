#include "roster.h"

/****************** rosterItem *******************/
rosterItem_::rosterItem_(){
	alert = false;
}

string rosterItem_::escapeSpecialChars(string text){
	int pos = -1;
	string ret = text;

	while((pos = ret.find_first_of('&', pos + 1)) != string::npos){
		ret.replace(pos, 1, "&amp;", 5);
	}
	pos = -1;
	while((pos = ret.find_first_of('"', pos + 1)) != string::npos){
		ret.replace(pos, 1, "&quot;", 6);
	}
	pos = -1;
	while((pos = ret.find_first_of('\'', pos + 1)) != string::npos){
		ret.replace(pos, 1, "&apos;", 6);
	}
	pos = -1;
	while((pos = ret.find_first_of('<', pos + 1)) != string::npos){
		ret.replace(pos, 1, "&lt;", 4);
	}
	pos = -1;
	while((pos = ret.find_first_of('<', pos + 1)) != string::npos){
		ret.replace(pos, 1, "&gt;", 4);
	}
	return ret;
}

void rosterItem_::sendMessage(jabberConnection *jc, string text){
	int id = 0;

	backlog.addSent(text);
	text = escapeSpecialChars(text);
	*jc << xmlStanza::xmlMessage(jid, text, id);
}

const bool rosterItem_::operator<(rosterItem_ a){
	string tmp1, tmp2;

	if(alert == a.alert){
		if(group == a.group){
			if(stav == a.stav){
				tmp1 = name; tmp2 = a.name;
				transform(tmp1.begin(), tmp1.end(), tmp1.begin(), (int(*)(int)) tolower);
				transform(tmp2.begin(), tmp2.end(), tmp2.begin(), (int(*)(int)) tolower);
				return tmp1 < tmp2;
			}
			else{
				return stav < a.stav;
			}
		}
		else{
			return group < a.group;
		}
	}
	else{
		return alert;
	}
}

/****************** roster **********************/
Roster::Roster(jabberConnection *jabcon){
	jc = jabcon;
	thr = jc->thr;

	pthread_mutex_init(&mutLock, NULL);

	rosterChangeCallback = NULL;
	messageReceivedCallback = NULL;

	loadRoster();
	presenceChange((void *)this, 0);
#ifdef _DEBUG_
	print();
#endif
	thr->setPresenceCallback((void *)this, Roster::presenceChange);
	thr->setMessageCallback((void *)this, Roster::messageReceived);
	messageReceived(this);
	aktBuddy = roster.begin();
	//cout << "size: >>" << roster.size() << "<<" << endl;
}
Roster::~Roster(){
	pthread_mutex_destroy(&mutLock);
	roster.clear();
}

void Roster::presenceChange(void *obj, int id){
	Roster *me;
	rcvStanza_ in;
	rosterItemIter tmp;
	xmlParserWrap xmlPar;
	rosterItem_ item;
	string stav;
	

	me = (Roster *)obj;
	while(me->thr->getStanzaByType(PRESENCE, in)){
		if(!me->findByJid(in.from, tmp)){
#ifdef _DEBUG_
			cerr << "received presence info from buddy whhich is not in roster, strange, ignoring..\n";
#endif
		}
		else{
			xmlPar.parse(in.data);
			if(xmlPar.getErrorString() == ""){
				stav = xmlPar.getPresenceShow();
				transform(stav.begin(), stav.end(), stav.begin(), (int(*)(int)) tolower);
				me->lock();
				if(stav == "online")
					(*tmp).stav = S_ON;
				else if(stav == "away")
					(*tmp).stav = S_AWAY;
				else if(stav == "xa")
					(*tmp).stav = S_NA;
				else if(stav == "chat")
					(*tmp).stav = S_CHAT;
				else if(stav == "dnd")
					(*tmp).stav = S_DND;
				else if(stav == "offline")
					(*tmp).stav = S_OFF;
				else
					(*tmp).stav = S_UNKNOWN;

			}
			else{
				me->lock();
				(*tmp).stav = S_UNKNOWN;
			}
			me->roster.sort();
			me->unlock();

			if(me->rosterChangeCallback != NULL){
				(me->rosterChangeCallback)(me->callbackObj);
			}
#ifdef _DEBUG_
			cout << "buddy " << in.from << "(" << (*tmp).jid << ")" << " changed status to " << stav << endl;
#endif
		}
	}
	while(me->thr->getStanzaByType(SUBSCRIBE, in)){
		me->lock();
		while(!me->findByJid(in.from, tmp)){
			//has to be done this way, can't use add buddy, because i'm the thread that's necesary for this addBuddy, ouroboros ;-)
			item.jid = in.from;
			item.name = in.from;
			item.stav = S_NOTINLIST;
			item.sub = S_REQUEST;
			item.alert = true;
			me->roster.push_back(item);

			me->findByJid(in.from, tmp);
		}
		tmp->sub = S_REQUEST;
		tmp->alert = true;
		me->roster.sort();
		me->unlock();
		if(me->rosterChangeCallback != NULL)
			(me->rosterChangeCallback)(me->callbackObj);
	}
}

void Roster::messageReceived(void *obj){
	Roster *me;
	rcvStanza_ in;
	rosterItemIter tmp;
	xmlParserWrap xmlPar;
	
	me = (Roster *)obj;
	while(me->thr->getStanzaByType(MESSAGE, in)){
		if(!me->findByJid(in.from, tmp)){
			//do nothing
		}
		else{
			xmlPar.parse(in.data);
			me->lock();
			tmp->backlog.addReceived(xmlPar.getMessageBody());
			tmp->alert = true;
			me->roster.sort();
			me->unlock();
		}
		if(me->rosterChangeCallback != NULL){
			(me->rosterChangeCallback)(me->callbackObj);
		}
		/*if(me->messageReceivedCallback != NULL){
			(me->messageReceivedCallback)(me->callbackObj);
		}*/
#ifdef _DEBUG_
			cout << "message from " << in.from << ": \"" << xmlPar.getMessageBody() << "\"\n";
#endif
	}
}

bool Roster::findByJid(string jid, rosterItemIter &ret){
	rosterItemIter iter;

	//strip resource
	jid = jid.substr(0, jid.find_last_of('/'));
	for(iter = roster.begin();iter != roster.end();iter++){
		if((*iter).jid == jid){
			ret = iter;
			return true;
		}
	}
	return false;
}

bool Roster::findByJid(string jid){
	rosterItemIter iter;

	//strip resource
	jid = jid.substr(0, jid.find_last_of('/'));
	for(iter = roster.begin();iter != roster.end();iter++){
		if((*iter).jid == jid){
			aktBuddy = iter;
			return true;
		}
	}
	return false;
}

void Roster::print(void){
	rosterItemIter iter;

	cout << ",------------------------------------------\n";

	cout << "pred forem\n";
	for(iter = roster.begin();iter != roster.end();iter++){
		cout << "|" << (*iter).jid << "\t|" << (*iter).name << "\t|" << (*iter).stav << endl;
	}
	cout << "'------------------------------------------\n";
}

void Roster::pushToRoster(void){
	rcvStanza_ in;
	xmlParserWrap xmlPar;
	rosterItem_ tmp;
	rosterItemIter tmp1;
	string buf;
	
	while(!thr->getStanzaByType(ROSTER_IQ, in)) sleep(1);
	do{
		if(!xmlPar.parse(in.data)){
			cerr << "unable to parse roster data\n";
			exit(1);
		}

		if(xmlPar.getRosterAtr("subscription") == "remove"){
#ifdef _DEBUG_
			cout << "removing buddy from roster" << endl;
#endif
			if(findByJid(xmlPar.getRosterAtr("jid"), tmp1)){
				roster.erase(tmp1);
				if(rosterChangeCallback != NULL)
					(rosterChangeCallback)(callbackObj);
			}
		}
		else{
#ifdef _DEBUG_
			cout << "adding new buddy to roster" << endl;
#endif
			tmp.name = xmlPar.getRosterAtr("name");
			tmp.jid = xmlPar.getRosterAtr("jid");
			buf = xmlPar.getRosterAtr("subscription");
			if(buf == "from"){
				tmp.sub = S_FROM;
				tmp.stav = S_OFF;
			}
			else if(buf == "to")
				tmp.sub = S_TO;
			else if(buf == "both")
				tmp.sub = S_BOTH;
			else if(buf == "none"){
				tmp.sub = S_NONE;
				tmp.stav = S_OFF;
			}
			tmp.stav = S_OFF;
			if(!findByJid(tmp.jid, tmp1)){  //if it's already in roster, update info
				roster.push_back(tmp);
			}
			else{
				tmp1->name = tmp.name;
				tmp1->sub = tmp.sub;
			}
		}
	}
	while(thr->getStanzaByType(ROSTER_IQ, in));
}

void Roster::loadRoster(void){
	int id;
	rcvStanza_ in;
	xmlParserWrap xmlPar;
	rosterItem_ tmp;
	string buf;

	lock();
	if(!roster.empty()) roster.erase(roster.begin(), roster.end());
	
	*jc << xmlStanza::xmlRosterQuery(jc->getJID(), id);
	
	//while(!thr->getStanzaById(id, in)) sleep(1);  //wait for roster
	while(!thr->getStanzaByType(ROSTER_IQ, in)) sleep(1);  //wait for roster

	if(!xmlPar.parse(in.data)){
		cerr << "unable to parse roster data\n";
		exit(1);
	}

	do{
#ifdef _DEBUG_
		cout << "adding new buddy to roster" << endl;
#endif
		tmp.name = xmlPar.getRosterAtr("name");
		if(tmp.name == "") //roster is empty
			break;
		tmp.jid = xmlPar.getRosterAtr("jid");
		tmp.group = xmlPar.getRosterGroup();
		buf = xmlPar.getRosterAtr("subscription");
		if(buf == "from")
			tmp.sub = S_FROM;
		else if(buf == "to")
			tmp.sub = S_TO;
		else if(buf == "both")
			tmp.sub = S_BOTH;
		else if(buf == "none")
			tmp.sub = S_NONE;
		tmp.stav = S_OFF;
		roster.push_back(tmp);
	}
	while(xmlPar.nextRosterItem());
	roster.sort();
	unlock();
}

bool Roster::addBuddy(string jid, string name, string group){
	int id;
	rcvStanza_ in;
	rosterItemIter tmp;

	*jc << xmlStanza::xmlRosterAdd(jc->getJID(), jid, name, group, id);

	while(!thr->getStanzaById(id, in));
	
	if(in.type == ERROR)
		return false;

	pushToRoster();
	
	if(findByJid(jid, tmp)){
		tmp->group = group;
	}

	roster.sort();

	return true;
}

bool Roster::delBuddy(string jid){
	int id;
	rcvStanza_ in;

	*jc << xmlStanza::xmlRosterRemove(jc->getJID(), jid, id);


	pushToRoster();

	while(!thr->getStanzaById(id, in)) sleep(1);
	
	if(in.type == ERROR)
		return false;
	
	roster.sort();

	return true;
}

void Roster::changeBuddySubscription(string jid, string type){
	rosterItemIter rii;

	findByJid(jid, rii);
	if((*rii).sub == S_REQUEST)
		(*rii).sub = S_NONE;

	*jc << xmlStanza::xmlPresenceSubscription(jid, type);
	//pushToRoster();
}

bool Roster::next(void){
	if(aktBuddy != roster.end() && ++aktBuddy != roster.end()){
		return true;
	}
	else{
		return false;
	}
}

void Roster::setRosterChangeCallback(void *obj, void( *cb)(void*)){
	rosterChangeCallback = cb;
	callbackObj = obj;
}

void Roster::setMessageReceivedCallback(void *obj, void( *cb)(void*)){
	messageReceivedCallback = cb;
	callbackObj = obj;
}

