#include "Symbol.h"

bool operator ==(const Symbol&one, const Symbol&other) {
	return one.content == other.content;
}

bool operator <(const Symbol&one, const Symbol&other) {
	return one.content < other.content;
}

bool isVT(string s) {
	if (s == "int" || s == "void" || s == "if" || s == "while" || s == "else" || s == "return") {
		return true;
	}
	if (s == "+" || s == "-" || s == "*" || s == "/" || s == "=" || s == "==" || s == ">" || s == "<" || s == "!=" || s == ">=" || s == "<=") {
		return true;
	}
	if (s == ";" || s == "," || s == "(" || s == ")" || s == "{" || s == "}" || s == "ID" || s == "NUM") {
		return true;
	}
	return false;
}

Symbol::Symbol(const Symbol& sym) :isVt(sym.isVt), content(sym.content) {};

Symbol::Symbol(const bool& isVt, const string& content) :isVt(isVt), content(content) {};

Symbol::Symbol() { this->isVt = true; };

Id::Id(const Symbol& sym, const string& name) : Symbol(sym) {
	this->name = name;
}

Num::Num(const Symbol& sym, const string& number) : Symbol(sym) {
	this->number = number;
}

FunctionDeclare::FunctionDeclare(const Symbol& sym) : Symbol(sym) {}

Parameter::Parameter(const Symbol& sym) : Symbol(sym) {}

ParameterList::ParameterList(const Symbol& sym) : Symbol(sym) {}

SentenceBlock::SentenceBlock(const Symbol& sym) : Symbol(sym) {}

SentenceList::SentenceList(const Symbol& sym) : Symbol(sym) {}

Sentence::Sentence(const Symbol& sym) : Symbol(sym) {}

WhileSentence::WhileSentence(const Symbol& sym) : Symbol(sym) {}

IfSentence::IfSentence(const Symbol& sym) : Symbol(sym) {}

Expression::Expression(const Symbol& sym) : Symbol(sym) {}

M::M(const Symbol& sym) : Symbol(sym) {}

N::N(const Symbol& sym) : Symbol(sym) {}

AddExpression::AddExpression(const Symbol& sym) : Symbol(sym) {}

Nomial::Nomial(const Symbol& sym) : Symbol(sym) {}

Factor::Factor(const Symbol& sym) : Symbol(sym) {}

ArgumentList::ArgumentList(const Symbol& sym) : Symbol(sym) {}