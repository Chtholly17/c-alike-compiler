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
	// read the source, get the next char and ignore the space and tab
	char getChar();
	// read from the source and get the next token
	Token getToken();
	// output the result to the stream
	void outputToStream(ostream&out);
public:
	LexicalAnalyser(const char* path);
	~LexicalAnalyser();
	void analyse();
	void outputToScreen();
	void outputToFile(const char *fileName);
	list<Token>&getResult();
};
