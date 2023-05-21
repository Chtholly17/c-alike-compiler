#include "Token.h"

string Token::toString() {
	const char* TokenTypeStr[] = {
	"ENDFILE", "ERROR",
	"IF", "ELSE", "INT", "RETURN", "VOID", "WHILE",
	"ID", "NUM",
	"LBRACE", "RBRACE", "GTE", "LTE", "NEQ", "EQ", "ASSIGN", "LT", "GT", "PLUS", "MINUS", "MULT", "DIV", "LPAREN", "RPAREN", "SEMI", "COMMA",
	"LCOMMENT", "PCOMMENT"
	};
	return TokenTypeStr[type] + string("  ") + value;
}

Token::Token(string value): value(value){

	if (value == "if") {
		type = IF;
	}
	else if (value == "else") {
		type = ELSE;
	}
	else if (value == "int") {
		type = INT;
	}
	else if (value == "return") {
		type = RETURN;
	}
	else if (value == "void") {
		type = VOID;
	}
	else if (value == "while") {
		type = WHILE;
	}
	else if(value == "#"){
		type = ENDFILE;
	}
	else if(value == "{"){
		type = LBRACE;
	}
	else if(value == "}"){
		type = RBRACE;
	}
	else if(value == "+"){
		type = PLUS;
	}
	else if(value == "-"){
		type = MINUS;
	}
	else if(value == "*"){
		type = MULT;
	}
	else if(value == ","){
		type = COMMA;
	}
	else if(value == ";"){
		type = SEMI;
	}
	else if(value == "="){
		type = ASSIGN;
	}
	else if(value == ">"){
		type = GT;
	}
	else if(value == "<"){
		type = LT;
	}
	else if(value == "!"){
		type = NEQ;
	}
	else if(value == "("){
		type = LPAREN;
	}
	else if(value == ")"){
		type = RPAREN;
	}
	else if(value == "=="){
		type = EQ;
	}
	else if(value == ">="){
		type = GTE;
	}
	else if(value == "<="){
		type = LTE;
	}
	else if(value == "!="){
		type = NEQ;
	}
	else if(value == "/"){
		type = DIV;
	}
	else if(value == "\n"){
		type = NEXTLINE;
	}
	else if(isalpha(value[0])){
		type = ID;
	}
	// invalid identifier
	else
		type = ERROR;
}

