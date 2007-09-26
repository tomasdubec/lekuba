#include "connection.h"
#include <string>
#include <ctime>
#include <string.h>
#include <openssl/md5.h>
#include <err.h>

konexe::konexe(string hst, int prt, bool tls){
	hostname = hst;
	port = prt;
	useTls = tls;

	if(useTls)
		sslInit();
	
#ifdef LOGFILE
	vystup.open(LOGFILE);
#endif
}
konexe::konexe(void){
	hostname = "";
	port = 0;
	useTls = true;
	sslInit();
#ifdef LOGFILE
	vystup.open(LOGFILE);
#endif
}
konexe::~konexe(void){
	konexe::disconnect();
#ifdef LOGFILE
	vystup.close();
#endif
}
bool konexe::sslInit(void){
	SSL_library_init();
	SSL_load_error_strings();
	ssl_ctx_TLSv1 = SSL_CTX_new(TLSv1_client_method());
        if (ssl_ctx_TLSv1 == NULL){
		cerr << "TLS not available" << endl;
		return false;
	}
	else{
#ifdef _DEBUG_
		cout << "*** SSL library initialized" << endl;
#endif
	}
}
void konexe::setHost(string hst){
	hostname = hst;
}
string konexe::getHost(void){
	return hostname;
}
void konexe::setPort(int prt){
	port = prt;
}
bool konexe::cnct(void){
	if((host = gethostbyname(hostname.c_str())) == NULL){
		error = "unknown host";
		return false;
	}
	if((soket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		error = "unable to create socket";
		return false;
	}
	serverSock.sin_family = AF_INET;
	serverSock.sin_port = htons(port);
	memcpy(&(serverSock.sin_addr), host->h_addr, host->h_length);
	if(connect(soket, (sockaddr *)&serverSock, sizeof(serverSock)) == -1){
		error = "unable to connect";
		return false;
	}
	status = ONLINE;
	if(useTls){
		ssl = SSL_new(ssl_ctx_TLSv1); //initialize new SSL structure for TLS
		if(ssl == NULL){
			cerr << "error creating SSL structure" << endl;
			return false;
		}
		if(SSL_set_fd(ssl, soket) == 0){
			cerr << "SSL_set_fd: error connecting SSL to socket" << endl;
			return false;
		}
#ifdef _DEBUG_
		else{
			cout << "connected SSL to socket" << endl;
		}
#endif //_DEBUG_
	}
	return true;
}
bool konexe::snd(string text){
	if(status != OFFLINE){
#ifdef _DEBUG_
		cout << "<<<\n" << text << endl << endl;
#endif //_DEBUG_

#ifdef LOGFILE
		vystup << "<<<\n" << text << endl;
#endif

		if(status == TLS)
			size = SSL_write(ssl, text.c_str(), text.size());
		else
			size = send(soket, text.c_str(), text.size(), 0);

		if(size == -1){
			error = "unable to send data";
			status = OFFLINE;
			return false;
		}

		return true;
	}
	else
		return false;
}
string konexe::rcv(void){
	string tmp;
	char buf[BUFSIZE + 1];
	int erro;

	if(status != OFFLINE){
		tmp = "";
		memset(buf, 0, BUFSIZE + 1);
		if(status == TLS)
			size = SSL_read(ssl, buf, BUFSIZE);
		else
			size = recv(soket, buf, BUFSIZE, 0);

		if(size == -1){
			tmp = "error receiving data from socket";
			cerr << SSL_get_error(ssl, size) << endl;
			err(2, NULL);
			status = OFFLINE;
			throw tmp;
		}
		else{
			tmp = buf;
#ifdef _DEBUG_
			cout << ">>>\n" << tmp << endl << endl;
#endif
#ifdef LOGFILE
			vystup << ">>>\n" << tmp << endl;
#endif

			return tmp;
		}
	}
	else{
		throw "not ocnnected";
	}
}

bool konexe::disconnect(void){
	if(status != OFFLINE){
		if(close(soket) == -1){
			error = "unable to close socket";
			return false;
		}
		//status = OFFLINE;
	}
	return true;
}

string konexe::getError(void){
	return error;
}

bool konexe::doSSLHandshake(void){
	if(SSL_connect(ssl) == -1)
		return false;
	else{
		status = TLS;
		return true;
	}
}

const konexe& konexe::operator<<(const string& txt){
	if(!snd(txt)) throw "unable to send data via socket";

	return *this;
}
const konexe& konexe::operator>>(string& txt){
	try{
		txt = rcv();
	}
	catch(string e){
		throw e;
	}
	return *this;
}
