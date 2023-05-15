#pragma once
#include "utils.h"
#include "Symbol.h"
#include "Token.h"
#include "table.h"
#include "IntermediateCode.h"
#ifndef PARSER_H
#define PARSER_H

class ParserAndSemanticAnalyser {
private:
	int lineCount;
	int nowLevel;//当前分析行所在的语句块级次
	vector<Production>productions;
	DFA dfa;
	map<GOTO,Behavior> SLR1_Table;//由product.txt构造出的SLR1表
	map<Symbol,set<Symbol> >first;//由product.txt构造出的first集
	map<Symbol, set<Symbol> >follow;//由product.txt构造出的follow集
	stack<Symbol*> symStack;//符号栈
	stack<int> staStack;//状态栈
	vector<Var> varTable;//变量表
	vector<Func> funcTable;//函数表
	IntermediateCode code;//生成的四元式
	NewTemper nt;

	void readProductions(const char*fileName);
	status derive(Item item);
	void createDFA(); 
	void outputDFA(ostream& out);
	void analyse(list<Token>&words,ostream& out);
	void outputSymbolStack(ostream& out);
	void outputStateStack(ostream& out);
	void getFirst();
	void getFollow();
	Func* lookUpFunc(string ID);
	Var* lookUpVar(string ID);
	bool march(list<string>&argument_list,list<DType>&parameter_list);
	Symbol* popSymbol();
	void pushSymbol(Symbol* sym);
public:
	ParserAndSemanticAnalyser(const char*fileName);
	void outputDFA();
	void outputDFA(const char* fileName);
	void outputIntermediateCode();
	void outputIntermediateCode(const char* fileName);
	void analyse(list<Token>&words,const char*fileName);
	void analyse(list<Token>&words);
	vector<pair<int, string> > getFuncEnter();
	IntermediateCode* getIntermediateCode();
};

#endif // !PARSER_H