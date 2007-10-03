#include <iostream>
#include <sys/prctl.h>
#include "roster.h"
#include "jabber_connection.h"
#include "tui.h"
#include "resource_parser.h"

#define VERSION "0.1.1"

using namespace std;

bool debug = true;

void reg(void){
	string username, server, password, resource, port, tls, configFile, tmp;
	ifstream in;
	ofstream out;
	
	configFile = getenv("HOME");
	configFile = configFile + "/.lekuba";
	in.open(configFile.c_str(), ifstream::in);
	if(in.is_open()){
		cout << "config file for lekuba already exists (" << configFile << ")\nDo you want me to replace it? [y/n] ";
		cin >> tmp;
		if(tmp == "y" || tmp == "Y"){
			in.close();
			remove(configFile.c_str());
		}
		else{
			in.close();
			exit(0);
		}
	}

	cout << "\t__,-register new account-,__\n";
	cout << "enter username for your account (e.g. franta): ";
	cin >> username;
	cout << "enter server name on which to create account (e.g. jabber.cz): ";
	cin >> server;
	cout << "enter server port number (default is 5222): ";
	cin >> port;
	if(port == "")
		port = "5222";

	cout << "do you want to use TLS? [y/n] ";
	cin >> tls;
	if(tls == "Y" || tls == "y")
		tls = "yes";
	else
		tls = "no";

	cout << "enter resource to use with lekuba (e.g. home or work): ";
	cin >> resource;
	cout << "enter password for " << username << "@" << server << " (will show as you type): ";
	cin >> password;

	//zmenit defaultni true u tls, ackoli tady je to asi fuk
	jabberConnection jcon(username + "@" + server + "/" + resource, password, server, atoi(port.c_str()), true);
	switch(jcon.registerNewAccount()){
	case 0:
		cout << "\nregistration successful!\n";
		out.open(configFile.c_str());
		if(!out.is_open()){
			cout << "cannot open config file " << configFile << endl;
			exit(1);
		}
		out << "#file generated automaticaly by lekuba. feel free to edit :-)\n";
		out << "username = \"" << username << "\"\n";
		out << "password = \"" << password << "\"\n";
		out << "server = \"" << server << "\"\n";
		out << "resource = \"" << resource << "\"\n";
		out << "port = " << port << "\n";
		out << "tls = \"" << tls << "\"\n";
		out.close();
		break;
	case 1:
		cout << "\nregistration failed, username already in use.\nchoose different username and try again.\n";
		exit(1);
	case 2:
		cout << "\nregistration failed, server requires registration parameters not supported by lekuba.\ntry choosing different server.\n";
		exit(1);
	case 3:
		cout << "\nregistration failed, server does not support registration.\ntry choosing different server.\n";
		exit(1);
	case -1:
		cout << "\nregistration failes because of malformed xml or unknown reason\n";
		exit(1);
	default:
		cout << "\nuknown error\n";
		exit(1);
	}
}

int main(int argc, char **argv){
	string aaa;
	rosterItemIter buddy;
	TUI *ui;
	ConfigParser *cp;
	string configFile, buf, resource, pw;
	char *tmp;
	int prt;
	
	if(argc == 2){
		if(!strcmp(argv[1], "-r")){
			reg();
		}
		if(!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
			cout << "Lekuba " << VERSION << " by Tomas Dubec\n\nusage: " << argv[0] << " [-r | -h | --help]" << endl << "\t-r\t\tregister new account\n\t-h | --help\tdisplay this help\n\n\tconfiguration file (~/.lekuba) syntax:\n\t\tusername=your_username\n\t\tpassword=\"your password\" [optional]\n\t\tserver=server_name\n\t\tresource=your_resource\n\t\tport=portnumber\n\t\ttls=yes|no" << endl;
			exit(0);
		}
	}
	else if(argc > 2){
		cout << "too many parameters!" << endl << "use -h or --help for usage help" << endl;
		exit(1);
	}

	configFile = getenv("HOME");
	configFile = configFile + "/.lekuba";
	cp = new ConfigParser(configFile);

	if(cp->getUsername() == ""){
		cerr << "missing username in " << configFile << endl;
		exit(1);
	}
	resource = cp->getResource();
	if(resource == "") resource = "lekuba";
	buf = cp->getUsername() + "@" + cp->getServer() + "/" + resource;

	prt = cp->getPort();
	if(prt == 0) prt = 5222;

	pw = cp->getPassword();
	if(pw == ""){
		tmp = getpass("enter password: ");
		pw = tmp;
	}

	jabberConnection jcon(buf, pw, cp->getServer(), cp->getPort(), cp->getTls());

	delete cp;
	//jabberConnection jcon(buf, cp->getPassword());

	//jabberConnection jcon("tombstone@localhost/bakule", "aaa");
	//jabberConnection jcon("bak@jabber.cz/bakule", "abcdef");
	//jabberConnection jcon("bakule@jabber.org/bakule", "abcdef");
	//jabberConnection jcon("tombstone@jabber.cz/bak", "omega2pi");
	//
	/*if(!jcon.login()){
		cout << "error logging in!\n";
		exit(1);
	}*/

	if(!jcon.setStatus(S_ON))
		exit(1);
	
	Roster rst(&jcon);

	//sleep(5);

	/*rst.delBuddy("bak@jabber.cz");
	sleep(30);*/

	/*if(rst.findByJid("tombstone@jabber.cz"))
		rst.sendMessage("ahoj");*/

	//rst.addBuddy("bak@jabber.cz", "bak", "test");

	ui = new TUI(&rst, &jcon);
	ui->mainLoop();

	jcon.logout(NONE);

	delete ui;

	return 0;
}
