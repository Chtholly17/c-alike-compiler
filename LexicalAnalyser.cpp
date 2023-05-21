#include "LexicalAnalyser.h"

/**
 * @brief Construct a new Lexical Analyser:: Lexical Analyser object
 * @author: chtholly
 * @param path: the path of the source file
 */
LexicalAnalyser::LexicalAnalyser(const char* path) {
	lineCount = 0;
	src.open(path, ios::in);
	// failed to open the source file
	if (!src.is_open()) {
		cerr << "file " << path << " open error" << endl;
		exit(-1);
	}
}

/**
 * @brief Destroy the Lexical Analyser:: Lexical Analyser object
 * @author: chtholly
 */
LexicalAnalyser::~LexicalAnalyser() {
	if (src.is_open()) {
		src.close();
	}
}

/**
 * @brief read the source, get the next char and ignore the space and tab
 * @author chtholly
 * @return char 
 */
char LexicalAnalyser::getChar() {
	char nextChar;
	while (src >> nextChar) {
		// linecount++ when the next char is '\n'
		if (nextChar == '\n') {
			lineCount++;
		}
		// ignore the space and tab
		if (nextChar == ' '|| nextChar=='\t') {
			continue;
		}
		return nextChar;
	}
	// read the end of the file
	return 0;
}

/**
 * @brief get the next token from the source file
 * @author chtholly
 * @return Token 
 */
Token LexicalAnalyser::getToken(){
	char nextChar = getChar();
	switch (nextChar) {
		// the characters needed to read next
		case '=':
			if (src.peek() == '=') {
				src.get();
				return Token("==");
			}
			else {
				return Token("=");
			}
			break;
		case '>':
			if (src.peek() == '=') {
				src.get();
				return Token(">=");
			}
			else {
				return Token(">");
			}
			break;
		case '<':
			if (src.peek() == '=') {
				src.get();
				return Token("<=");
			}
			else {
				return Token("<");
			}
			break;
		case '!':
			if (src.peek() == '=') {
				src.get();
				return Token("!=");
			}
			else {
				return Token(ERROR, string("Lexical analyser detected unknow Token ") + nextChar + string("in line ") + to_string(lineCount));
			}
			break;
		// comment or divide
		case '/':
			//line comment
			if (src.peek() == '/') {
				char buf[1024];
				src.getline(buf, 1024);
				return Token(LCOMMENT, string("/")+buf);
			}
			//paragraph comment
			else if (src.peek() == '*') {
				src.get();
				string buf = "/*";
				while (src >> nextChar) {
					buf += nextChar;
					if (nextChar == '*') {
						src >> nextChar;
						buf += nextChar;
						if (nextChar == '/') {
							return Token(PCOMMENT, buf);
							break;
						}
					}
				}
				// unclose paragraph comment
				if (src.eof()) {
					return Token(ERROR, string("unclose paragraph comment in line ") + to_string(lineCount));
				}
			}
			//divide
			else {
				return Token("/");
			}
			break;
		// other characters
		default:
			// read number
			if (isdigit(nextChar)) {
				string buf;
				buf.push_back(nextChar);
				while (nextChar=src.peek()) {
					if (isdigit(nextChar)) {
						src >> nextChar;
						buf += nextChar;
					}
					else {
						break;
					}
				}
				// this token is a NUMBER
				return Token(NUM, buf);
			}
			// read identifier
			else if (isalpha(nextChar)) {
				string buf;
				buf.push_back(nextChar);
				while (nextChar = src.peek()) {
					if (isdigit(nextChar)||isalpha(nextChar)) {
						src >> nextChar;
						buf += nextChar;
					}
					else {
						break;
					}
				}
				// might be a keyword or a identifier
				return Token(buf);
			}
			// any other characters
			else {
				Token tmp = Token(string("").append(1, nextChar));
				if(tmp.getType()==ERROR)
					return Token(ERROR, string("Lexical analyser detected unknow Token ") + nextChar + string("in line ") + to_string(lineCount));
				else
					return tmp;
			}
	}
	// unkown error
	return Token(ERROR, "UNKOWN ERROR");
}

/**
 * @brief analyse the source file and store the result in the result list
 * @author chtholly
 */
void LexicalAnalyser::analyse() {
	while (true) {
		Token t = getToken();
		result.push_back(t);
		if (t.getType() == ERROR) {
			outputError(t.getValue());
		}
		else if (t.getType() == ENDFILE) {
			break;
		}
	}
}

/**
 * @brief output the analyse result to the screen or file
 * @author chtholly
 * @param out 
 */
void LexicalAnalyser::outputToStream(ostream&out) {
	if (result.back().getType() == ERROR) {
		out << result.back().toString() <<endl;
	}
	else {
		list<Token>::iterator iter;
		for (iter = result.begin(); iter != result.end(); iter++) {
			out << (*iter).toString() << endl;
		}
	}
}

void LexicalAnalyser::outputToScreen() {
	outputToStream(cout);
}

/**
 * @brief output the analyse result to the file named fileName
 * @author chtholly
 * @param fileName 
 */
void LexicalAnalyser::outputToFile(const char *fileName) {
	ofstream fout;
	fout.open(fileName, ios::out);
	if (!fout.is_open()) {
		cerr << "file " << fileName << " open error" << endl;
		return;
	}
	outputToStream(fout);
	fout.close();
}

/**
 * @brief get the analyse result list of the source file
 * @author chtholly
 * @return list<Token>& 
 */
list<Token>&LexicalAnalyser::getResult() {
	return result;
}