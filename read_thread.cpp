#include "read_thread.h"

string readThread::stripResource(string text){
	return text.substr(0, text.find_first_of('/'));
}

readThread::readThread(konexe *c){
	conn = c;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_create(&thr, &attr, thrLoop, (void *)this);
	if(!thr){
		cerr << "error creating thread";
		exit(1);
	}
	messageCallback = NULL;
	presenceCallback = NULL;
}

readThread::~readThread(){
	if(pthread_kill(thr, SIGTERM) != 0){
		cerr << "error killing thread\n";
	}
	pthread_cancel(thr);
}

void readThread::thrTerm(int signal){
#ifdef _DEBUG_
	cout << "************* vlakno konci ****************\n";
#endif
	exit(0);
}

void *readThread::thrLoop(void *th){
	readThread *t = (readThread *)th;
	string buffer;
	rcvStanzaIter iter;
	rcvStanza_ tmp;
	xmlParserWrap xmlPar;

	signal(SIGTERM, t->thrTerm);

#ifdef _DEBUG_
	cout << "************* vlakno se hlasi do sluzby ****************\n";
#endif
	while(true){
		try{
			*t->conn >> buffer;
		}catch(string e){
			cerr << e << endl << "koncim" << endl;
			exit(1);
		}
		if(xmlPar.parse(buffer)){  //ignore anything that's not parseable
			do{
				tmp.from = t->stripResource(xmlPar.getArg("from"));
				transform(tmp.from.begin(), tmp.from.end(), tmp.from.begin(), (int(*)(int)) tolower);
				//tmp.to = xmlPar.getArg("to");
				tmp.id = xmlPar.getArg("id");
				tmp.type = xmlPar.getType();
				tmp.data = xmlPar.getBuffer();
				//tmp.data = buffer;
				t->inStanza.push_back(tmp);
				switch(tmp.type){
				case MESSAGE:
					if(t->messageCallback != NULL)
						(t->messageCallback)(t->callbackObj);
					break;
				case ROSTER_IQ:
					break;
				case PRESENCE: case SUBSCRIBE:
					if(t->presenceCallback != NULL)
						(t->presenceCallback)(t->callbackObj, 10);
					break;
				}
			}while(xmlPar.next());

		}
		else{
			cerr << "unparseable xml received\n";
			exit(1);
		}
#ifdef _DEBUG_
		t->print();
#endif
	}

}

void readThread::print(void){
	rcvStanzaIter iter;

	cout << ",---------------------------------------------------------\n";
	for(iter=inStanza.begin();iter!=inStanza.end();iter++)
		cout << "|" << (*iter).id << "\t|" << (*iter).from << "\t\t|" << (*iter).type << "\t|" << /*(*iter).data <<*/ endl;

	cout << "'---------------------------------------------------------\n";
}

bool readThread::getStanzaById(int id, rcvStanza_ &ret){
	rcvStanzaIter iter;

	if(inStanza.empty()) //queue is empty
		return false;

	for(iter = inStanza.begin();iter != inStanza.end();iter++){
		if((*iter).id == xmlStanza::itoa(id)){
			ret = *iter;
			inStanza.erase(iter);
			return true;
		}
	}

	return false;
}

bool readThread::getStanzaByType(stanzaType type, rcvStanza_ &ret){
	rcvStanzaIter iter;

	if(inStanza.empty()) //queue is empty
		return false;

	for(iter = inStanza.begin();iter != inStanza.end();iter++){
		if((*iter).type == type){
			ret = *iter;
			inStanza.erase(iter);
			return true;
		}
	}

	return false;
}

/*bool readThread::getPresenceStanza(rcvStanza_ &ret){
	rcvStanzaIter iter;

	for(iter = inStanza.begin();iter != inStanza.end();iter++){
		if((*iter).type == PRESENCE){
			ret = (*iter);
			inStanza.erase(iter);
			return true;
		}
	}

	return false;
}*/

void readThread::setPresenceCallback(void *obj, void( *cb)(void*, const int)){
	presenceCallback = cb;
	callbackObj = obj;
}

void readThread::setMessageCallback(void *obj, void( *cb)(void*)){
	messageCallback = cb;
	callbackObj = obj;
}

