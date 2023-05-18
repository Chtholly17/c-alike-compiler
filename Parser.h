#include "utils.h"
#include "Symbol.h"
#include "Token.h"
#include "table.h"
#include "IntermediateCode.h"
#include "table.h"
#ifndef PARSER_H
#define PARSER_H

/**
 * @brief the parser class, which is used to parse the source code and run semantic analysis
 * @author chtholly
 */
class Parser {
private:
	int lineCount;						// current line number
	int nowLevel;						// level of current statement block, used to realize function scope
	
	AnalyseTable* analyseTable;			// analyse table
	stack<Symbol*> symStack;			// symbol stack
	stack<int> staStack;				// status stack
	vector<Var> varTable;				// variable table
	vector<Func> funcTable;				// function table
	IntermediateCode code;				// intermediate code
	NewTemper nt;						// new temper generator

	Func* lookUpFunc(string ID);
	Var* lookUpVar(string ID);
	bool march(list<string>&argument_list,list<DType>&parameter_list);
	Symbol* popSymbol();
	void pushSymbol(Symbol* sym);
	void outputStateStack(ostream& out);
	void outputSymbolStack(ostream& out);
public:
	Parser(AnalyseTable* table);
	void analyse(list<Token>&tokens, ostream& out);
	void analyse(list<Token>&tokens, const char* fileName);
	vector<pair<int, string> > getFuncEnter();
	IntermediateCode* getIntermediateCode();
};

#endif // !PARSER_H