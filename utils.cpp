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