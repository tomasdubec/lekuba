#include "jabber_connection.h"
#include <string>
#include <signal.h>
//#include <mhash.h>
#include <ctime>
#include <string.h>
#include <openssl/md5.h>

void jabberConnection::debugPrint(string txt){
#ifdef _DEBUG_
	cerr << txt << endl;
#endif
}

string jabberConnection::base64encode(string text){
	int a=0, delka=text.length(), i=0, vyslDelka = (text.length() / 3) * 4 + 1;
	char ret[vyslDelka];
	string *rets, retst;
	char base64table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	memset(ret, '\0', vyslDelka);
	for(;delka>2;delka-=3){
		ret[i++] = base64table[(text[a] >> 2)];
		ret[i++] = base64table[((text[a] << 4) & 0x30) | (text[a+1] >> 4)];
		ret[i++] = base64table[((text[a+1] & 0x0f) << 2) | (text[a+2] >> 6)];
		ret[i++] = base64table[(text[a+2] & 0x3f)];
		a += 3;
	}

	switch(delka){
	case 2: ret[i++] = base64table[(text[a] >> 2) & 0x3f];
		ret[i++] = base64table[((text[a] << 4) & 0x30) | (text[a+1] >> 4)];
		ret[i++] = base64table[(text[a+1] << 2) & 0x3c];
		ret[i++] = '=';
		break;
	case 1: ret[i++] = base64table[(text[a] >> 2) & 0x3f];
		ret[i++] = base64table[(text[a] << 4) & 0x30];
		ret[i++] = '=';
		ret[i++] = '=';
		break;
	}
	ret[i]='\0';
	//rets = new string(ret);
	retst = ret;
	return retst;
	//return *rets;
}

string jabberConnection::base64decode(string co){
	int a=0, delka, i=0;
	string buf, ret;

	ret = co+co;
	delka = co.length();
	for(a=0;a<co.length();a++){
		if(co[a] >= 'A' && co[a] <= 'Z'){
			co[a] = co[a] - 'A';
			continue;
		}
		if(co[a] >= 'a' && co[a] <= 'z'){
			co[a] = co[a] - 'a' + 26;
			continue;
		}
		if(co[a] >= '0' && co[a] <= '9'){
			co[a] = co[a] - '0' + 52;
			continue;
		}
		if(co[a] == '/'){
			co[a] = 63;
			continue;
		}
		if(co[a] == '+'){
			co[a] = 62;
			continue;
		}
		if(co[a] == '='){
			co[a] = 0;
			continue;
		}
	}

	for(a=0;delka>=4;delka-=4){
		ret[i++] = (co[a] << 2) | (co[a+1] >> 4);
		ret[i++] = (co[a+1] << 4) | (co[a+2] >> 2);
		ret[i++] = (co[a+2] << 6) | (co[a+3] & 0x3f);
		a+=4;
	}
	ret = ret.substr(0,i);
	ret = ret.substr(0,ret.find_first_of('\0')); //they may be some zeros on the end (remains after those equals)

	return ret;
}

/*
void jabberConnection::startReadThread(void){
	pthread_attr_t attr;

	debugPrint("\nStarting new thread\n");
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_create(&readThread, &attr, readStanza, (void *)this);
	if(!readThread)
		debugPrint("error creating thread");
}

void jabberConnection::thrTerm(int signal){
	//debugPrint("************* vlakno konci ****************");
	exit(0);
}

void *jabberConnection::readStanza(void *th){
	jabberConnection *t = (jabberConnection *)th;
	string buffer;
	rcvStanzaIter iter;
	rcvStanza_ tmp;
	xmlParserWrap xmlPar;

	signal(SIGTERM, t->thrTerm);

#ifdef _DEBUG_
	cout << "************* vlakno se hlasi do sluzby ****************\n";
#endif
	while(true){
		t->con >> buffer;
		if(xmlPar.parse(buffer)){  //ignore anything that's no parseable
			switch(xmlPar.getType()){
			case MESSAGE:
				tmp.from = xmlPar.getArg("from");
				tmp.to = xmlPar.getArg("to");
				tmp.id = xmlPar.getArg("id");
				tmp.type = xmlPar.getArg("type");
				tmp.data = xmlPar.getContent();
				t->inStanza.push_back(tmp);
				break;
			case ROSTER_IQ:
				tmp.from = xmlPar.getArg("from");
				tmp.to = xmlPar.getArg("to");
				tmp.id = xmlPar.getArg("id");
				tmp.type = xmlPar.getArg("type");
				tmp.data = buffer;
				t->inStanza.push_back(tmp);
				break;
			}
		}
	}

}
*/

