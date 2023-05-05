#pragma once
#include "utils.h"
#include "Token.h"

/**
 * @brief lexical analyser
 * @author chtholly
 * @details analyse the source file and output the result as a list of tokens
 */
class LexicalAnalyser {
private:
	// the source file
	ifstream src;
	// result, a list of tokens
	list<Token> result;
	// the current line number
	int lineCount;
private:
	char getNextChar();
	Token getNextToken();
	void outputToStream(ostream&out);
public:
	LexicalAnalyser(const char* path);
	~LexicalAnalyser();
	void analyse();
	void outputToScreen();
	void outputToFile(const char *fileName);
	list<Token>&getResult();
};
