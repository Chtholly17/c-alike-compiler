#ifndef TOKEN_H
#define TOKEN_H
#include "utils.h"


/**
 * @brief Construct a new Token:: Token object
 * @author chtholly
 * @param type: the type of the token
 * @param value: the value of the token
 */
 // define the type of lexical token
typedef enum
{
	// special token, error, end of file
	ENDFILE, ERROR,
	// tokens about keywords
	IF, ELSE, INT, RETURN, VOID, WHILE,
	// tokens about identifiers and numbers
	ID, NUM,
	// tokens about operators and delimiters
	//  {       }    >=  <=   !=   ==    =     <   >    +      -     *     /     (        )     ;     ,   */
	LBRACE, RBRACE, GTE, LTE, NEQ, EQ, ASSIGN, LT, GT, PLUS, MINUS, MULT, DIV, LPAREN, RPAREN, SEMI, COMMA,
	// tokens about comments
	LCOMMENT, PCOMMENT,
	// new line
	NEXTLINE
} TokenType;


class Token{
private:
	TokenType type;
	string value;

public:
	Token(TokenType type, string value) :type(type), value(value) {};
    // getter and setter
	TokenType getType() { return type; }
	string getValue() { return value; }
	void setType(TokenType type) { this->type = type; }
	void setValue(string value) { this->value = value; }
	// transform Token to string
	string toString();
};

#endif // !TOKEN_H