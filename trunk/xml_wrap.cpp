#ifdef MHASH
#include <mhash.h>
#endif
#include "xml_wrap.h"
#include <string>

int sID=0;

#ifdef MHASH
/* authenticate without SASL (XEP-0078) DEPRECATED!!! */
string xmlStanza::xmlStanzaAuth(string username, string password, string resource, string hashid, string id){  
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *mem;
	int size;
	string ret;

	doc = xmlNewDoc((xmlChar *)"1.0");
	//node = xmlNewDocNode(doc, NULL, (xmlChar *)"iq", NULL);
	node = xmlNewNode(NULL, (xmlChar *)"iq");
	xmlDocSetRootElement(doc, node);
	if(password == "")
		xmlNewProp(node, (xmlChar *)"type", (xmlChar *)"get");
	else
		xmlNewProp(node, (xmlChar *)"type", (xmlChar *)"set");
	xmlNewProp(node, (xmlChar *)"id", (xmlChar *)id.c_str());
	node = xmlNewChild(node, NULL, (xmlChar *)"query", NULL);
	xmlNewProp(node, (xmlChar *)"xmlns", (xmlChar *)"jabber:iq:auth");
	if(username != "")
		xmlNewChild(node, NULL, (xmlChar *)"username", (xmlChar *)username.c_str());
	if(resource != "")
		xmlNewChild(node, NULL, (xmlChar *)"resource", (xmlChar *)resource.c_str());
	if(hashid != ""){
		hashid += password;
		hashid = sha(hashid);
		xmlNewChild(node, NULL, (xmlChar *)"digest", (xmlChar *)hashid.c_str());
	//	xmlNewChild(node, NULL, (xmlChar *)"password", (xmlChar *)password.c_str());
	}
	xmlDocDumpMemory(doc, &mem, &size);
	ret = (char *)mem;
	ret = ret.substr(22);
	return ret;
}

#endif

string xmlStanza::xmlStream(string to){
	string ret;

	ret = "<?xml version='1.0'?><stream:stream xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:client' to='"+to+"' version='1.0'>";

	return ret;
}

string xmlStanza::xmlSASLauth(string method){
	string ret;
	/*xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *mem;
	int size;

	doc = xmlNewDoc((xmlChar *)"1.0");
	node = xmlNewNode(NULL, (xmlChar *)"auth");
	xmlDocSetRootElement(doc, node);
	xmlNewProp(node, (xmlChar *)"xmlns", (xmlChar *)"urn:ietf:params:xml:ns:xmpp-sasl");
	xmlNewProp(node, (xmlChar *)"mechanism", (xmlChar *)method.c_str());
	xmlDocDumpMemory(doc, &mem, &size);
	ret = (char *)mem;
	size = ret.find_first_of(">");
	ret = ret.substr(size+2);*/

	ret = "<auth xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\" mechanism=\"" + method + "\"/>";

	return ret;
}

string xmlStanza::xmlSASLresponse(string text){
	string ret;
	/*xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *mem;
	int size;

	doc = xmlNewDoc((xmlChar *)"1.0");
	node = xmlNewNode(NULL, (xmlChar *)"response");
	xmlDocSetRootElement(doc, node);
	xmlNewProp(node, (xmlChar *)"xmlns", (xmlChar *)"urn:ietf:params:xml:ns:xmpp-sasl");
	xmlNodeAddContent(node, (xmlChar *)text.c_str());
	xmlDocDumpMemory(doc, &mem, &size);
	ret = (char *)mem;
	size = ret.find_first_of(">");
	ret = ret.substr(size+2);*/

	ret = "<response xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\"";
	if(text != "")
		ret += ">" + text + "</response>";
	else
		ret += "/>";

	return ret;
}

