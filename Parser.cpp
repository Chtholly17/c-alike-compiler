#include "Parser.h"

/**
 * @brief Construct a new Parser:: Parser object
 * @param table : the analyse table of corresponding grammar
 */
Parser::Parser(AnalyseTable* table) {
	this->analyseTable = table;
	this->lineCount = 1;
	this->nowLevel = 0;
}

/**
 * @brief find the function in the function table
 * @param ID: the name of the function
 * @return Func*: the pointer to the function, if not found, return NULL
 */
Func* Parser::lookUpFunc(string ID) {
	for (vector<Func>::iterator iter = funcTable.begin(); iter != funcTable.end(); iter++) {
		if (iter->name == ID) {
			return &(*iter);
		}
	}
	return NULL;
}

/**
 * @brief find the variable in the variable table
 * @param ID: the name of the variable
 * @return Func*: the pointer to the variable, if not found, return NULL
 */
Var* Parser::lookUpVar(string ID) {
	for (vector<Var>::reverse_iterator iter = varTable.rbegin(); iter != varTable.rend(); iter++) {
		if (iter->name == ID) {
			return &(*iter);
		}
	}
	return NULL;
}


vector<pair<int, string> >Parser::getFuncEnter() {
	vector<pair<int, string> >ret;
	for (vector<Func>::iterator iter = funcTable.begin(); iter != funcTable.end(); iter++) {
		ret.push_back(pair<int, string>(iter->enterPoint, iter->name));
	}
	sort(ret.begin(), ret.end());
	return ret;
}


/**
 * @brief check whether the argument list and parameter list match
 * @param argument_list: the argument list
 * @param parameter_list: the parameter list
 * @return true: match
 * @return false: not match
 */
bool Parser::march(list<string>&argument_list, list<DType>&parameter_list) {
	return argument_list.size() == parameter_list.size();
}

/**
 * @brief pop a symbol from the symbol stack
 * @return Symbol*: the pointer to the symbol
 */
Symbol* Parser::popSymbol() {
	Symbol* ret = symStack.top();
	symStack.pop();
	staStack.pop();
	return ret;
}

/**
 * @brief push a symbol to the symbol stack
 * @param sym: the pointer to the symbol
 */
void Parser::pushSymbol(Symbol* sym) {
	symStack.push(sym);
	// if could not find the goto, then there must be some error against the grammar
	if (analyseTable->LR1_Table.count(GOTO(staStack.top(), *sym)) == 0) {
		outputError(string("gramma error, unexcepted stmbol ") + sym->content + " in line " + to_string(lineCount));
	}
	// get the corresponding behavior, and push the next status to the status stack
	Behavior bh = analyseTable->LR1_Table[GOTO(staStack.top(), *sym)];
	staStack.push(bh.nextStat);
}

/**
 * @brief output the symbol stack to the screen or file
 * @param out 
 */
void Parser::outputSymbolStack(ostream& out) {
	stack<Symbol*>temp = symStack;
	stack<Symbol*>other;
	while (!temp.empty()) {
		other.push(temp.top());
		temp.pop();
	}
	while (!other.empty()) {
		out << other.top()->content << " ";
		other.pop();
	}
	out << endl;
}

/**
 * @brief output the status stack to the screen or file
 * @param out 
 */
void Parser::outputStateStack(ostream& out) {
	stack<int>temp = staStack;
	stack<int>other;
	while (!temp.empty()) {
		other.push(temp.top());
		temp.pop();
	}
	while (!other.empty()) {
		out << other.top() << " ";
		other.pop();
	}
	out << endl;

}

/**
 * @brief analyse the token list and generate the intermediate code
 * @param tokens 
 * @param out 
 */
