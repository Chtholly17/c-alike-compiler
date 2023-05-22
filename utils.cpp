#include "utils.h"

string errorMessage = "";
string infoMessage = "";

void outputError(string err) {
	errorMessage = err;
}

list<int>merge(list<int>&l1, list<int>&l2) {
	list<int>ret;
	ret.assign(l1.begin(), l1.end());
	ret.splice(ret.end(), l2);
	return ret;
}

bool isVar(string name) {
	return isalpha(name[0]);
}

bool isNum(string name) {
	return isdigit(name[0]);
}

bool isControlOp(string op) {
	if (op[0] == 'j' || op == "call" || op == "return" || op == "get") {
		return true;
	}
	return false;
}