string xmlStanza::xmlStanzaStartTLS(void){
	string ret;
	/*xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *mem;
	int size;

	doc = xmlNewDoc((xmlChar *)"1.0");
	node = xmlNewNode(NULL, (xmlChar *)"starttls");
	xmlDocSetRootElement(doc, node);
	xmlNewProp(node, (xmlChar *)"xmlns", (xmlChar *)"urn:ietf:params:xml:ns:xmpp-tls");
	xmlDocDumpMemory(doc, &mem, &size);
	ret = (char *)mem;
	size = ret.find_first_of(">");
	ret = ret.substr(size+2);*/

	ret = "<starttls xmlns=\"urn:ietf:params:xml:ns:xmpp-tls\"/>";

	return ret;
}

string xmlStanza::itoa(int cislo){
	string buf = "00000";
	int i=0;

	buf[i] = cislo/10000 + '0';if(buf[i] != '0') i++;cislo = cislo-10000*(cislo/10000);
	buf[i] = cislo/1000 + '0';if(buf[i] != '0') i++;cislo = cislo-1000*(cislo/1000);
	buf[i] = cislo/100 + '0';if(buf[i] != '0') i++;cislo = cislo-100*(cislo/100);
	buf[i] = cislo/10 + '0';if(buf[i] != '0') i++;cislo = cislo-10*(cislo/10);
	buf[i] = cislo + '0';if(buf[i] != '0') i++;

	buf[i] = '\0';

	return buf.substr(0,i);;
}

string xmlStanza::xmlIqRegister(string username, string password, string &id){
	string ret;

	id = itoa(++sID);
	ret = "<iq type='set' id='" + id + "'><query xmlns='jabber:iq:register'><username>" + username + "</username><password>" + password + "</password></query></iq>";

	return ret;
}

string xmlStanza::xmlIqChangePassword(string jid, string username, string password, string &id){
	string ret;

	id = itoa(++sID);
	ret = "<iq type='set' from='" + jid + "' id='" + id + "'><query xmlns='jabber:iq:register'><username>" + username + "</username><password>" + password + "</password></query></iq>";

	return ret;
}

/*string xmlStanza::xmlIqRegisterQuery(void){
	string ret;

	ret = "<iq type='get' id='" + itoa(++sID) + "'><query xmlns='jabber:iq:register'/><username>" + username + "</username><password>" + password + "</password></iq>";

	return ret;
}*/

string xmlStanza::xmlIqBindResource(string res){
	string ret;
	/*xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *mem;
	int size;

	doc = xmlNewDoc((xmlChar *)"1.0");
	node = xmlNewNode(NULL, (xmlChar *)"iq");
	xmlDocSetRootElement(doc, node);
	xmlNewProp(node, (xmlChar *)"type", (xmlChar *)"set");
	xmlNewProp(node, (xmlChar *)"id", (xmlChar *)xmlStanza::itoa(++sID).c_str());
	node = xmlNewChild(node, NULL, (xmlChar *)"bind", NULL);
	xmlNewProp(node, (xmlChar *)"xmlns", (xmlChar *)"urn:ietf:params:xml:ns:xmpp-bind");
	if(res != ""){
		node = xmlNewChild(node, NULL, (xmlChar *)"resource", (xmlChar *)res.c_str());
	}

	xmlDocDumpMemory(doc, &mem, &size);
	ret = (char *)mem;
	size = ret.find_first_of(">");
	ret = ret.substr(size+2);*/

	ret = "<iq type=\"set\" id=\"1\"><bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\"><resource>" + res + "</resource></bind></iq>";

	return ret;
}

string xmlStanza::xmlIqSession(void){
	string ret;
	/*xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *mem;
	int size;

	doc = xmlNewDoc((xmlChar *)"1.0");
	node = xmlNewNode(NULL, (xmlChar *)"iq");
	xmlDocSetRootElement(doc, node);
	xmlNewProp(node, (xmlChar *)"type", (xmlChar *)"set");
	xmlNewProp(node, (xmlChar *)"id", (xmlChar *)xmlStanza::itoa(++sID).c_str());
	node = xmlNewChild(node, NULL, (xmlChar *)"session", NULL);
	xmlNewProp(node, (xmlChar *)"xmlns", (xmlChar *)"urn:ietf:params:xml:ns:xmpp-session");

	xmlDocDumpMemory(doc, &mem, &size);
	ret = (char *)mem;
	size = ret.find_first_of(">");
	ret = ret.substr(size+2);*/

	ret = "<iq type=\"set\" id=\"" + xmlStanza::itoa(++sID) + "\"><session xmlns=\"urn:ietf:params:xml:ns:xmpp-session\"/></iq>";

	return ret;
}

