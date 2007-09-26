#include "tui.h"

pthread_mutex_t wrLock;

bool TUI::initCurses(void){
	initscr();
	start_color();
	raw();
	noecho();
	keypad(stdscr, TRUE);
	halfdelay(WAIT_FOR_KEY_TIME);
	init_pair(C_FRAME, COLOR_BLUE, COLOR_BLACK); //ramecky
	init_pair(C_FRAME_INPUT, COLOR_CYAN, COLOR_BLACK); //ramecky vyrazne
	init_pair(C_ROSTER_INACT, COLOR_GREEN, COLOR_BLACK); //roster inactive
	init_pair(C_ROSTER_ACT, COLOR_BLACK, COLOR_WHITE); //roster active
	init_pair(C_ROSTER_ALERT, COLOR_YELLOW, COLOR_BLACK);
	init_pair(C_ROSTER_ACT_ALERT, COLOR_BLACK, COLOR_WHITE);
	init_pair(C_HIST_SENT, COLOR_CYAN, COLOR_BLACK);
	init_pair(C_HIST_RECV, COLOR_YELLOW, COLOR_BLACK);
	init_pair(C_CURSOR, COLOR_BLUE, COLOR_BLACK);
	init_pair(C_HELP, COLOR_BLACK, COLOR_WHITE);
	init_pair(C_INPUTBOX_BACK, COLOR_BLACK, COLOR_CYAN);
	init_pair(C_ALERT, COLOR_BLACK, COLOR_RED);
	init_pair(C_OK, COLOR_BLACK, COLOR_GREEN);
	return true;
}

WINDOW *TUI::createWin(int w, int h){
	WINDOW *tmp;

	tmp = newwin(h, w, 0, 0);

	return tmp;
}

void TUI::refreshAll(void){
	input->render();
	roster->render();
	history->render();
	help->render();
}

TUI::TUI(Roster *ros, jabberConnection *j, int mita, int mitn){
	proster = ros;
	jc = j;
	max_idle_to_away = mita * 10;
	max_idle_to_na = mitn * 10;
	idle = 0;
	beforeIdle = S_ON;

	if(!initCurses()){
		cerr << "unable to init curses!\n";
		exit(1);
	}

	pthread_mutex_init(&wrLock, NULL);

	ros->setRosterChangeCallback((void *)this, TUI::changeCallback);

	input = new TUIInput(ros);
	roster = new TUIRoster(ros);
	history = new TUIHistory(ros, false);
	help = new TUIHelp();
}

TUI::~TUI(){
	pthread_mutex_destroy(&wrLock);
	endwin();
	delete input;
	delete roster;
	delete history;
	delete help;
}

void TUI::changeCallback(void *obj){
	TUI *me;

	me = (TUI *)obj;
	me->refreshAll();
	//me->roster->render();
}

void TUI::rename(void){
	string tmp, jid;

	proster->lock();
	proster->reset();
	proster->findByJid(roster->getActiveJID());
	jid = proster->getBuddyJID();
	proster->unlock();

	TUIInputBox in("update buddy name", "new name: ");
	if(in.getInput(tmp)){
		proster->lock();
		proster->findByJid(jid);
		if(!proster->addBuddy(proster->getBuddyJID(), tmp, proster->getBuddyGroup())){
			proster->unlock();
			TUIInfoDialog info("ERROR", C_ALERT);
			return;
		}
		proster->unlock();
		refreshAll();
		TUIInfoDialog info("    OK    ", C_OK);
	}
}

