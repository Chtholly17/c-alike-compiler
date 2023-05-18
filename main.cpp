#include "Compiler.h"
int main() {
	Compiler compiler = Compiler("productions.txt");
	compiler.compile("test.txt","program.asm");

	return 0;
}