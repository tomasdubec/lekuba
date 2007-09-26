#include "resource_parser.h"

/******************************** LEXAN ******************************/

Lexan::Lexan(string filename){
	input.open(filename.c_str());
	if(!input.is_open()){
		cout << "could not open file " << filename << endl;
		cout << "use -r if you want to register a new account, or fill in your account information in ~/.lekuba if you already have one" << endl;
		exit(1);
	}
	rdChar();
}

Lexan::~Lexan(){
	input.close();
}

void Lexan::rdChar(void){
	znak = input.get();

	if(znak >= 'A' && znak <= 'Z' || znak >= 'a' && znak <= 'z' || znak == '.' || znak == '-' || znak == '_')
		charType = 'p';
	else if(znak >= '0' && znak <= '9')
		charType = 'c';
	else if(znak == EOF || input.eof() || znak == 0)
		charType = 'e';
	else if(znak == '\n' || znak == ' ')
		charType = 'n';
	else
		charType = znak;
}

void Lexan::rdInput(void){
	slovo.clear();

q0:
	while(charType == 'n')
		rdChar();
	
	switch(charType){
	case 'p':
		while(charType == 'p' || charType == 'c'){
			slovo += znak;
			rdChar();
		}
		symbol = isKW(slovo);
		break;
	case 'c':
		while(charType == 'c'){
			slovo += znak;
			rdChar();
		}
		symbol = CISLO;
		break;
	case '=':
		symbol = EQUALS;
		rdChar();
		break;
	case '"':
		symbol = OTHER;
		rdChar();
		while(charType != '"' && charType != 'e'){
			slovo += znak;
			rdChar();
		}
		if(charType == 'e'){
			cout << "unexpected end of file while parsing config file (missing '\"')\n";
			exit(1);
		}
		rdChar();
		break;
	case '#':
		rdChar();
		while(znak != '\n' && charType != 'e')
			rdChar();
		goto q0;
		break;
	case 'e':
		symbol = KONEC;
		break;
	}
}

LexSymb Lexan::isKW(string s){
	int i = 0;

	while(keyWords[i].slovo != "konec" && keyWords[i].slovo != s)
		i++;

	return keyWords[i].symb;
}

/************************* PARSER *******************************/

ConfigParser::ConfigParser(string filename){
	prt = 0;
	la = new Lexan(filename);
	parse();
}

ConfigParser::~ConfigParser(){
	delete la;
}

void ConfigParser::parse(void){
	bool go = true;

	while(go){
		la->rdInput();
		switch(la->symbol){
		case kwUserName:
			username();
			break;
		case kwPassword:
			password();
			break;
		case kwServer:
			server();
			break;
		case kwResource:
			resource();
			break;
		case kwTls:
			tls();
			break;
		case kwPort:
			port();
			break;
		case KONEC:
			go = false;
			break;
		default:
			cout << "error parsing config file, unexpected string or keyword (" << la->slovo << ")\n";
			exit(1);
		}
	}
}

void ConfigParser::username(void){
	la->rdInput();
	switch(la->symbol){
	case EQUALS:
		la->rdInput();
		switch(la->symbol){
		case OTHER:
			user = la->slovo;
			break;
		default:
			cout << "error parsing config file!\n\tusername expected\n";
			exit(1);
		}
		break;
	default:
		cout << "error parsing config file!\n\t'=' expected\n";
		exit(1);
	}
}

void ConfigParser::password(void){
	la->rdInput();
	switch(la->symbol){
	case EQUALS:
		la->rdInput();
		switch(la->symbol){
		case OTHER:
			pass = la->slovo;
			break;
		default:
			cout << "error parsing config file!\n\tpassword expected\n";
			exit(1);
		}
		break;
	default:
		cout << "error parsing config file!\n\t'=' expected\n";
		exit(1);
	}
}

void ConfigParser::server(void){
	la->rdInput();
	switch(la->symbol){
	case EQUALS:
		la->rdInput();
		switch(la->symbol){
		case OTHER:
			srv = la->slovo;
			break;
		default:
			cout << "error parsing config file!\n\tserver name expected\n";
			exit(1);
		}
		break;
	default:
		cout << "error parsing config file!\n\t'=' expected\n";
		exit(1);
	}
}

void ConfigParser::resource(void){
	la->rdInput();
	switch(la->symbol){
	case EQUALS:
		la->rdInput();
		switch(la->symbol){
		case OTHER:
			res = la->slovo;
			break;
		default:
			cout << "error parsing config file!\n\tresource expected\n";
			exit(1);
		}
		break;
	default:
		cout << "error parsing config file!\n\t'=' expected\n";
		exit(1);
	}
}

void ConfigParser::port(void){
	la->rdInput();
	switch(la->symbol){
	case EQUALS:
		la->rdInput();
		switch(la->symbol){
		case CISLO:
			prt = atoi(la->slovo.c_str());
			break;
		default:
			cout << "error parsing config file!\n\tport number expected\n";
			exit(1);
		}
		break;
	default:
		cout << "error parsing config file!\n\t'=' expected\n";
		exit(1);
	}
}

void ConfigParser::tls(void){
	la->rdInput();
	switch(la->symbol){
	case EQUALS:
		la->rdInput();
		switch(la->symbol){
		case OTHER:
			if(la->slovo == "yes")
				usetls = true;
			else
				usetls = false;

			break;
		default:
			cout << "error parsing config file!\n\t'yes' or 'no' expected\n";
			exit(1);
		}
		break;
	default:
		cout << "error parsing config file!\n\t'=' expected\n";
		exit(1);
	}
}