bool TUI::mainLoop(void){
	int ch = 0;
	string jid, group;
	TUIInputBox *in;
	string tmp;

	while(ch != KEY_F(10)){
		ch = wgetch(stdscr);
		if(ch != ERR){
			idle = 0;
			if(jc->getStatus() != beforeIdle){
				jc->setStatus(beforeIdle);
				help->setStatus(beforeIdle);
				help->render();
			}
		}
		switch(ch){
		case 27:
			if(history->isVisible()) history->hide();
			if(input->isVisible()) input->hide();
			break;
		case 10:
			if(roster->getActive() != -1 && roster->getActiveJID() != ""){
				if(!history->isVisible()){
					history->show(roster->getActiveJID());
				}
				if(!input->isVisible()){
					input->show(roster->getActiveJID());
				}
			}
			if(input->isVisible()){
				input->writeChar(ch);
				history->render();
			}
			break;
		case KEY_DOWN:
			roster->setActive(roster->getActive() + 1);
			break;
		case KEY_UP:
			roster->setActive(roster->getActive() - 1);
			break;
		case KEY_NPAGE:
			roster->setActive(roster->getActive() + 10);
			break;
		case KEY_PPAGE:
			roster->setActive(roster->getActive() - 10);
			break;
		case KEY_F(1):
			{
			int i;
			TUIMenu mApp("account menu", 1);
			mApp.addItem("change password", 0);
			if(mApp.getChosenItem(i)){
				switch(i){
				case 0:
					string newpass;
					in = new TUIInputBox("change password", "enter new password: ");
					if(in->getInput(newpass)){
						jc->setPassword(newpass);
						TUIInfoDialog *info;
						switch(jc->changePassword()){
						case 0:  //successful
							info = new TUIInfoDialog("    OK    ", C_OK);
							break;
						case 2:
							info = new TUIInfoDialog("unsupported requierements", C_ALERT);
							break;
						case 3:
							info = new TUIInfoDialog("not supported", C_ALERT);
							break;
						default:
							info = new TUIInfoDialog("unknown error", C_ALERT);
						}
						delete info;
					}
					delete in;
					break;
				}
			}
			}
			break;
		case KEY_F(2):
			if(!input->isVisible() && roster->getActive() != -1 && roster->getActiveJID() != ""){
				int i;
				TUIMenu mBud(roster->getActiveJID(), 7);
				mBud.addItem("rename", 0);
				mBud.addItem("delete", 1);
				mBud.addItem("", -1);
				mBud.addItem("request subscription", 2);
				mBud.addItem("grant subscription", 3);
				mBud.addItem("", -1);
				mBud.addItem("remove subscription", 4);
				if(mBud.getChosenItem(i)){
					switch(i){
					case 0:
						rename();
						break;
					case 1: //delete buddy
						proster->lock();
						proster->reset();
						proster->findByJid(roster->getActiveJID());
						proster->unlock();
						if(!proster->delBuddy(proster->getBuddyJID())){
							TUIInfoDialog info("ERROR", C_ALERT);
						}
						else{
							TUIInfoDialog info("    OK    ", C_OK);
							roster->setActive(roster->getActive() - 1);
						}
						break;
					case 2: //request subscription
						proster->lock();
						proster->reset();
						proster->findByJid(roster->getActiveJID());
						proster->changeBuddySubscription(proster->getBuddyJID(), "subscribe");
						proster->unlock();
						break;
					case 3: //grant subscription
						proster->lock();
						proster->reset();
						proster->findByJid(roster->getActiveJID());
						proster->changeBuddySubscription(proster->getBuddyJID(), "subscribed");
						proster->unlock();
						break;
					case 4: //remove subscription
						proster->lock();
						proster->reset();
						proster->findByJid(roster->getActiveJID());
						proster->changeBuddySubscription(proster->getBuddyJID(), "unsubscribed");
						proster->unlock();
						break;
					}
				}
			}
			break;
		case KEY_F(3):
			{
				int i;
				TUIMenu mStav("change status", 6);
				mStav.addItem("online", 0);
				mStav.addItem("free for chat", 4);
				mStav.addItem("away", 1);
				mStav.addItem("n/a", 2);
				mStav.addItem("dnd", 5);
				mStav.addItem("offline", 3);
				if(mStav.getChosenItem(i)){
					switch(i){
					case 0:
						jc->setStatus(S_ON);
						help->setStatus(S_ON);
						beforeIdle = S_ON;
						break;
					case 1:
						jc->setStatus(S_AWAY);
						help->setStatus(S_AWAY);
						beforeIdle = S_AWAY;
						break;
					case 2:
						jc->setStatus(S_NA);
						help->setStatus(S_NA);
						beforeIdle = S_NA;
						break;
					case 3:
						jc->setStatus(S_OFF);
						return true;
						break;
					case 4:
						jc->setStatus(S_CHAT);
						help->setStatus(S_CHAT);
						beforeIdle = S_CHAT;
						break;
					case 5:
						jc->setStatus(S_DND);
						help->setStatus(S_DND);
						beforeIdle = S_DND;
						break;
					}
					help->render();
				}
			}
			break;
		case KEY_F(4):
			if(!input->isVisible()){
				in = new TUIInputBox("add new buddy", "enter JID: ");
				if(in->getInput(jid)){
					delete in; in = new TUIInputBox("add new buddy", "enter group name: ");
					if(in->getInput(group)){
						proster->lock();
						if(!proster->addBuddy(jid, "", group)){
							TUIInfoDialog info("ERROR", C_ALERT);
						}
						proster->unlock();
						refreshAll();
						TUIInfoDialog info("    OK    ", C_OK);
					}
				}
				delete in;
			}
			break;
		case 'r':
			if(!history->isVisible())
				rename();
			else
				input->writeChar(ch);
			break;
		case 'm':
			jid = roster->getActiveJID();
			if(!input->isVisible()){
				in = new TUIInputBox("move to group", "enter group name: ");
				if(in->getInput(group)){
					proster->lock();
					proster->findByJid(jid);
					if(!proster->addBuddy(jid, proster->getBuddyName(), group)){
						TUIInfoDialog info("ERROR", C_ALERT);
					}
					proster->unlock();
					refreshAll();
					TUIInfoDialog info("    OK    ", C_OK);
				}
				delete in;
			}
			else
				input->writeChar(ch);
			break;
		case KEY_RESIZE: case CTRL('l'):
			endwin();
			refresh();
			//resizeterm(LINES, COLS);
			roster->clear();
			input->resize(); //we must explicitly resize input buffer! (s*it, 2 hours spent on realizing this! i'm a f...ing moron!)
			refreshAll();
			break;
		case ERR: //nothing pressed, timeout occured
			idle += WAIT_FOR_KEY_TIME;
			if(jc->getConnectionStatus() == OFFLINE)
				return false;
			else if(idle > max_idle_to_away && jc->getStatus() != S_NA && jc->getStatus() != S_AWAY){
				jc->setStatus(S_AWAY);
				help->setStatus(S_AWAY);
				help->render();
			}
			else if(idle > max_idle_to_na && jc->getStatus() != S_NA){
				jc->setStatus(S_NA);
				help->setStatus(S_NA);
				help->render();
			}
			break;
		default:
			input->writeChar(ch);
			break;
		}

		roster->render();
	}
	return true;
}