string jabberConnection::genA1hex(string nonce, string cnonce, string authzid, string realm){
	unsigned char buf[16];
	MD5_CTX context;

	MD5_Init(&context);
	MD5_Update(&context, username.c_str(), username.length());
	MD5_Update(&context, ":", 1);
	if(realm != "") MD5_Update(&context, realm.c_str(), realm.length());
	MD5_Update(&context, ":", 1);
	MD5_Update(&context, password.c_str(), password.length());
	MD5_Final(buf, &context);  //hash(username::password)

	MD5_Init(&context);
	MD5_Update(&context, buf, 16);
	MD5_Update(&context, ":", 1);
	MD5_Update(&context, nonce.c_str(), nonce.length());
	MD5_Update(&context, ":", 1);
	MD5_Update(&context, cnonce.c_str(), cnonce.length());
	if(authzid != ""){
		MD5_Update(&context, ":", 1);
		MD5_Update(&context, authzid.c_str(), authzid.length());
	}
	MD5_Final(buf, &context);  //hash(hash(username::password):nonce:cnonce:authzid) == A1

	return hex(buf, 16);
}

string jabberConnection::genA2hex(string qop){
	unsigned char buf[16];
	MD5_CTX context;

	MD5_Init(&context);
	MD5_Update(&context, "AUTHENTICATE:xmpp/", 18);
	MD5_Update(&context, server.c_str(), server.length());
	if(qop == "auth-conf" || qop == "auth-int")
		MD5_Update(&context, ":00000000000000000000000000000000", 33);
	MD5_Final(buf, &context);  //hash(AUTHENTICATE:xmpp/server)

	return hex(buf, 16);
}

string jabberConnection::genA2Shex(string qop){
	unsigned char buf[16];
	MD5_CTX context;

	MD5_Init(&context);
	MD5_Update(&context, ":xmpp/", 6);
	MD5_Update(&context, server.c_str(), server.length());
	if(qop == "auth-conf" || qop == "auth-int")
		MD5_Update(&context, ":00000000000000000000000000000000", 33);
	MD5_Final(buf, &context);  //hash(AUTHENTICATE:xmpp/server)

	return hex(buf, 16);
}

string jabberConnection::genResponseHex(string nonce, string cnonce, string qop, string A1, string A2){
	unsigned char buf[16];
	MD5_CTX context;

	MD5_Init(&context);
	MD5_Update(&context, A1.c_str(), A1.length());
	MD5_Update(&context, ":", 1);
	MD5_Update(&context, nonce.c_str(), nonce.length());
	MD5_Update(&context, ":", 1);
	MD5_Update(&context, "00000001", 8);
	MD5_Update(&context, ":", 1);
	MD5_Update(&context, cnonce.c_str(), cnonce.length());
	MD5_Update(&context, ":", 1);
	MD5_Update(&context, qop.c_str(), qop.length());
	MD5_Update(&context, ":", 1);
	MD5_Update(&context, A2.c_str(), A2.length());
	MD5_Final(buf, &context);  //response

	return hex(buf, 16);
}