void Parser::analyse(list<Token>&tokens, ostream& out) {
	// if accpted or not
	bool acc = false;
	// initialize the symbol stack and status stack
	symStack.push(new Symbol(true, "#"));
	staStack.push(0);
	// the main loop, traverse the token list
	for (list<Token>::iterator iter = tokens.begin(); iter != tokens.end(); ) {
		outputSymbolStack(out);
		outputStateStack(out);
		TokenType LT = iter->getType();
		string token = iter->getValue();

		// if the token is a comment, ignore it
		if (LT == LCOMMENT || LT == PCOMMENT) {
			continue;
		}

		// if the token is a nextline, ignore it and add the line count
		if (LT == NEXTLINE) {
			lineCount++;
			continue;
		}

		// if the token is a identifier or a number, then create a symbol with the token
		Symbol* nextSymbol;
		if (LT == ID) {
			nextSymbol = new Id(Symbol{ true,"ID" }, token);
		}
		else if (LT == NUM) {
			nextSymbol = new Num(Symbol{ true,"NUM" }, token);
		}
		else {
			nextSymbol = new Symbol(true, token);
		}

		// if could not find the goto, then there must be some error against the grammar
		if (analyseTable->LR1_Table.count(GOTO(staStack.top(), *nextSymbol)) == 0) {
			outputError(string("gramma error, unexcepted stmbol ") + nextSymbol->content + " in line " + to_string(lineCount));
		}

		// get the behavior with from the goto table
		Behavior bh = analyseTable->LR1_Table[GOTO(staStack.top(), *nextSymbol)];
		// if the behavior is shift, then push the symbol and the next status to the stack
		if (bh.behavior == shift) {
			symStack.push(nextSymbol);
			staStack.push(bh.nextStat);
			iter++;
		}
		// if the behavior is reduct
		else if (bh.behavior == reduct) {
			// get the production to be reducted
			Production reductPro = analyseTable->productions[bh.nextStat];
			// get the number of symbols to be poped
			int popSymNum = reductPro.right.size();
			// reduce the symbol stack, and generate the intermediate code
			switch (bh.nextStat) {
				/**
				 * @brief function declare
				 * @details declare ::= int ID M A function_declare
				 */
				case 3:
				{
					// pop the symbols
					FunctionDeclare *function_declare = (FunctionDeclare*)popSymbol();
					Symbol* A = popSymbol();
					M* m = (M*)popSymbol();
					Id* ID = (Id*)popSymbol();
					Symbol* _int = popSymbol();
					// add the function to the function table, use m to record the enter point
					funcTable.push_back(Func{ ID->name,D_INT,function_declare->plist,m->quad });
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
				/**
				 * @brief variable declare
				 * @details declare ::= int ID var_declare
				 */
				case 4:
				{
					Symbol* var_declare = popSymbol();
					Id* ID = (Id*)popSymbol();
					Symbol* _int = popSymbol();
					// add the variable to the variable table, and record the level
					varTable.push_back(Var{ ID->name,D_INT,nowLevel });
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
				/**
				 * @brief function declare
				 * @details declare ::= void ID M A function_declare
				 */
				case 5: //declare ::= void ID M A function_declare
				{
					FunctionDeclare* function_declare = (FunctionDeclare*)popSymbol();
					Symbol* A = popSymbol();
					M* m = (M*)popSymbol();
					Id* ID = (Id*)popSymbol();
					Symbol* _void = popSymbol();
					funcTable.push_back(Func{ ID->name, D_VOID, function_declare->plist,m->quad });
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
				/**
				 * @brief add the now level, using symbol A to control the level
				 * @details A ::=
				 */
				case 6: //A ::=
				{
					nowLevel++;
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
				/**
				 * @brief function declare
				 * @details function_declare ::= ( parameter ) sentence_block
				 */
				case 8:
				{
					SentenceBlock* sentence_block = (SentenceBlock*)popSymbol();
					Symbol* rparen = popSymbol();
					Parameter* paramter = (Parameter*)popSymbol();
					Symbol* lparen = popSymbol();
					FunctionDeclare* function_declare = new FunctionDeclare(reductPro.left);
					function_declare->plist.assign(paramter->plist.begin(), paramter->plist.end());
					pushSymbol(function_declare);
					break;
				}
				/**
				 * @brief parameter list
				 * @details parameter :: = parameter_list
				 */
				case 9: //parameter :: = parameter_list
				{
					ParameterList* parameter_list = (ParameterList*)popSymbol();
					Parameter *parameter = new Parameter(reductPro.left);
					parameter->plist.assign(parameter_list->plist.begin(), parameter_list->plist.end());
					pushSymbol(parameter);
					break;
				}
				/**
				 * @brief void parameter 
				 * @details parameter ::= void
				 */
				case 10:
				{
					Symbol* _void = popSymbol();
					Parameter* parameter = new Parameter(reductPro.left);
					pushSymbol(parameter);
					break;
				}
				/**
				 * @brief parameter list
				 * @details parameter_list ::= param
				 */
				case 11:
				{
					Symbol* param = popSymbol();
					ParameterList* parameter_list = new ParameterList(reductPro.left);
					parameter_list->plist.push_back(D_INT);
					pushSymbol(parameter_list);
					break;
				}
				/**
				 * @brief parameter list
				 * @details parameter_list ::= param , parameter_list
				 */
				case 12:
				{
					ParameterList* parameter_list2 = (ParameterList*)popSymbol();
					Symbol* comma = popSymbol();
					Symbol* param = popSymbol();
					ParameterList *parameter_list1 = new ParameterList(reductPro.left);
					parameter_list2->plist.push_front(D_INT);
					parameter_list1->plist.assign(parameter_list2->plist.begin(), parameter_list2->plist.end());
					pushSymbol(parameter_list1);
					break;
				}
				/**
				 * @brief int parameter
				 * @details param ::= int ID
				 */
				case 13:
				{
					Id* ID = (Id*)popSymbol();
					Symbol* _int = popSymbol();
					varTable.push_back(Var{ ID->name,D_INT,nowLevel });
					code.emit("get", "_", "_", ID->name);
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
				/**
				 * @brief sentence block end, pop the variable not in the current level
				 * @details sentence_block ::= { inner_declare sentence_list }
				 */
				case 14: 
				{
					Symbol* rbrace = popSymbol();
					SentenceList* sentence_list = (SentenceList*)popSymbol();
					Symbol* inner_declare = popSymbol();
					Symbol* lbrace = popSymbol();
					SentenceBlock* sentence_block = new SentenceBlock(reductPro.left);
					sentence_block->nextList = sentence_list->nextList;
					// now level decrease
					nowLevel--;
					// pop the variable not in the current level, local variable
					int popNum = 0;
					for (vector<Var>::reverse_iterator riter = varTable.rbegin(); riter != varTable.rend(); riter++) {
						if (riter->level > nowLevel)
							popNum++;
						else
							break;
					}
					// we cannot pop the table while traversing it, so we use a temp vector to store the variable to be poped
					for (int i = 0; i < popNum; i++) {
						varTable.pop_back();
					}
					pushSymbol(sentence_block);
					break;
				}
				/**
				 * @brief inner variable declare
				 * @details inner_var_declare ::= int ID
				 */
				case 17:
				{
					Id* ID = (Id*)popSymbol();
					Symbol* _int = popSymbol();
					pushSymbol(new Symbol(reductPro.left));
					varTable.push_back(Var{ ID->name,D_INT,nowLevel });
					break;
				}
				/**
				 * @brief sentence list
				 * @details sentence_list ::= sentence M sentence_list
				 */
				case 18:
				{
					SentenceList* sentence_list2 = (SentenceList*)popSymbol();
					M* m = (M*)popSymbol();
					Sentence* sentence = (Sentence*)popSymbol();
					SentenceList* sentence_list1 = new SentenceList(reductPro.left);
					sentence_list1->nextList = sentence_list2->nextList;
					// back patch the next list of the sentence
					code.back_patch(sentence->nextList, m->quad);
					pushSymbol(sentence_list1);
					break;
				}
				/**
				 * @brief sentence list
				 * @details sentence_list ::= sentence
				 */
				case 19:
				{
					Sentence* sentence = (Sentence*)popSymbol();
					SentenceList* sentence_list = new SentenceList(reductPro.left);
					sentence_list->nextList = sentence->nextList;
					pushSymbol(sentence_list);
					break;
				}
				/**
				 * @brief sentence
				 * @details sentence ::= if_sentence
				 */
				case 20:
				{
					IfSentence* if_sentence = (IfSentence*)popSymbol();
					Sentence* sentence = new Sentence(reductPro.left);
					sentence->nextList = if_sentence->nextList;
					pushSymbol(sentence);
					break;
				}
				/**
				 * @brief sentence
				 * @details sentence ::= while_sentence
				 */
				case 21: 
				{
					WhileSentence* while_sentence = (WhileSentence*)popSymbol();
					Sentence* sentence = new Sentence(reductPro.left);
					sentence->nextList = while_sentence->nextList;
					pushSymbol(sentence);
					break;
				}
				/**
				 * @brief sentence
				 * @details sentence ::= return_sentence
				 */
				case 22:
				{
					Symbol* return_sentence = popSymbol();
					Sentence* sentence = new Sentence(reductPro.left);
					pushSymbol(sentence);
					break;
				}
				/**
				 * @brief sentence
				 * @details sentence ::= assign_sentence
				 */
				case 23:
				{
					Symbol* assign_sentence = popSymbol();
					Sentence* sentence = new Sentence(reductPro.left);
					pushSymbol(sentence);
					break;
				}
				/**
				 * @brief assign_sentence
				 * @details assign_sentence ::= ID = expression ;
				 */
				case 24:
				{
					Symbol* comma = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* assign = popSymbol();
					Id* ID = (Id*)popSymbol();
					Symbol* assign_sentence = new Symbol(reductPro.left);
					// emit the intermediate code
					code.emit("=", expression->name, "_", ID->name);
					pushSymbol(assign_sentence);
					break;
				}
				/**
				 * @brief return sentence
				 * @details return_sentence ::= return ;
				 */
				case 25:
				{
					Symbol* comma = popSymbol();
					Symbol* _return = popSymbol();
					code.emit("return", "_", "_", "_");
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
				/**
				 * @brief return sentence
				 * @details return_sentence ::= return expression ;
				 */
				case 26:
				{
					Symbol* comma = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* _return = popSymbol();
					code.emit("return", expression->name, "_", "_");
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
				/**
				 * @brief while sentence
				 * @details while_sentence ::= while M ( expression ) A sentence_block
				 */
				case 27:
				{
					SentenceBlock* sentence_block = (SentenceBlock*)popSymbol();
					Symbol* A = popSymbol();
					Symbol* rparen = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* lparen = popSymbol();
					M* m = (M*)popSymbol();
					Symbol* _while = popSymbol();
					WhileSentence* while_sentence = new WhileSentence(reductPro.left);
					code.back_patch(sentence_block->nextList, m->quad);
					while_sentence->nextList = expression->falseList;
					// generate the intermediate code to jump to the while sentence
					code.emit("j", "_", "_", to_string(m->quad));
					pushSymbol(while_sentence);
					break;
				}
				/**
				 * @brief if sentence
				 * @details if_sentence ::= if ( expression ) A sentence_block
				 */
				case 28:
				{
					SentenceBlock* sentence_block = (SentenceBlock*)popSymbol();
					Symbol* A = popSymbol();
					Symbol* rparen = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* lparen = popSymbol();
					Symbol* _if = popSymbol();
					IfSentence* if_sentence = new IfSentence(reductPro.left);
					expression->falseList.splice(expression->falseList.begin(), sentence_block->nextList);
					if_sentence->nextList = expression->falseList;
					pushSymbol(if_sentence);
					break;
				}
				/**
				 * @brief if sentence
				 * @details if_sentence ::= if ( expression ) A1 sentence_block1 N else M A2 sentence_block2
				 */
				case 29:
				{
					SentenceBlock* sentence_block2 = (SentenceBlock*)popSymbol();
					Symbol* A2 = popSymbol();
					M* m = (M*)popSymbol();
					Symbol* _else = popSymbol();
					N* n = (N*)popSymbol();
					SentenceBlock* sentence_block1 = (SentenceBlock*)popSymbol();
					Symbol* A1 = popSymbol();
					Symbol* rparen = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* lparen = popSymbol();
					Symbol* _if = popSymbol();
					IfSentence* if_sentence = new IfSentence(reductPro.left);
					code.back_patch(expression->falseList, m->quad);
					if_sentence->nextList = merge(sentence_block1->nextList, sentence_block2->nextList);
					if_sentence->nextList = merge(if_sentence->nextList, n->nextList);
					pushSymbol(if_sentence);
					break;
				}
				/**
				 * @brief N ::=, control the transfer of the if sentence
				 * @details N ::= 
				 */
				case 30:
				{
					N* n = new N(reductPro.left);
					n->nextList.push_back(code.nextQuad());
					code.emit("j", "_", "_", "-1");
					pushSymbol(n);
					break;
				}
				/**
				 * @brief M ::=, control the transfer of the while sentence
				 * @details M ::= 
				 */
				case 31: 
				{
					M* m = new M(reductPro.left);
					m->quad = code.nextQuad();
					pushSymbol(m);
					break;
				}
				/**
				 * @brief expression
				 * @details expression ::= add_expression
				 */
				case 32: 
				{
					AddExpression* add_expression = (AddExpression*)popSymbol();
					Expression* expression = new Expression(reductPro.left);
					expression->name = add_expression->name;
					pushSymbol(expression);
					break;
				}
				/**
				 * @brief expression
				 * @details expression ::= add_expression1 > add_expression2
				 */
				case 33: 
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* gt = popSymbol();
					AddExpression* add_expression1 = (AddExpression*)popSymbol();
					Expression* expression = new Expression(reductPro.left);
					expression->falseList.push_back(code.nextQuad());
					code.emit("j<=", add_expression1->name, add_expression2->name, "-1");
					pushSymbol(expression);
					break;
				}
				/**
				 * @brief expression
				 * @details expression ::= add_expression1 < add_expression2
				 */
				case 34:
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* lt = popSymbol();
					AddExpression* add_expression1 = (AddExpression*)popSymbol();
					Expression* expression = new Expression(reductPro.left);
					expression->falseList.push_back(code.nextQuad());
					code.emit("j>=", add_expression1->name, add_expression2->name, "-1");
					pushSymbol(expression);
					break;
				}
				/**
				 * @brief expression
				 * @details expression ::= add_expression1 == add_expression2
				 */
				case 35: 
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol *eq = popSymbol();
					AddExpression *add_expression1 = (AddExpression*)popSymbol();
					Expression *expression = new Expression(reductPro.left);
					expression->falseList.push_back(code.nextQuad());
					code.emit("j!=", add_expression1->name, add_expression2->name, "-1");
					pushSymbol(expression);
					break;
				}
				/**
				 * @brief expression
				 * @details expression ::= add_expression1 >= add_expression2
				 */
				case 36: 
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* get = popSymbol();
					AddExpression* add_expression1 = (AddExpression*)popSymbol();
					Expression* expression = new Expression(reductPro.left);
					expression->falseList.push_back(code.nextQuad());
					code.emit("j<", add_expression1->name, add_expression2->name, "-1");
					pushSymbol(expression);
					break;
				}
				/**
				 * @brief expression
				 * @details expression ::= add_expression1 <= add_expression2
				 */
				case 37:
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* let = popSymbol();
					AddExpression* add_expression1 = (AddExpression*)popSymbol();
					Expression* expression = new Expression(reductPro.left);
					expression->falseList.push_back(code.nextQuad());
					code.emit("j>", add_expression1->name, add_expression2->name, "-1");
					pushSymbol(expression);
					break;
				}
				/**
				 * @brief expression
				 * @details expression ::= add_expression1 != add_expression2
				 */
				case 38:
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* neq = popSymbol();
					AddExpression* add_expression1 = (AddExpression*)popSymbol();
					Expression* expression = new Expression(reductPro.left);
					expression->falseList.push_back(code.nextQuad());
					code.emit("j==", add_expression1->name, add_expression2->name, "-1");
					pushSymbol(expression);
					break;
				}
				/**
				 * @brief add expression
				 * @details add_expression ::= item
				 */
				case 39:
				{
					Nomial* item = (Nomial*)popSymbol();
					AddExpression* add_expression = new AddExpression(reductPro.left);
					add_expression->name = item->name;
					pushSymbol(add_expression);
					break;
				}
				/**
				 * @brief add expression
				 * @details add_expression1 ::= item + add_expression2
				 */
				case 40:
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* add = popSymbol();
					Nomial* item = (Nomial*)popSymbol();
					AddExpression* add_expression1 = new AddExpression(reductPro.left);
					add_expression1->name = code.newTemp();
					code.emit("+", item->name, add_expression2->name, add_expression1->name);
					pushSymbol(add_expression1);
					break;
				}
				/**
				 * @brief add expression
				 * @details add_expression1 ::= item - add_expression2
				 */
				case 41:
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* sub = popSymbol();
					Nomial* item = (Nomial*)popSymbol();
					AddExpression* add_expression1 = new AddExpression(reductPro.left);
					add_expression1->name = code.newTemp();
					code.emit("-", item->name, add_expression2->name, add_expression1->name);
					pushSymbol(add_expression1);
					break;
				}
				/**
				 * @brief item
				 * @details item ::= factor
				 */
				case 42: 
				{
					Factor* factor = (Factor*)popSymbol();
					Nomial* item = new Nomial(reductPro.left);
					item->name = factor->name;
					pushSymbol(item);
					break;
				}
				/**
				 * @brief item
				 * @details item1 ::= factor * item2
				 */
				case 43:
				{
					Nomial* item2 = (Nomial*)popSymbol();
					Symbol* mul = popSymbol();
					Factor* factor = (Factor*)popSymbol();
					Nomial* item1 = new Nomial(reductPro.left);
					item1->name = code.newTemp();
					code.emit("*", factor->name, item2->name, item1->name);
					pushSymbol(item1);
					break;
				}
				/**
				 * @brief item
				 * @details item1 ::= factor / item2
				 */
				case 44:
				{
					Nomial* item2 = (Nomial*)popSymbol();
					Symbol* div = popSymbol();
					Factor* factor = (Factor*)popSymbol();
					Nomial* item1 = new Nomial(reductPro.left);
					item1->name = code.newTemp();
					code.emit("/", factor->name, item2->name, item1->name);
					pushSymbol(item1);
					break;
				}
				/**
				 * @brief factor
				 * @details factor ::= NUM
				 */
				case 45:
				{
					Num* num = (Num*)popSymbol();
					Factor* factor = new Factor(reductPro.left);
					factor->name = num->number;
					pushSymbol(factor);
					break;
				}
				/**
				 * @brief factor
				 * @details factor ::= ( expression )
				 */
				case 46: 
				{
					Symbol* rparen = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* lparen = popSymbol();
					Factor* factor = new Factor(reductPro.left);
					factor->name = expression->name;
					pushSymbol(factor);
					break;
				}
				/**
				 * @brief factor, call the function
				 * @details factor ::= ID ( argument_list )
				 */
				case 47: 
				{
					Symbol* rparen = popSymbol();
					ArgumentList* argument_list = (ArgumentList*)popSymbol();
					Symbol* lparen = popSymbol();
					Id* ID = (Id*)popSymbol();
					Factor* factor = new Factor(reductPro.left);
					Func* f = lookUpFunc(ID->name);
					// check if the function is declared and the input parameter is correct
					if (!f) {
						outputError(string("gramma error, undeclared function")+ ID->name + "in line" + to_string(lineCount));
					}
					else if (!march(argument_list->alist, f->paramTypes)) {
						outputError(string("gramma error, inputed parameter do not match with decleration of function") + ID->name + "in line" + to_string(lineCount));
					}
					else {
						// generate the intermediate code, indicate the parameters
						for (list<string>::iterator iter = argument_list->alist.begin(); iter != argument_list->alist.end(); iter++) {
							code.emit("par", *iter, "_", "_");
						}
						factor->name = code.newTemp();
						// function call
						code.emit("call", ID->name,"_", "_");
						// get the return value of function
						code.emit("=", "@RETURN_PLACE", "_", factor->name);
						
						pushSymbol(factor);
					}
					break;
				}
				/**
				 * @brief factor, get the value of the variable
				 * @details factor ::= ID
				 */
				case 48:
				{
					Id* ID = (Id*)popSymbol();
					if (lookUpVar(ID->name) == NULL) {
						outputError(string("gramma error, undeclared variable")+ ID->name + "in line" + to_string(lineCount));
					}
					Factor* factor = new Factor(reductPro.left);
					factor->name = ID->name;
					pushSymbol(factor);
					break;
				}
				/**
				 * @brief argument list
				 * @details argument_list ::= 
				 */
				case 49: //argument_list ::= 
				{
					ArgumentList* argument_list = new ArgumentList(reductPro.left);
					pushSymbol(argument_list);
					break;
				}
				/**
				 * @brief argument list
				 * @details argument_list ::= expression
				 */
				case 50:
				{
					Expression* expression = (Expression*)popSymbol();
					ArgumentList* argument_list = new ArgumentList(reductPro.left);
					argument_list->alist.push_back(expression->name);
					pushSymbol(argument_list);
					break;
				}
				/**
				 * @brief argument list expand
				 * @details argument_list ::= expression , argument_list
				 */
				case 51:
				{
					ArgumentList* argument_list2 = (ArgumentList*)popSymbol();
					Symbol* comma = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					ArgumentList* argument_list1 = new ArgumentList(reductPro.left);
					argument_list2->alist.push_front(expression->name);
					argument_list1->alist.assign(argument_list2->alist.begin(),argument_list2->alist.end());
					pushSymbol(argument_list1);
					break;
				}
				/**
				 * @brief other production, just pop the symbol stack and push the left symbol
				 */
				default:
				{
					for (int i = 0; i < popSymNum; i++) {
						popSymbol();
					}
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
			}
		}
		/**
		 * @brief accept the input
		 * @details P ::= N declare_list
		 */
		else if (bh.behavior == accept) {
			acc = true;
			Func*f = lookUpFunc("main");
			popSymbol();
			N* n = (N*)popSymbol();
			code.back_patch(n->nextList, f->enterPoint);
			break;
		}
	}
	if (!acc) {
		outputError("gramma error: unexpected end of file");
	}
}

/**
 * @brief analyse the token list and generate the intermediate code
 * @param tokens 
 * @param fileName: the name of the file to output the intermediate code
 */
void Parser::analyse(list<Token>&tokens, const char* fileName) {
	ofstream fout;
	fout.open(fileName);
	if (!fout.is_open()) {
		outputError("fail to open file" + string(fileName));
	}
	analyse(tokens, fout);

	fout.close();
}

/**
 * @brief get the intermediate code
 * @return IntermediateCode* 
 */
IntermediateCode* Parser::getIntermediateCode() {
	return &code;
}