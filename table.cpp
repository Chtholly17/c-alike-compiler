#include "table.h"

bool operator < (const Item&one, const Item& other) {
	return pair<int, int>(one.pro, one.pointPos) < pair<int, int>(other.pro, other.pointPos);
}
bool operator ==(const Item&one, const Item& other) {
	return one.pro == other.pro&&one.pointPos == other.pointPos;
}


NewTemper::NewTemper() {
	now = 0;
}

string NewTemper::newTemp() {
	return string("T") + to_string(now++);
}