string jabberConnection::hex(unsigned char *in, int delka){
	int i, h, l;
	string sout;
	unsigned char *out;

	out = (unsigned char *)malloc(2 * delka * sizeof(unsigned char *) + 1);
	for(i = 0; i < delka; i++) {
		h = in[i] & 0xf0;
		h >>= 4;
		l = in[i] & 0x0f;
		out[i * 2] = (h >= 0x0 && h <= 0x9) ? (h + 0x30) : (h + 0x57);
		out[i * 2 + 1] = (l >= 0x0 && l <= 0x9) ? (l + 0x30) : (l + 0x57);
	}
	out[i * 2] = '\0';
	sout = (char *)out;
	free(out);
	return sout;
}

string jabberConnection::parseDigest(string digest, string prop){
	string tmp,firstHalf, secondHalf;
	int index=0, newindex=0;

	digest = digest.substr(0,digest.length())+","; //libxml adds two zero chars at the end
	while(index != -1){
		if(index == 0) index--; //that's just for the first iteration.
		newindex = digest.find_first_of(',', ++index);
		tmp = digest.substr(index, newindex-index); // in tmp we now have something like property=value or property="value" or proprty='value'
		//is the first part the property we are looking for?
		firstHalf = tmp.substr(0, tmp.find_first_of('='));
		if(firstHalf == prop){
			//if it is what we are looking for, read the actual value and remove qotes
			secondHalf = tmp.substr(tmp.find_first_of('=')+1, tmp.length());
			if(secondHalf[0] == '"'){
				secondHalf = secondHalf.substr(1, secondHalf.length()-2);
			}
			else if(secondHalf[0] == '\''){
				secondHalf = secondHalf.substr(1, secondHalf.length()-2);
			}
			return secondHalf;
		}
		else{
			index = newindex;
		}
	}
	return "";
}

bool jabberConnection::doSASLMD5(void){
	string chal, tmp;
	xmlParserWrap xmlPar;
	char buf[6]="00000";

	con << xmlStanza::xmlSASLauth("DIGEST-MD5");
	try{
		con >> tmp;
	}
	catch(string e){
		cout << e << endl;
		exit(1);
	}
	xmlPar.parse(tmp);
	if(xmlPar.getType() == CHALLENGE){
		try{
			chal = xmlPar.getContent();
			chal = base64decode(chal);
		}
		catch(const char* e){
			cerr << "xmlStanza error: " << e << endl;
		}

		//generate our cnonce
		srand((unsigned)time(0)); 
		int rint = rand()%25;
		string cnonce;
		for(int q=0;q<25;q++){
			cnonce += ('a'+rint);
			rint = rand()%25;
		}
		cnonce = hex((unsigned char *)cnonce.c_str(), cnonce.length());
		cnonce = cnonce.substr(0,48); //RFC suggests 64 bits of entropy, so we're way better than that

		string nonce = parseDigest(chal, "nonce");
		string authzid = parseDigest(chal, "authzid");
		string realm = parseDigest(chal, "realm");
		string qop = parseDigest(chal, "qop");

		//cout << parseDigest(chal, "algorithm") << ", " << parseDigest(chal, "algorithm").length() << endl;

		string A1 = genA1hex(nonce, cnonce, authzid, realm);
		string A2 = genA2hex(qop);
		string response = genResponseHex(nonce, cnonce, qop, A1, A2);

		string resp = "charset=utf-8,username=\""+username+"\"";
		if(realm != "") resp += ",realm=\""+realm+"\"";
		resp += ",nonce=\""+nonce+"\",nc=00000001,cnonce=\""+cnonce+"\",digest-uri=\"xmpp/"+server+"\",response="+response+",qop="+qop;

		con << xmlStanza::xmlSASLresponse(base64encode(resp));
		con >> tmp;
		
		//expecting another challenge or failure
		xmlPar.parse(tmp);
		if(xmlPar.getType() == CHALLENGE){
			chal = xmlPar.getContent();
			chal = base64decode(chal);
			A2 = genA2Shex(qop);
			//cout << "\n'" << response.length() << "'\n'" << resp.length() << "'" << endl;
			if(parseDigest(chal, "rspauth") != genResponseHex(nonce, cnonce, qop, A1, A2)){
				//uh-oh, what the ...
				debugPrint("SASL authentication failed: wrong rspauth");
				return false;
			}
			con << xmlStanza::xmlSASLresponse("");
			con >> tmp;
			xmlPar.parse(tmp);
			if(xmlPar.getType() != SASL_SUCCESS){
				debugPrint("SASL authentication failed after second response");
				return false;
			}
			return true;
		}
		else if(xmlPar.getType() == SASL_FAILURE){
			cerr << "SASL authentication failed with error: "+xmlPar.getSASLError() << endl;
			return false;
		}
		else{
			debugPrint("SASL authentication failed, unexpected xml received:\t\n"+chal);
			return false;
		}
		
	}
	return true;
}

