#ifndef TABLE_H
#define TABLE_H
#include "Symbol.h"

//a lr(1) item
struct Item {
	int pro;					// production id
	int pointPos;				// the position of the point
	friend bool operator ==(const Item&one, const Item& other);
	friend bool operator <(const Item&one, const Item& other);
};

//a status in DFA
typedef set<Item> status;

//a transfer in DFA
typedef pair<int, Symbol> GOTO;

//DFA table
struct DFA {
	// all status
	list<status> stas;
	// all transfer in DFA
	map<GOTO, int> goTo;
};

//the behavior of a status in LR1 table
struct Behavior {
	tableBehave behavior;
	int nextStat;
};

class NewTemper {
private:
	int now;
public:
	NewTemper();
	string newTemp();
};

#endif // !TABLE_H