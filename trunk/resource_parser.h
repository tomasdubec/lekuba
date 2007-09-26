#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;

typedef enum {kwUserName, kwPassword, kwResource, kwServer, kwTls, kwPort, EQUALS, KONEC, CISLO, OTHER} LexSymb;
const struct {string slovo; LexSymb symb;} keyWords[] = {
	{"username", kwUserName},
	{"password", kwPassword},
	{"resource", kwResource},
	{"server", kwServer},
	{"port", kwPort},
	{"tls", kwTls},
	{"konec", OTHER}
};

class Lexan{
	char znak, charType;
	ifstream input;

	void rdChar(void);
	LexSymb isKW(string);
public:
	LexSymb symbol;
	string slovo;

	Lexan(string);
	~Lexan();
	void rdInput(void);
};

class ConfigParser{
	Lexan *la;
	string user, pass, res, srv;
	int prt;
	bool usetls;
	
	void parse(void);
	void username(void);
	void password(void);
	void resource(void);
	void server(void);
	void port(void);
	void tls(void);
public:
	ConfigParser(string);
	~ConfigParser(void);
	string getUsername(void){return user;}
	string getPassword(void){return pass;}
	string getServer(void){return srv;}
	string getResource(void){return res;}
	int getPort(void){return prt;}
	bool getTls(void){return usetls;}
};