bool jabberConnection::doSASLPLAIN(void){
	string tmp;
	xmlParserWrap xmlPar;
	unsigned char *in, *out;
	int in_len, out_len;

	tmp = '\0' + username + '\0' + password;
	/*cout << tmp << endl;
	in_len = tmp.size() + 1;
	out_len = in_len * 2;
	in = (unsigned char *)malloc(tmp.size() * sizeof(unsigned char));
	strncpy((char *)in, tmp.c_str(), in_len);
	in[in_len] = 0;
	out = (unsigned char *)malloc(out_len * sizeof(unsigned char));
	fprintf(stdout, "iso: %s\n", in);
	if(isolat1ToUTF8(out, &out_len, in, &in_len) > 0){
		out[out_len] = 0;
		fprintf(stdout, "utf8: %s\nin_len: %i, out_len: %i\n", out, in_len, out_len);
	}
	else
		cerr << "error converting to utf8\n";
	
	tmp = (char *)out;
	cout << "tmp: " << tmp << endl;
	exit(1);*/
	con << xmlStanza::xmlSASLauthPLAIN(base64encode(tmp));
	try{
		con >> tmp;
	}
	catch(string e){
		cerr << e << endl;
		exit(1);
	}
	xmlPar.parse(tmp);
	if(xmlPar.getType() != SASL_SUCCESS){
		debugPrint("SASL PLAIN authentication failed. Wrong password?");
		return false;
	}
	return true;
}

/******************** public ************************/

jabberConnection::jabberConnection(string newjid, string pass, string srvr, int prt, bool tls){
	int atpos, slashpos;
	string host;


	stat = OFFLINE;
	stav = S_OFF;
	atpos = newjid.find_last_of('@');
	slashpos = newjid.find_first_of('/');
	username = newjid.substr(0, atpos);
	useTls = tls;

	if(slashpos != string::npos){
		server = newjid.substr(atpos+1, slashpos - atpos-1);
		resource = newjid.substr(slashpos+1);
	}
	else
		server = newjid.substr(atpos+1);

/*	if(srvr == "no server")
		host = server;
	else*/
	if(srvr == "talk.google.com"){
		server = newjid.substr(newjid.find_first_of('@') + 1, newjid.find_last_of('@') - newjid.find_first_of('@') - 1);
	}

	password = pass;
	con.setHost(srvr);
//	con.setHost(host);
	con.setPort(prt);
#ifdef _DEBUG_
		cout << "parsed JID:\n\tusername:\t" << username << endl;
		cout << "\tpassword:\t" << password << endl;
		cout << "\tserver:\t\t" << server << endl;
		cout << "\tresource:\t" << resource << endl << endl;
#endif
	thr = NULL;
}

jabberConnection::~jabberConnection(void){
	if(thr)
		delete thr;

	logout(NONE);
}

void jabberConnection::setJID(string newjid){
	//jid = newjid;
}

void jabberConnection::setPassword(string newpass){
	password = newpass;
}

/*
 * returns number indicating result of registration:
 * 	-1 - error not connected with registration proccess (e.g. could not connect)
 * 	0 - registration successful
 * 	1 - conflict - username already registered
 * 	2 - not-acceptable - not all mandatory values were filled, IM _should_ require only username and password
 * 	3 - service-unavailable - registration is not supported by this server
 */
