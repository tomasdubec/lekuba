#ifndef __konexe_h__
#define __konexe_h__

#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <openssl/ssl.h>

#ifdef LOGFILE
#include <fstream>
#endif

#define BUFSIZE 5000

using namespace std;

enum enumStatus{
	OFFLINE,
	ONLINE,
	TLS
};

class konexe{
	hostent *host;
	sockaddr_in serverSock;
	int soket, port, size;
	string error, hostname;
	SSL_CTX *ssl_ctx_TLSv1;
	SSL *ssl;
	enumStatus status;
	bool useTls;
	bool sslInit(void);
#ifdef LOGFILE
	ofstream vystup;
#endif
public:
	konexe(string hst, int prt, bool tls=false); //hostname, port for tcp connection
	konexe(void); //blank constructor
	~konexe();
	void setHost(string hst); //sets hostname, should end any currently active connection
	string getHost(void);
	void setPort(int prt); //sets port, should end any currently active connection
	bool cnct(void);
	bool disconnect(void);
	bool snd(string text);
	string rcv(void);
	string getError(void);
	bool getSSL(void){ if(ssl != NULL) return true; else return false;}
	bool doSSLHandshake(void);
	enumStatus getStatus(void){return status;};
	const konexe& operator>>(string& txt);
	const konexe& operator<<(const string& txt);
};
#endif