/**************** TUI roster ***************/

TUIRoster::TUIRoster(Roster *ros){
	roster = ros;

	win = newwin(ROSTER_H, ROSTER_W, ROSTER_Y, ROSTER_X);
	akt = -1;
	offset = 0;
	render();
}

TUIRoster::~TUIRoster(){

}

void TUIRoster::render(void){
	int index=0;
	string buf, tmp, blank(ROSTER_W - 2, ' '), lastGroup="";

	pthread_mutex_lock(&wrLock);
	pocetRadku = 0;
	wresize(win, ROSTER_H, ROSTER_W);
	wattron(win, COLOR_PAIR(C_FRAME));
	box(win, 0, 0);
	wattroff(win, COLOR_PAIR(C_FRAME));


	roster->lock();
	roster->reset();
	if(!roster->isEmpty()){
		for(int a=0;a < offset; a++){
			if(lastGroup != roster->getBuddyGroup()){
				lastGroup = roster->getBuddyGroup();
			}
			else{
				roster->next();
			}
			pocetRadku++;
		}
		do{
			//group title
			if(lastGroup != roster->getBuddyGroup() && !roster->getBuddyAlert()){
				lastGroup = roster->getBuddyGroup();
				mvwprintw(win, ++index, 1, blank.c_str());
				if(index + offset - 1 == akt){
					aktJID = "";
					wattron(win, COLOR_PAIR(C_ROSTER_ACT_ALERT));
				}
				else
					wattron(win, COLOR_PAIR(C_ROSTER_ALERT) | A_BOLD);

				mvwprintw(win, index, 1, lastGroup.c_str());
				wattroff(win, COLOR_PAIR(C_ROSTER_ACT_ALERT));
			}
			else{
				switch(roster->getBuddyStatus()){
				case S_OFF:
					if(roster->getBuddySubscription() == S_REQUEST)
						buf = " [s] ";
					else if(roster->getBuddySubscription() != S_NONE && roster->getBuddySubscription() != S_FROM)
						buf = " [ ] ";
					else
						buf = " [x] ";
					break;
				case S_ON:
					buf = " [o] ";
					break;
				case S_AWAY:
					buf = " [a] ";
					break;
				case S_NA:
					buf = " [n] ";
					break;
				case S_CHAT:
					buf = " [c] ";
					break;
				case S_DND:
					buf = " [d] ";
					break;
				default:
					buf = " [?] ";
				}
				tmp = roster->getBuddyName();
				if(tmp == "")
					tmp = roster->getBuddyJID();
				//if name is too big
				if(tmp.size() > ROSTER_W - 6)
					tmp = tmp.substr(0, ROSTER_W - 6);
				buf += tmp;

				//clear whatever there was before
				mvwprintw(win, ++index, 1, blank.c_str());

				if(index + offset - 1 == akt && roster->getBuddyAlert()){
					aktJID = roster->getBuddyJID();
					wattron(win, COLOR_PAIR(C_ROSTER_ACT_ALERT));
				}
				else if(index + offset - 1 == akt){
					aktJID = roster->getBuddyJID();
					wattroff(win, A_BOLD);
					wattron(win, COLOR_PAIR(C_ROSTER_ACT));
				}
				else if(roster->getBuddyAlert())
					wattron(win, COLOR_PAIR(C_ROSTER_ALERT) | A_BOLD);
				else{
					wattroff(win, A_BOLD);
					wattron(win, COLOR_PAIR(C_ROSTER_INACT));
				}
				mvwprintw(win, index, 1, buf.c_str());

				//clear nex line (necesary when deleting contacts)
				wattron(win, COLOR_PAIR(C_FRAME));
				//mvwprintw(win, index + 1, 1, blank.c_str());
				if(!roster->next())
					break;
			}
			pocetRadku++;
		}
		while(index < ROSTER_H - 2);
		while(roster->next()){ //we need the correct line count in pocetRadku
			if(lastGroup != roster->getBuddyGroup()){
				lastGroup = roster->getBuddyGroup();
				pocetRadku++;
			}
			pocetRadku++;
		}
	}
	else{
		mvwprintw(win, 1, 1, "empty");
	}
	refresh();
	wrefresh(win);
	roster->unlock();

	pthread_mutex_unlock(&wrLock);
}