int jabberConnection::registerNewAccount(void){
	string tmp, id;
	xmlParserWrap xmlPar;

	if(!con.cnct()){
		return -1;
	}
	//send the opening stream tag
	//con << xmlStanza::xmlStream(con.getHost());
	con << xmlStanza::xmlStream(server);
	con >> tmp;  //wait for answer
	//some servers don't advertise their ability to register new accounts eventhough they support it => dont parse answer and assume register is supported
	con << xmlStanza::xmlIqRegister(username, password, id);
	con >> tmp;  //wait for answer
	xmlPar.parse(tmp);
	//if we didnt get the answer to our iq, wait for it..
	while(xmlPar.getArg("id") != id){
		sleep(1);
		con >> tmp;  //wait for answer
		xmlPar.parse(tmp);
	}
	if(xmlPar.hasSubTag("error")){
		if(xmlPar.getErrorString() == "bad-request")
			return -1;
		else if(xmlPar.getErrorString() == "conflict")
			return 1;
		else if(xmlPar.getErrorString() == "not-acceptable")
			return 2;
		else if(xmlPar.getErrorString() == "service-unavailable")
			return 3;
		else
			return -1;
	}
	else{
		return 0;
	}
}

int jabberConnection::changePassword(void){
	string id;
	rcvStanza_ st;
	xmlParserWrap xmlPar;
	string jid;

	jid = username+"@"+server+"/"+resource;
	con << xmlStanza::xmlIqChangePassword(jid, username, password, id);
	while(!thr->getStanzaById(atoi(id.c_str()), st)) sleep(1);

	if(!xmlPar.parse(st.data))
		return -1;

	if(xmlPar.hasSubTag("error")){
		if(xmlPar.getErrorString() == "bad-request")
			return -1;
		else if(xmlPar.getErrorString() == "conflict")
			return 1;
		else if(xmlPar.getErrorString() == "not-acceptable")
			return 2;
		else if(xmlPar.getErrorString() == "service-unavailable")
			return 3;
		else
			return -1;
	}
	else{
		return 0;
	}
}

