#include "utils.h"

void outputError(string err) {
	cerr << err << endl;
	exit(-1);
}

list<int>merge(list<int>&l1, list<int>&l2) {
	list<int>ret;
	ret.assign(l1.begin(), l1.end());
	ret.splice(ret.end(), l2);
	return ret;
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

NewTemper::NewTemper() {
	now = 0;
}

string NewTemper::newTemp() {
	return string("T") + to_string(now++);
}