string xmlStanza::xmlPresence(int prio, string show){
	string ret;
	/*xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *mem;
	int size;
	string pr;

	pr = xmlStanza::itoa(prio);

	doc = xmlNewDoc((xmlChar *)"1.0");
	node = xmlNewNode(NULL, (xmlChar *)"presence");
	xmlDocSetRootElement(doc, node);
	xmlNewProp(node, (xmlChar *)"id", (xmlChar *)xmlStanza::itoa(++sID).c_str());
	node = xmlNewChild(node, NULL, (xmlChar *)"priority", (xmlChar *)pr.c_str());
	if(show != ""){
		node = xmlNewChild(xmlDocGetRootElement(doc), NULL, (xmlChar *)"show", (xmlChar *)show.c_str());
	}

	xmlDocDumpMemory(doc, &mem, &size);
	ret = (char *)mem;
	size = ret.find_first_of(">");
	ret = ret.substr(size+2);*/

	ret = "<presence id=\"" + xmlStanza::itoa(++sID) + "\"><priority>" + itoa(prio) + "</priority>";
	if(show != "")
		ret += "<show>" + show + "</show>";
	ret += "</presence>";

	return ret;
}

string xmlStanza::xmlPresenceSubscription(string to, string type){
	string ret;

	ret = "<presence to='"+to+"' type='"+type+"'/>";

	return ret;
}

/*string xmlStanza::xmlQuery(string from, string ns, int &id){
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *mem;
	int size;
	string ret, pr;

	doc = xmlNewDoc((xmlChar *)"1.0");
	node = xmlNewNode(NULL, (xmlChar *)"iq");
	xmlDocSetRootElement(doc, node);
	xmlNewProp(node, (xmlChar *)"id", (xmlChar *)xmlStanza::itoa(++sID).c_str());
	id = sID;

	xmlNewProp(node, (xmlChar *)"from", (xmlChar *)from.c_str());
	xmlNewProp(node, (xmlChar *)"type", (xmlChar *)"get");

	node = xmlNewChild(node, NULL, (xmlChar *)"query", NULL);
	xmlNewProp(node, (xmlChar *)"xmlns", (xmlChar *)ns.c_str());

	xmlDocDumpMemory(doc, &mem, &size);
	ret = (char *)mem;
	size = ret.find_first_of(">");
	ret = ret.substr(size+2);
	return ret;
}*/

string xmlStanza::xmlRosterQuery(string from, int &id){
	string ret;

	//ret="<iq id=\""+xmlStanza::itoa(++sID)+"\" from=\""+from+"\" type=\"get\"><query xmlns=\"jabber:iq:roster\"/></iq>";
	ret="<iq id=\""+xmlStanza::itoa(++sID)+"\" type=\"get\"><query xmlns=\"jabber:iq:roster\"/></iq>";
	id = sID;

	return ret;
}

string xmlStanza::xmlRosterAdd(string from, string jid, string name, string group, int &id){
	string ret;

	ret="<iq id=\""+xmlStanza::itoa(++sID)+"\" from=\""+from+"\" type=\"set\"><query xmlns=\"jabber:iq:roster\"><item jid=\""+jid+"\" ";
	if(name != "")
		ret += "name=\""+name+"\">";
	else
		ret += "name=\""+jid.substr(0, jid.find_first_of('@'))+"\">";
	if(group != "")
		ret += "<group>"+group+"</group>";
	ret += "</item></query></iq>";
	id = sID;

	return ret;
}

string xmlStanza::xmlRosterRemove(string from, string jid, int &id){
	string ret;

	ret = "<iq from='"+from+"' type='set' id='"+xmlStanza::itoa(++sID)+"'><query xmlns='jabber:iq:roster'><item jid='"+jid+"' subscription='remove'/></query></iq>";

	id = sID;

	return ret;
}