void TUIRoster::clear(void){
	wattron(win, COLOR_PAIR(C_FRAME));
	for(int y = 1;y <= ROSTER_H - 2;y++)
		for(int x = 1;x < ROSTER_W - 1;x++)
			mvwaddch(win, y, x, ' ');

	refresh();
	wrefresh(win);
}

void TUIRoster::setActive(int a){
	if(pocetRadku == 0){
		akt = -1;
		return;
	}
	if(a > pocetRadku)
		akt = pocetRadku;
	else if(a < 0)
		akt = 0;
	else
		akt = a;

	if(akt - offset >= ROSTER_H - 2){
		offset += ((akt - offset) - ROSTER_H + 5);
		clear();
	}
	else if(akt - offset < 0){
		offset -= (offset - akt);
		clear();
	}

	if(offset < 0) offset = 0;
}

/*********** TUIHistory ***************/

TUIHistory::TUIHistory(Roster *ros, bool vis){
	roster = ros;
	visible = vis;
}

TUIHistory::~TUIHistory(){

}

void TUIHistory::render(void){
	int lineIndex=HISTORY_Y + HISTORY_H - 2, index;
	bool sent;
	double date;
	string text, buf;
	struct tm *time;
	time_t t;

	pthread_mutex_lock(&wrLock);
	if(visible){
		wresize(win, HISTORY_H, HISTORY_W);
		mvwin(win, HISTORY_Y, HISTORY_X);
		clear();
		wattron(win, COLOR_PAIR(C_FRAME));
		box(win, 0, 0);
		mvwaddch(win, HISTORY_H-1, 0, ACS_LTEE);
		mvwaddch(win, HISTORY_H-1, HISTORY_W-1, ACS_RTEE);
		mvwprintw(win, 0, HISTORY_W - 9 - activeJID.size(), "[ %s ]", activeJID.c_str());
		wattroff(win, COLOR_PAIR(C_FRAME));

		activeHistory->reset();
		do{
			activeHistory->getMessage(sent, date, text);
			if(sent){
				text.insert(0, " << ");
				wattron(win, COLOR_PAIR(C_HIST_SENT));
			}
			else{
				text.insert(0, " >> ");
				wattron(win, COLOR_PAIR(C_HIST_RECV));
			}

			t = (time_t)date;
			time = localtime(&t);
			stringstream out;
			out << time->tm_mday << "." << time->tm_mon+1 << "." << time->tm_year+1900 << " " << time->tm_hour << ":";
			if(time->tm_min < 10)
				out << "0" << time->tm_min;
			else
				out << time->tm_min;

			text.insert(0, out.str());

			index = (text.length()/(INPUT_W - 2)) * (INPUT_W - 2);
			do{
				buf = text.substr(index, INPUT_W - 2);
				mvwprintw(win, lineIndex, 1, "%s", buf.c_str());
				index -= (INPUT_W - 2);
				lineIndex--;
			}
			while(index >= 0);
		}
		while(lineIndex > 0 && activeHistory->next());

		refresh();
		wrefresh(win);
	}
	else{
		for(int y = HISTORY_Y;y<HISTORY_Y+HISTORY_H;y++)
			for(int x = HISTORY_X;x<HISTORY_X+HISTORY_W;x++){
				mvaddch(y, x, ' ');
			}
				
		refresh();
		wrefresh(stdscr);
	}
	pthread_mutex_unlock(&wrLock);
}

