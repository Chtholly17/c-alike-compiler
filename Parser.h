#include "utils.h"
#include "Symbol.h"
#include "Token.h"
#include "table.h"
#include "IntermediateCode.h"
#include "table.h"
#ifndef PARSER_H
#define PARSER_H

/**
 * @brief the type of variable
 */
struct Var {
	string name;				// variable name
	DType type;					// variable type
	int level;					// variable level	
};

/**
 * @brief the type of function
 */
struct Func {
	string name;				// function name
	DType returnType;			// function return type
	list<DType> paramTypes;		// function parameter type
	int enterPoint;				// function enter point
};

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

	Func* lookUpFunc(string ID);
	Var* lookUpVar(string ID);
	bool march(list<string>&argument_list,list<DType>&parameter_list);
	Symbol* popSymbol();
	void pushSymbol(Symbol* sym);
	void outputStateStack(ostream& out);
	void outputSymbolStack(ostream& out);
public:
	Parser(AnalyseTable* table);
	void analyseLR(list<Token>&tokens, ostream& out0, ostream& out1);
	void analyse(list<Token>&tokens,const char* f0, const char* f1);
	void analyse(list<Token>&tokens, const char* fileName);
	vector<pair<int, pair<string,DType>>> getFuncEnter();
	IntermediateCode* getIntermediateCode();
};

#endif // !PARSER_H