string xmlStanza::xmlMessage(string to, string text, int &id){
	string ret;

	ret = "<message to=\""+to+"\" type=\"chat\" id=\""+xmlStanza::itoa(++sID)+"\"><body>"+text+"</body></message>";
	id = sID;

	return ret;
}

#ifdef MHASH

string xmlStanza::sha(string co){
	string ret;
	MHASH td;
	unsigned char *buf;
	char tmp[3]="\0\0";

	td = mhash_init(MHASH_SHA1);
	if (td == MHASH_FAILED) return "";
	buf = (unsigned char *)malloc(co.size()*sizeof(char));
	strncpy((char *)buf, co.c_str(), co.size());
	//mhash(td, buf, co.size());
	for(int a=0;a<co.size();a++)
		mhash(td, &buf[a], 1);
	mhash_deinit(td, buf);
	ret = "";
	for(int a=0;a<mhash_get_block_size(MHASH_SHA1);a++){ 
		sprintf(tmp, "%.2x", buf[a]);
		ret += tmp;
	}
	return ret;
}

#endif

/***************** xmlParserWrap ***************/
xmlParserWrap::xmlParserWrap(void){
	doc = NULL;
}
xmlParserWrap::xmlParserWrap(string mem){
	doc = NULL;
	parse(mem);
}
xmlParserWrap::~xmlParserWrap(void){
	xmlFreeDoc(doc);
}

xmlNodePtr xmlParserWrap::getRoot(void){
	xmlNodePtr n;

	if(doc == NULL)
		return NULL;
	
	n = xmlDocGetRootElement(doc);
	n = n->xmlChildrenNode;

	return n;
}