void TUIHistory::clear(void){
	for(int y = 1;y <= HISTORY_H - 2;y++)
		for(int x = 1;x < HISTORY_W - 1;x++)
			mvwaddch(win, y, x, ' ');

	refresh();
	wrefresh(win);
}

void TUIHistory::show(string j){
	int a;
	
	roster->lock();
	roster->reset();
	roster->findByJid(j);
	activeJID = roster->getBuddyJID();
	activeHistory = roster->getBuddyHistory();
	visible = true;
	win = newwin(HISTORY_H, HISTORY_W, HISTORY_Y, HISTORY_X);
	if(roster->getBuddySubscription() == S_REQUEST){
		TUIConfirmDialog conf(activeJID + " wants to add you to his/her roster", "accept", "reject");
		if(conf.getAnswer()){
			if(roster->getBuddyStatus() == S_NOTINLIST)
				roster->addBuddy(activeJID, "", "");
			roster->findByJid(activeJID);
			roster->changeBuddySubscription(activeJID, "subscribed");
			roster->changeBuddySubscription(activeJID, "subscribe");
		}
		else{
			roster->changeBuddySubscription(activeJID, "unsubscribed");
			if(roster->getBuddyStatus() == S_NOTINLIST){
				roster->delBuddy(activeJID);
				hide();
			}
		}
	}
	render();
	roster->unlock();
}

void TUIHistory::hide(void){
	visible = false;
	delwin(win);
	render();
}

/******************** TUIInput *******************/

TUIInput::TUIInput(Roster *ros){
	roster = ros;
	visible = false;
	x = y = 1;
	index = 0;
	buf = (char *)malloc((2 * (INPUT_W - 2)) * sizeof(char));
	memset(buf, 0, 2 * (INPUT_W - 2));
}

TUIInput::~TUIInput(){
	delwin(win);
	free(buf);
}

void TUIInput::resize(void){
	clear();
	buf = (char *)realloc(buf, (2 * (INPUT_W - 2)) * sizeof(char));
}

void TUIInput::clear(void){
	if(visible){
		for(int y = 1;y <= INPUT_H - 2;y++)
			for(int x = 1;x < INPUT_W - 1;x++)
				mvwaddch(win, y, x, ' ');
	}

	refresh();
	//wrefresh(win);
}

void TUIInput::show(string j){
	int a, ch;
	
	roster->lock();
	roster->reset();
	roster->findByJid(j);
	visible = true;
	jid = roster->getBuddyJID();
	roster->setBuddyEasy();
	roster->unlock();
	win = newwin(INPUT_H, INPUT_W, INPUT_Y, INPUT_X);
	render();
}

void TUIInput::hide(void){
	roster->lock();
	roster->findByJid(jid);
	roster->setBuddyEasy();
	roster->unlock();
	visible = false;
	memset(buf, 0, 2 * (INPUT_W - 2));
	x = y = 1;
	index = 0;
	delwin(win);
	render();
}

