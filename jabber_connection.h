#ifndef __jabkonexe_h__
#define __jabkonexe_h__

#include <iostream>
#include <list>
#include <pthread.h>
#include "connection.h"
#include "read_thread.h"
#include "xml_wrap.h"

using namespace std;

enum enumError{
	NOT_WELL_FORMED,
	INVALID,

	NONE
};

enum enumStav{
	S_ON,
	S_AWAY,
	S_NA,
	S_CHAT,
	S_DND,
	S_UNKNOWN,
	S_OFF,
	S_NOTINLIST
};

enum enumStanzaType{
	GET,
	SET
};

/*typedef struct rcvStanza_{
	string id;
	string from;
	string to;
	string type;
	string data;
};

typedef std::list<rcvStanza_> rcvStanza;
typedef rcvStanza::iterator rcvStanzaIter;*/

class jabberConnection{
	string username, password, server, resource;
	bool useTls;
	konexe con;
	enumStatus stat;
	enumStav stav;
	//pthread_t readThread;

	void debugPrint(string);
	string base64decode(string);
	string base64encode(string);
	string parseDigest(string digest, string prop);
	bool doSASLMD5(void);
	bool doSASLPLAIN(void);
	string md5(string);
	unsigned char *md5c(unsigned char *co);

	//for SASL
	string genA1hex(string nonce, string cnonce, string authzid, string realm);
	string genA2hex(string);
	string genA2Shex(string);
	string genResponseHex(string nonce, string cnonce, string qop, string A1, string A2);

	string hex(unsigned char *, int);
	bool doSSLHandshake(void){return con.doSSLHandshake();}

	//connection "sitting duck" thread related
	/*void startReadThread(void);
	static void *readStanza(void *);
	static void thrTerm(int signal);*/
public:
	readThread *thr;
	//rcvStanza inStanza;
	
	jabberConnection(string , string pass, string srvr="no server", int prt=5222, bool tls=true); //jid should include username@domain/resource
	~jabberConnection();
	void debug(string nonce, string username, string heslo, string cnonce);
	void setJID(string username); //should terminate any ongoing connection
	void setPassword(string password);
	string getJID(void){ return username+"@"+server+"/"+resource; }
	int registerNewAccount(void);
	int changePassword(void);
	bool login(void);
	bool setStatus(enumStav);
	enumStav getStatus(void){return stav;}
	enumStatus getConnectionStatus(void){return con.getStatus();}
	bool logout(enumError er);
	const jabberConnection& operator<<(const string& txt);
};

#endif