bool xmlParserWrap::parse(string data = ""){
	xmlNodePtr root;
	xmlChar *tmp;

	if(data != ""){
		//delete <?xml ?> tag
		if(data.substr(0, 2) == "<?")
			data = data.substr(data.find_first_of(">") + 1);

		//add wrapper tag (if it isn't there already) for case when there is more than one "root" tag
		if(data.substr(0, 9) != "<wrapper>"){
			data.insert(0, "<wrapper>");
			data.insert(data.length(), "</wrapper>");
		}
		
		buffer = data;

		if(doc != NULL) xmlFreeDoc(doc);

		doc = xmlReadMemory(data.c_str(), data.length(), "noname.xml", NULL, XML_PARSE_RECOVER | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
		if(doc == NULL){
			error = "unable to parse xml memory chunk";
			return false;
		}
	}
	
	//finds out what tag we actualy have here and sets type accordingly
	root = getRoot();
	if(root == NULL)
		return false;

	if(!xmlStrcmp(root->name, (const xmlChar *)"stream"))
		type = STREAM;
	else if(!xmlStrcmp(root->name, (const xmlChar *)"features"))
		type = FEATURES;
	else if(!xmlStrcmp(root->name, (const xmlChar *)"proceed"))
		type = PROCEED;
	else if(!xmlStrcmp(root->name, (const xmlChar *)"challenge"))
		type = CHALLENGE;
	else if(!xmlStrcmp(root->name, (const xmlChar *)"failure") && !xmlStrcmp(root->nsDef->href, (xmlChar *)"urn:ietf:params:xml:ns:xmpp-sasl"))
		type = SASL_FAILURE;
	else if(!xmlStrcmp(root->name, (const xmlChar *)"success") && !xmlStrcmp(root->nsDef->href, (xmlChar *)"urn:ietf:params:xml:ns:xmpp-sasl"))
		type = SASL_SUCCESS;
	else if(!xmlStrcmp(root->name, (const xmlChar *)"message")){
		if(!xmlStrcmp((tmp = xmlGetProp(root, (const xmlChar *)"type")), (const xmlChar *)"error"))
			type = ERROR;
		else
			type = MESSAGE;
		xmlFree(tmp);
	}
	else if(!xmlStrcmp(root->name, (const xmlChar *)"presence")){
		if(!xmlStrcmp(xmlGetProp(root, (const xmlChar *)"type"), (const xmlChar *)"subscribe"))
			type = SUBSCRIBE;
		else
			type = PRESENCE;
	}
	else if(!xmlStrcmp(root->name, (const xmlChar *)"iq")){
		type = IQ;
		root = root->xmlChildrenNode;

		if((root != NULL) && (root->nsDef != NULL) && (!xmlStrcmp(root->nsDef->href, (xmlChar *)"jabber:iq:roster"))){
			type = ROSTER_IQ;
		}
		node = NULL;
	}
	else
		type = UNKNOWN;
	
	return true;
}

bool xmlParserWrap::hasSubTag(string name){
	xmlNodePtr nod;

	nod = getRoot()->xmlChildrenNode;
	while(nod != NULL){
		if(!xmlStrcmp(nod->name, (const xmlChar *)name.c_str()))
			return true;
		else
			nod = nod->next;
	}
	return false;
}

bool xmlParserWrap::supportsTLS(void){
	xmlNodePtr nod;

	if(type == FEATURES){
		nod = getRoot()->xmlChildrenNode;
		if(!xmlStrcmp(nod->name, (const xmlChar *)"features"))
			nod = nod->xmlChildrenNode;
		while(nod != NULL){
			if(!xmlStrcmp(nod->name, (const xmlChar *)"starttls"))
				return true;
			else
				nod = nod->next;
		}
	}
	return false;
}

bool xmlParserWrap::supportsMD5(void){
	xmlNodePtr nod;
	xmlChar *tmp;

	if(type == FEATURES){
		nod = getRoot()->xmlChildrenNode;
		if(!xmlStrcmp(nod->name, (const xmlChar *)"features"))
			nod = nod->xmlChildrenNode;
		while(nod != NULL){
			if(!xmlStrcmp(nod->name, (const xmlChar *)"mechanisms"))
				break;
			else
				nod = nod->next;
		}
		nod = nod->xmlChildrenNode;
		while(nod != NULL){
			if(!xmlStrcmp(nod->name, (const xmlChar *)"mechanism") && !xmlStrcmp((const xmlChar *)"DIGEST-MD5", (tmp = xmlNodeListGetString(doc, nod->xmlChildrenNode, 1)))){
				xmlFree(tmp);
				return true;
			}
			else
				nod = nod->next;

			xmlFree(tmp);
		}
	}
	return false;
}

bool xmlParserWrap::supportsBind(void){
	xmlNodePtr nod;

	if(type == FEATURES){
		nod = getRoot()->xmlChildrenNode;
		while(nod != NULL){
			if(!xmlStrcmp(nod->name, (const xmlChar *)"bind"))
				return true;
			else
				nod = nod->next;
		}
	}
	return false;
}

string xmlParserWrap::getArg(string name){
	string buf;
	xmlChar *a;
	
	//if(type == STREAM){
		a = xmlGetProp(getRoot(), (xmlChar *)(name.c_str()));
		if(a != NULL)
			buf = (char *)a;
		else
			buf = "";

		xmlFree(a);
		return buf;
	/*}
	else
		return "";*/
}

string xmlParserWrap::getContent(void){
	xmlChar *buf;
	string ret;

	if(doc != NULL && type == CHALLENGE){
		buf = xmlNodeGetContent(getRoot());
		ret = (char *)buf;
		xmlFree(buf);
		return ret;
	}
	else if(doc != NULL && (type == MESSAGE || type == IQ)){
		buf = xmlNodeGetContent(getRoot()->xmlChildrenNode);
	}
	else
		throw "No document to parse!";
}

string xmlParserWrap::getSASLError(void){
	string ret;

	if(type == SASL_FAILURE){
		ret = (char *)(getRoot())->xmlChildrenNode->name;
	}
	else{
		ret = "this is not a sasl failure xml, somewhere something went terribly wrong, send a bugreport would you?";
	}

	return ret;
}

stanzaType xmlParserWrap::getType(void){
	xmlNodePtr nod;

	nod = getRoot()->xmlChildrenNode;
	//what we got may be <stream> and <features> or only <stream> or anything else
	if(type == STREAM && nod != NULL && !xmlStrcmp(nod->name, (const xmlChar *)"features")){
		type = FEATURES;
		return STREAM;
	}
	else return type;
		
}

string xmlParserWrap::getRosterAtr(string name){
	string ret;
	xmlChar *buf;

	if(type != ROSTER_IQ){
		return "";
	}
	if(node == NULL){ //dig in to first roster item
		node = getRoot(); //iq
		node = node->xmlChildrenNode; //query
		node = node->xmlChildrenNode; //first item
	}
	buf = xmlGetProp(node, (xmlChar *)name.c_str());
	if(buf != NULL)
		ret = (char *)buf;
	else
		ret = "";
	
	xmlFree(buf);
	return ret;
}

string xmlParserWrap::getRosterGroup(void){
	string ret;
	xmlChar *buf;
	xmlNodePtr tmp;

	if(type != ROSTER_IQ){
		return "";
	}
	if(node == NULL){ //dig in to first roster item
		node = getRoot(); //iq
		node = node->xmlChildrenNode; //query
		node = node->xmlChildrenNode; //first item
	}
	tmp = node->xmlChildrenNode; //has it group child?
	if(tmp == NULL)
		return "unknown";
	else{
		if((!xmlStrcmp(tmp->name, (xmlChar *)"group"))){
			buf = xmlNodeGetContent(tmp);
			ret = (char *)buf;
			free(buf); 
			return ret;
		}
	}
	return "unknown";
}

bool xmlParserWrap::nextRosterItem(void){
	if(node == NULL)
		return false;

	node = node->next;

	if(node == NULL)
		return false;

	return true;
}

string xmlParserWrap::getPresenceShow(void){
	string ret;
	xmlChar *buf;
	xmlNodePtr n, root;

	if(type != PRESENCE)
		return "";

	root = getRoot();
	n = getRoot()->xmlChildrenNode;
	while(n != NULL && xmlStrcmp(n->name, (xmlChar *)"show"))
		n = n->next;
	
	if(n == NULL || xmlStrcmp(n->name, (xmlChar *)"show")){
		if(!xmlStrcmp(xmlGetProp(root, (const xmlChar *)"type"), (const xmlChar *)"unavailable"))
			return "offline";
		else
			return "online";
	}
	else{
		buf = xmlNodeGetContent(n);
		ret = (char *)buf;
		free(buf);
		return ret;
	}
}

string xmlParserWrap::getMessageBody(void){
	string ret;
	xmlChar *buf;
	xmlNodePtr n;

	if(type != MESSAGE)
		return "";

	n = getRoot()->xmlChildrenNode;
	while(n != NULL && xmlStrcmp(n->name, (xmlChar *)"body"))
		n = n->next;

	if(n == NULL || xmlStrcmp(n->name, (xmlChar *)"body"))
		return "";
	else{
		buf = xmlNodeGetContent(n);
		ret = (char *)buf;
		free(buf);
		return ret;
	}
}

bool xmlParserWrap::next(void){
	xmlNodePtr root;
	xmlChar *mem;
	int size;

	root = getRoot();
	if(root != NULL && root->next != NULL){
		xmlUnlinkNode(root);
		xmlDocDumpMemory(doc, &mem, &size);
		buffer = (char *)mem;
		buffer = buffer.substr(buffer.find_first_of(">") + 2);
		//cout << "buffer: " << buffer << endl;
		return parse();
	}
	else
		return false;
}

string xmlParserWrap::getErrorString(void){
	xmlNodePtr node;
	xmlChar *mem;
	string ret;
	int size;

	node = getRoot()->xmlChildrenNode;
	if(node != NULL && xmlStrcmp(node->name, (xmlChar *)"error"))
		node = node->next; // get to the error subtag

	if(node == NULL || xmlStrcmp(node->name, (xmlChar *)"error"))
		return "";

	node = node->xmlChildrenNode;
	if(node != NULL){
		ret = (char *)node->name;
		return ret;
	}	
}

