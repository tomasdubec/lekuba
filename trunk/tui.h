#ifndef _TUI_H_
#define _TUI_H_
#include <iostream>
#include <sstream>
#include <ncurses.h>
#include <time.h>
#include "roster.h"

#ifndef CTRL
#define CTRL(x) ((x) & 0x1F)
#endif
#define WAIT_FOR_KEY_TIME 10

#define ROSTER_X 0
#define ROSTER_Y 0
#define ROSTER_W (COLS / 4)
#define ROSTER_H LINES - 1
#define HISTORY_X (ROSTER_W + ROSTER_X)
#define HISTORY_Y 0
#define HISTORY_W (COLS - ROSTER_W)
#define HISTORY_H (LINES - 4)
#define INPUT_X HISTORY_X
#define INPUT_Y (HISTORY_H + HISTORY_Y - 1)
#define INPUT_W (COLS - ROSTER_W)
#define INPUT_H (LINES - HISTORY_H)
#define INPUTBOX_H 6
#define INPUTBOX_W 50

//text colors
#define C_FRAME 1
#define C_FRAME_INPUT 8
#define C_ROSTER_INACT 2
#define C_ROSTER_ACT 3
#define C_ROSTER_ACT_ALERT 4
#define C_ROSTER_ALERT 5
#define C_HIST_SENT 6
#define C_HIST_RECV 7
#define C_CURSOR 9
#define C_HELP 10
#define C_INPUTBOX_BACK 11
#define C_ALERT 12
#define C_OK 13

using namespace std;

class TUIRoster{
	int akt;
	string aktJID;
	int offset, pocetRadku;
	Roster *roster;
	WINDOW *win;
public:
	TUIRoster(Roster *ros);
	~TUIRoster();
	void render(void);
	void clear(void);
	void setActive(int a);
	int getActive(void){return akt;}
	string getActiveJID(void){return aktJID;}
};

class TUIHistory{
	WINDOW *win;
	bool visible;
	History *activeHistory;
	string activeJID;
	Roster *roster;
public:
	TUIHistory(Roster *, bool);
	~TUIHistory();
	bool isVisible(void){return visible;}
	void clear(void);
	void render(void);
	void show(string);
	void hide(void);
};

class TUIInput{
	WINDOW *win;
	bool visible;
	string jid;
	Roster *roster;
	int x, y, index;
	char *buf;

	void clearWin(void);
public:
	TUIInput(Roster *);
	~TUIInput();
	void render();
	bool isVisible(void){return visible;}
	void show(string);
	void hide(void);
	void writeChar(int);
	void resize(void);
	void clear(void);
};

class TUIHelp{
	WINDOW *win;
	enumStav st;
public:
	TUIHelp(void);
	~TUIHelp();
	void render(void);
	void setStatus(enumStav);
};

class TUI{
	TUIRoster *roster;
	TUIHistory *history;
	TUIInput *input;
	TUIHelp *help;
	Roster *proster;
	jabberConnection *jc;
	int max_idle_to_away, max_idle_to_na, idle;
	enumStav beforeIdle;

	bool initCurses(void);
	WINDOW *createWin(int, int);
	void refreshAll(void);
	void rename(void);
public:
	TUI(Roster *ros, jabberConnection *, int mita = 600, int mitn = 3600);
	~TUI();
	bool mainLoop(void);
	static void changeCallback(void *);
};

class TUIInputBox{
	WINDOW *win;
	string title, query, text;
public:
	TUIInputBox(string, string);
	~TUIInputBox();
	void render(void);
	bool getInput(string &);
};

class TUIInfoDialog{
	WINDOW *win;
	string text;
	void TUIInfoFill(void);
public:
	TUIInfoDialog(string, int);
	~TUIInfoDialog();
	void show(void);
};

class TUIMenu{
	string *titles;
	int *numbers;
	int longest, index, maxItems, chosen;
	WINDOW *win;
	string caption;

	void fill(void);
public:
	TUIMenu(string, int);
	~TUIMenu();
	void addItem(string, int);
	bool getChosenItem(int&);
	void render(void);
};

class TUIConfirmDialog{
	WINDOW *win;
	string question, ano, ne;
	bool choice;
public:
	TUIConfirmDialog(string, string, string);
	~TUIConfirmDialog();
	bool getAnswer(void);
	void render(void);
};

#endif