bool jabberConnection::login(void){
	string tmp, id;
	int a=0;
	xmlParserWrap xmlPar;

	//connect to server
	if(!con.cnct()){
		cerr << "unable to connect to " << con.getHost() << ": " << con.getError() << endl;
		return false;
	}
	//send the opening stream tag
	con << xmlStanza::xmlStream(server);
	con >> tmp;  //wait for answer

	//now we parse the <stream:stream> tag, i want to use libxml, so i must add closing tag for <stream:stream>
	tmp += "</stream:stream>";
	if(!xmlPar.parse(tmp)){
		debugPrint("jabber login failed: "+xmlPar.getError());
		logout(NOT_WELL_FORMED);
		return false;
	}
	//we expect stream tag, so, anything else is wrong and we stop talking to server
	if(xmlPar.getType() != STREAM){
		debugPrint("jabber login failed: unexpected xml");
		logout(INVALID);
		return false;
	}
	id = xmlPar.getArg("id");
	
	//we are expecting features tag*/
	if(xmlPar.getType() != FEATURES){
		//perhaps we did not managed to catch it yet
		con >> tmp;
		if(!xmlPar.parse(tmp)){
			debugPrint("jabber login failed: "+xmlPar.getError());
			logout(INVALID);
			return false;
		}
		if(xmlPar.getType() != FEATURES){
			debugPrint("jabber login failed: unexpected xml (features expected)");
			logout(INVALID);
			return false;
		}
	}
	if(useTls){
		//is TLS supported?
		if(xmlPar.supportsTLS()){
			//try to negotiate TLS 
			con << xmlStanza::xmlStanzaStartTLS();
			con >> tmp;
			if(!xmlPar.parse(tmp)){
				debugPrint("jabber login failed: "+xmlPar.getError());
				logout(NOT_WELL_FORMED);
				return false;
			}
			if(xmlPar.getType() != PROCEED){
				debugPrint("server announced TLS, but refuses to use it, will try non-TLS connection");
				logout(INVALID);
				return false;
			}
			if(!doSSLHandshake()){
				debugPrint("ssl handshake failed");
				return false;
			}
			//so we have successfully established TLS, lets start new stream
			con << xmlStanza::xmlStream(server);
			con >> tmp; //stream response
			xmlPar.parse(tmp);
			
			con >> tmp; //features
			if(!xmlPar.parse(tmp)){
				debugPrint("jabber login failed: "+xmlPar.getError());
				logout(NOT_WELL_FORMED);
				return false;
			}
			//we are expecting features tag
			if(xmlPar.getType() != FEATURES){
				debugPrint("jabber login failed: unexpected xml (features expected)");
				logout(INVALID);
				return false;
			}
		}
		stat = TLS;
	}

	//SASL authentication
	if(xmlPar.supportsMD5()){  //other then MD5 authentifications
		if(!doSASLMD5()){
			debugPrint("SASL authentication failed!");
			return false;
		}
	}
	else if(xmlPar.supportsPLAIN()){
		if(!doSASLPLAIN()){
			debugPrint("SASL authentication failed!");
			return false;
		}
	}
	//authentication successful, let's start new stream (this is the last time, i promise)
	con << xmlStanza::xmlStream(server);
	con >> tmp;
	tmp += "</stream:stream>"; //just for libxml
	xmlPar.parse(tmp);
	if(xmlPar.getType() != STREAM){
		debugPrint("Error opening new stream after SASL!");
		return false;
	}
	if(xmlPar.getType() != FEATURES){
		//we probably didn't cought it with stream
		con >> tmp;
		xmlPar.parse(tmp);
		if(xmlPar.getType() != FEATURES){
			debugPrint("<features> tag expected, but not received");
			return false;
		}
		if(xmlPar.supportsBind()){
			con << xmlStanza::xmlIqBindResource(resource);
			con >> tmp;
			con << xmlStanza::xmlIqSession();
			con >> tmp;
		}
		//setStatus(S_ON);
		stat = ONLINE;   //huray!, we're online
		//oncoming socket communication will be asynchronous, controled by separate thread
		thr = new readThread(&con);
		return true;
	}

	return false;
}

bool jabberConnection::setStatus(enumStav st){
	string tmp;

	switch(st){
	case S_OFF:
		logout(NONE);
		stav = S_OFF;
		break;
	case S_AWAY:
		con << xmlStanza::xmlPresence(5, "away");
		stav = S_AWAY;
		break;
	case S_CHAT:
		con << xmlStanza::xmlPresence(5, "chat");
		stav = S_CHAT;
		break;
	case S_DND:
		con << xmlStanza::xmlPresence(5, "dnd");
		stav = S_DND;
		break;
	case S_NA:
		con << xmlStanza::xmlPresence(5, "xa");
		stav = S_NA;
		break;
	case S_ON:
		if(stat != ONLINE)
			if(!login())
				return false;

		con << xmlStanza::xmlPresence(5);
		stav = S_ON;
		break;
	}
	return true;
}

bool jabberConnection::logout(enumError er){
	if(stat != OFFLINE){
		switch(er){
		case INVALID:
			//send invalid-xml tag to server
			break;
		case NOT_WELL_FORMED:
			//send xml-not-well-formed tag to server
			break;
		}
		con << "</stream:stream>";
		con.disconnect();
		stat = OFFLINE;
	}
	return true;
}

const jabberConnection& jabberConnection::operator<<(const string& txt){
	con << txt;
	return *this;
}

/*bool jabberConnection::getStanzaById(int id, rcvStanza_ &ret){
	rcvStanzaIter iter;

	for(iter = inStanza.begin();iter != inStanza.end();iter++){
		if((*iter).id == xmlStanza::itoa(id)){
			ret = *iter;
			return true;
		}
	}

	return false;
}*/

