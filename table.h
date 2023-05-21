#ifndef TABLE_H
#define TABLE_H
#include "Symbol.h"

/**
 * @file table.h
 * @brief structs related to LR1 table, including DFA, status, transfer and so on
 * @author chtholly
 */

//a lr(1) item
struct Item {
	int pro;					// production id
	int pointPos;				// the position of the point
	friend bool operator ==(const Item&one, const Item& other);
	friend bool operator <(const Item&one, const Item& other);
};

//a status in DFA, a status is a set of lr(1) items
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


/**
 * @brief analyse table class, including DFA, LR1 table, first set and follow set, construct from product.txt
 * @author chtholly
 */
class AnalyseTable {
private:
	vector<Production>productions;		// all productions of this grammar
	DFA dfa;							// construct DFA from product.txt
	map<GOTO,Behavior> LR1_Table;		// construct LR1 table from product.txt
	map<Symbol,set<Symbol> >first;		// construct first set from product.txt
	map<Symbol, set<Symbol> >follow;	// construct follow set from product.txt

	status derive(Item item);
	void readProductions(const char*fileName);
	void getFirst();
	void getFollow();
	void createDFA(); 
public:
	friend class Parser;
	AnalyseTable(const char*fileName);
	// output functions
	void outputDFA(ostream& out);
	void outputDFA(const char* fileName);
};

#endif // !TABLE_H