void TUIInput::render(void){
	pthread_mutex_lock(&wrLock);
	if(visible){
		wresize(win, INPUT_H, INPUT_W);
		mvwin(win, INPUT_Y, INPUT_X);
		wattron(win, COLOR_PAIR(C_FRAME));
		box(win, 0, 0);
		mvwaddch(win, 0, 0, ACS_LTEE);
		for(int a=1;a<INPUT_W-1;a++)
			mvwaddch(win, 0, a, ACS_HLINE);
		mvwaddch(win, 0, INPUT_W-1, ACS_RTEE);
		wattroff(win, COLOR_PAIR(C_FRAME));
		refresh();
		wrefresh(win);
	}
	else{
		for(int y = INPUT_Y;y<INPUT_Y + INPUT_H;y++)
			for(int x = INPUT_X;x<INPUT_X + INPUT_W;x++)
				mvaddch(y, x, ' ');

		refresh();
		wrefresh(stdscr);
	}
	pthread_mutex_unlock(&wrLock);
}

void TUIInput::clearWin(void){
	for(int y = 0;y<INPUT_H;y++)
		for(int x = 0;x<INPUT_W;x++){
			mvwaddch(win, y, x, ' ');

		}
	
	wrefresh(win);
}

void TUIInput::writeChar(int ch){
	string tmp;

	if(!visible)
		return;
	

	if(ch == KEY_BACKSPACE || ch == 127){
		if(index > 0)
			buf[--index] = 0;
		if(x > 1){
			//delete cursor
			mvwaddch(win, y, x, ' ');
			//delete last char
			mvwaddch(win, y, --x, ' ');
		}
		else if(x <= 1 && y > 1){
			//delete cursor
			mvwaddch(win, y, x + 1, ' ');
			x = INPUT_W - 2;
			//delete last char
			mvwaddch(win, --y, x, ' ');
		}
	}
	else if(ch == 10){ //odesleme a smazeme zpravu
		tmp = buf;
		if(tmp != ""){
			roster->lock();
			roster->findByJid(jid);
			roster->sendMessage(tmp);
			roster->unlock();
			clearWin();
			render();
			index = 0;
			x = y = 1;
			memset(buf, 0, 2 * (INPUT_W - 2));
		}
	}
	else if(isprint(ch)){
		if(x > INPUT_W - 2 && y < INPUT_H - 2){
			y++;
			x = 1;
		}
		mvwaddch(win, y, x++, ch);
		buf[index++] = (char)ch;
	}
	
	wattron(win, COLOR_PAIR(C_CURSOR) | A_BOLD);
	mvwaddch(win, y, x, '|');
	wattroff(win, COLOR_PAIR(C_CURSOR) | A_BOLD);

	render();
}

/************** TUIHelp ********************/

TUIHelp::TUIHelp(void){
	st = S_ON;
	win = newwin(1, COLS, LINES - 1, 0);
	render();
}

TUIHelp::~TUIHelp(){
}

void TUIHelp::setStatus(enumStav s){
	st = s;
}

void TUIHelp::render(void){
	char tmp;

	pthread_mutex_lock(&wrLock);
	mvwin(win, LINES-1, 0);
	wresize(win, 1, COLS);
	wattron(win, COLOR_PAIR(C_HELP));
	for(int a=0;a<COLS;a++)
		mvwaddch(win, 0, a, ' ');

	mvwprintw(win, 0, 0, " F1 - account menu | F2 - buddy menu | F3 - change status | F4 - add new buddy | F10 - quit");

	switch(st){
	case S_ON:
		tmp = 'o';
		break;
	case S_AWAY:
		tmp = 'a';
		break;
	case S_NA:
		tmp = 'n';
		break;
	case S_CHAT:
		tmp = 'c';
		break;
	case S_DND:
		tmp = 'd';
		break;
	default:
		tmp = '?';
	}
	wattron(win, A_BOLD);
	mvwprintw(win, 0, COLS - 4, "[%c]", tmp);
	wattroff(win, COLOR_PAIR(C_HELP) | A_BOLD);

	refresh();
	wrefresh(win);
	pthread_mutex_unlock(&wrLock);
}

/**************** TUIInputBox ******************/

