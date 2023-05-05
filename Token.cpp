#include "Token.h"

string Token:: toString() {
	const char* TokenTypeStr[] = {
	"ENDFILE", "ERROR",
	"IF", "ELSE", "INT", "RETURN", "VOID", "WHILE",
	"ID", "NUM",
	"LBRACE", "RBRACE", "GTE", "LTE", "NEQ", "EQ", "ASSIGN", "LT", "GT", "PLUS", "MINUS", "MULT", "DIV", "LPAREN", "RPAREN", "SEMI", "COMMA",
	"LCOMMENT", "PCOMMENT"
	};
	return TokenTypeStr[type] + string("  ") + value;
}