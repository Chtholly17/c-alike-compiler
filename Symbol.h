#pragma once
#include "utils.h"
#ifndef SYMBOL_H
#define SYMBOL_H

/**
 * @brief basic symbol class, including terminal and non-terminal symbols
 * @author chtholly
 * @details the symbol class is used to represent the symbols in the grammar
 */
class Symbol {
public:
	bool isVt;//是否为终结符
	string content;//内容
	friend bool operator ==(const Symbol&one, const Symbol&other);
	friend bool operator < (const Symbol&one, const Symbol&other);
	Symbol(const Symbol& sym);
	Symbol(const bool &isVt, const string& content);
	Symbol();
};

const Symbol symbol[] = { 
	{true,"int"},{true,"void"},{true,"if"},{true,"else"},{true,"while"},{true,"return"},
	{true,"+"},{true,"-"},{true,"*"},{true,"/"},{true,"="},
	{true,"=="},{true,">"},{true,"<"},{true,"!="},{true,">="},{true,"<="},
	{true,";"},{true,","},{true,"("},{true,")"},{true,"{"},{true,"}"},{true,"ID"},
	{false,"ID"}
};


class Id :public Symbol {
public:
	string name;
	Id(const Symbol& sym, const string& name);
};

class Num :public Symbol {
public:
	string number;
	Num(const Symbol& sym,const string& number);
};

class FunctionDeclare :public Symbol {
public:
	list<DType>plist;
	FunctionDeclare(const Symbol& sym);
};

class Parameter :public Symbol {
public:
	list<DType>plist;
	Parameter(const Symbol& sym);
};

class ParameterList :public Symbol {
public:
	list<DType>plist;
	ParameterList(const Symbol& sym);
};

class SentenceBlock :public Symbol {
public:
	list<int>nextList;
	SentenceBlock(const Symbol& sym);
};

class SentenceList :public Symbol {
public:
	list<int>nextList;
	SentenceList(const Symbol& sym);
};

class Sentence :public Symbol {
public:
	list<int>nextList;
	Sentence(const Symbol& sym);
};

class WhileSentence :public Symbol {
public:
	list<int>nextList;
	WhileSentence(const Symbol& sym);
};

class IfSentence :public Symbol {
public:
	list<int>nextList;
	IfSentence(const Symbol& sym);
};

class Expression :public Symbol {
public:
	string name;
	list<int>falseList;
	Expression(const Symbol& sym);
};

class M :public Symbol {
public:
	int quad;
	M(const Symbol& sym);
};

class N :public Symbol {
public:
	list<int> nextList;
	N(const Symbol& sym);
};

class AddExpression :public Symbol {
public:
	string name;
	AddExpression(const Symbol& sym);
};

class Nomial :public Symbol {
public:
	string name;
	Nomial(const Symbol& sym);
};

class Factor :public Symbol {
public:
	string name;
	Factor(const Symbol& sym);
};

class ArgumentList :public Symbol {
public:
	list<string> alist;
	ArgumentList(const Symbol& sym);
};


#endif // !SYMBOL_H