TUIInputBox::TUIInputBox(string t, string q){
	query = q;
	title = t;
	win = newwin(INPUTBOX_H, INPUTBOX_W, (LINES - INPUTBOX_H) / 2, (COLS - INPUTBOX_W) / 2);
	text = "";
	render();
}

TUIInputBox::~TUIInputBox(){
	for(int a=0;a<INPUTBOX_H;a++)
		for(int b=0;b<INPUTBOX_W;b++)
			mvwaddch(win, a, b, ' ');

	wrefresh(win);
	delwin(win);
}

bool TUIInputBox::getInput(string &t){
	int ch;

	while(ch != 27){
		ch = wgetch(stdscr);
		switch(ch){
		case 10:
			t = text;
			return true;
		case KEY_BACKSPACE: case 127:
			text = text.substr(0, text.length() - 1);
			break;
		default:
			//if(text.length() < INPUTBOX_W - 4)
			if(isprint(ch))
				text = text + (char)ch;
		}
		render();
	}
	return false;
}

void TUIInputBox::render(void){
	string tmp;
	int start, len, a;

	pthread_mutex_lock(&wrLock);
	wattron(win, COLOR_PAIR(C_FRAME));
	box(win, 0, 0);
	mvwprintw(win, 0, INPUTBOX_W/2 - title.length()/2 - 2, "[ %s ]", title.c_str());
	wattroff(win, COLOR_PAIR(C_FRAME));
	
	wattron(win, COLOR_PAIR(C_ROSTER_INACT));
	mvwprintw(win, INPUTBOX_H/2 - 1, 2, "%s", query.c_str());
	wattroff(win, COLOR_PAIR(C_ROSTER_INACT));

	wattron(win, COLOR_PAIR(C_INPUTBOX_BACK));
	for(int a=2;a<INPUTBOX_W-2;a++)
		mvwaddch(win, INPUTBOX_H/2, a, ' ');

	start = (text.length() - INPUTBOX_W + 4);
	if(start < 0)
		start = 0;

	tmp = text.substr(start, INPUTBOX_W - 4);

	mvwprintw(win, INPUTBOX_H/2, 2, "%s", tmp.c_str());
	wattroff(win, COLOR_PAIR(C_INPUTBOX_BACK));

	refresh();
	wrefresh(win);
	pthread_mutex_unlock(&wrLock);
}

/*************** TUIInfoDialog *************/

void TUIInfoDialog::TUIInfoFill(void){
	for(int a=0;a<LINES/2 - 2;a++)
		for(int b=0;b<COLS/2 - (text.length() + 2)/2;b++)
			mvwaddch(win, a, b, ' ');
}

TUIInfoDialog::TUIInfoDialog(string t, int cp){
	text = t;
	win = newwin(5, text.length() + 2, LINES/2 - 2, COLS/2 - (text.length() + 2)/2);
	wattron(win, COLOR_PAIR(cp));
	TUIInfoFill();
	box(win, 0, 0);
	pthread_mutex_lock(&wrLock);
	mvwprintw(win, 2, 1, "%s", text.c_str());
	wattroff(win, COLOR_PAIR(cp));
	wrefresh(win);
	refresh();
	wgetch(stdscr);
	pthread_mutex_unlock(&wrLock);
}

TUIInfoDialog::~TUIInfoDialog(){
	wattron(win, COLOR_PAIR(C_FRAME));
	pthread_mutex_lock(&wrLock);
	TUIInfoFill();
	pthread_mutex_unlock(&wrLock);

	wrefresh(win);
	delwin(win);
	refresh();
}

/************** TUIMenu ********************/

TUIMenu::TUIMenu(string cap, int mi){
	maxItems = mi;

	chosen = index = 0;
	caption = cap;
	longest = caption.length() + 4;
	win = NULL;
	titles = new string[maxItems];
	numbers = new int[maxItems];
}

TUIMenu::~TUIMenu(){
	delete [] titles;
	delete [] numbers;

	wattron(win, COLOR_PAIR(C_FRAME));
	fill();
	wattroff(win, COLOR_PAIR(C_FRAME));
	wrefresh(win);
	delwin(win);
}

void TUIMenu::addItem(string text, int num){
	if(index >= maxItems)
		return;

	titles[index] = text;
	numbers[index] = num;

	if(text.length() > longest)
		longest = text.length();
	
	index++;

	if(win != NULL)
		delwin(win);

	win = newwin(index + 2, longest + 4, (LINES - index - 2) / 2, (COLS - longest - 4) / 2);
}

bool TUIMenu::getChosenItem(int &i){
	int ch = 0;

	render();
	while(ch != 27){
		ch = getch();
		switch(ch){
		case KEY_DOWN:
			if(numbers[++chosen] == -1)
				chosen++;
			break;
		case KEY_UP:
			if(numbers[--chosen] == -1)
				chosen--;
			break;
		case 10:
			i = numbers[chosen];
			return true;
		}
		if(chosen >= index) chosen = index - 1;
		if(chosen < 0) chosen = 0;
		render();
	}
	return false;
}

void TUIMenu::fill(void){
	for(int a=0;a<index + 2;a++)
		for(int b=0;b<longest + 4;b++)
			mvwaddch(win, a, b, ' ');
}

void TUIMenu::render(void){
	pthread_mutex_lock(&wrLock);
	wattron(win, COLOR_PAIR(C_FRAME));
	box(win, 0, 0);
	mvwprintw(win, 0, (longest + 4)/2 - caption.length()/2 - 2, "[ %s ]", caption.c_str());
	wattroff(win, COLOR_PAIR(C_FRAME));
	
	for(int a=0;a<index;a++){
		if(numbers[a] != -1){
			if(chosen == a)
				wattron(win, COLOR_PAIR(C_ROSTER_ACT));
			else
				wattron(win, COLOR_PAIR(C_ROSTER_INACT));

			mvwprintw(win, a + 1, 2, "%s", titles[a].c_str());
		}
		else{
			wattron(win, COLOR_PAIR(C_FRAME));
			mvwaddch(win, a + 1, 0, ACS_LTEE);
			for(int b=1;b<longest + 3;b++){
				//mvwaddch(win, a + 1, b, 'x');
				mvwaddch(win, a + 1, b, ACS_HLINE);
			}
			mvwaddch(win, a + 1, longest + 3, ACS_RTEE);
			wattroff(win, COLOR_PAIR(C_FRAME));
		}
	}

	refresh();
	wrefresh(win);

	pthread_mutex_unlock(&wrLock);
}

/************** TUIConfirmDialog *************/

TUIConfirmDialog::TUIConfirmDialog(string q, string a, string n){
	question = q;
	ano = a;
	ne = n;
	choice = true;
	win = newwin(7, q.length() + 4, (LINES - 7) / 2, (COLS - q.length() - 4) / 2);
}

TUIConfirmDialog::~TUIConfirmDialog(){
	for(int a=0;a<7;a++)
		for(int b=0;b<question.length() + 4;b++)
			mvwaddch(win, a, b, ' ');
	delwin(win);
}

bool TUIConfirmDialog::getAnswer(void){
	int ch = 0;

	render();

	while(true){
		ch = getch();
		switch(ch){
		case KEY_LEFT:case KEY_RIGHT:case 9:
			choice = !choice;
			render();
			break;
		case 10:
			return choice;
			break;
		}
	}
}

void TUIConfirmDialog::render(void){
	pthread_mutex_lock(&wrLock);
	wattron(win, COLOR_PAIR(C_FRAME));
	box(win, 0, 0);
	wattroff(win, COLOR_PAIR(C_FRAME));

	wattron(win, COLOR_PAIR(C_ROSTER_INACT));
	mvwprintw(win, 2, 2, "%s", question.c_str());

	if(choice)
		wattron(win, COLOR_PAIR(C_ROSTER_ACT));
	else
		wattron(win, COLOR_PAIR(C_ROSTER_INACT));
	mvwprintw(win, 4, (((question.length() + 4) / 4) - ((ano.length() + 2) / 2)), "[%s]", ano.c_str());

	if(!choice)
		wattron(win, COLOR_PAIR(C_ROSTER_ACT));
	else
		wattron(win, COLOR_PAIR(C_ROSTER_INACT));
	mvwprintw(win, 4, (((question.length() + 4) / 4 * 3) - ((ne.length() + 2) / 2)), "[%s]", ne.c_str());

	refresh();
	wrefresh(win);
	pthread_mutex_unlock(&wrLock);
}

