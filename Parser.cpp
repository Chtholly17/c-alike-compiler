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
				case 3: //declare ::= int ID M A function_declare
				{
					FunctionDeclare *function_declare = (FunctionDeclare*)popSymbol();
					Symbol* A = popSymbol();
					M* m = (M*)popSymbol();
					Id* ID = (Id*)popSymbol();
					Symbol* _int = popSymbol();
					funcTable.push_back(Func{ ID->name,D_INT,function_declare->plist,m->quad });
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
				case 4: //declare ::= int ID var_declare
				{
					Symbol* var_declare = popSymbol();
					Id* ID = (Id*)popSymbol();
					Symbol* _int = popSymbol();
					varTable.push_back(Var{ ID->name,D_INT,nowLevel });
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
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
				case 6: //A ::=
				{
					nowLevel++;
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
				case 8: //function_declare ::= ( parameter ) sentence_block
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
				case 9: //parameter :: = parameter_list
				{
					ParameterList* parameter_list = (ParameterList*)popSymbol();
					Parameter *parameter = new Parameter(reductPro.left);
					parameter->plist.assign(parameter_list->plist.begin(), parameter_list->plist.end());
					pushSymbol(parameter);
					break;
				}
				case 10: //parameter ::= void
				{
					Symbol* _void = popSymbol();
					Parameter* parameter = new Parameter(reductPro.left);
					pushSymbol(parameter);
					break;
				}
				case 11: //parameter_list ::= param
				{
					Symbol* param = popSymbol();
					ParameterList* parameter_list = new ParameterList(reductPro.left);
					parameter_list->plist.push_back(D_INT);
					pushSymbol(parameter_list);
					break;
				}
				case 12: //parameter_list1 ::= param , parameter_list2
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
				case 13: //param ::= int ID
				{
					Id* ID = (Id*)popSymbol();
					Symbol* _int = popSymbol();
					varTable.push_back(Var{ ID->name,D_INT,nowLevel });
					code.emit("get", "_", "_", ID->name);
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
				case 14: //sentence_block ::= { inner_declare sentence_list }
				{
					Symbol* rbrace = popSymbol();
					SentenceList* sentence_list = (SentenceList*)popSymbol();
					Symbol* inner_declare = popSymbol();
					Symbol* lbrace = popSymbol();
					SentenceBlock* sentence_block = new SentenceBlock(reductPro.left);
					sentence_block->nextList = sentence_list->nextList;
					nowLevel--;
					int popNum = 0;
					for (vector<Var>::reverse_iterator riter = varTable.rbegin(); riter != varTable.rend(); riter++) {
						if (riter->level > nowLevel)
							popNum++;
						else
							break;
					}
					for (int i = 0; i < popNum; i++) {
						varTable.pop_back();
					}
					pushSymbol(sentence_block);
					break;
				}
				case 17: //inner_var_declare ::= int ID
				{
					Id* ID = (Id*)popSymbol();
					Symbol* _int = popSymbol();
					pushSymbol(new Symbol(reductPro.left));
					varTable.push_back(Var{ ID->name,D_INT,nowLevel });
					break;
				}
				case 18: //sentence_list ::= sentence M sentence_list
				{
					SentenceList* sentence_list2 = (SentenceList*)popSymbol();
					M* m = (M*)popSymbol();
					Sentence* sentence = (Sentence*)popSymbol();
					SentenceList* sentence_list1 = new SentenceList(reductPro.left);
					sentence_list1->nextList = sentence_list2->nextList;
					code.back_patch(sentence->nextList, m->quad);
					pushSymbol(sentence_list1);
					break;
				}
				case 19: //sentence_list ::= sentence
				{
					Sentence* sentence = (Sentence*)popSymbol();
					SentenceList* sentence_list = new SentenceList(reductPro.left);
					sentence_list->nextList = sentence->nextList;
					pushSymbol(sentence_list);
					break;
				}
				case 20: //sentence ::= if_sentence
				{
					IfSentence* if_sentence = (IfSentence*)popSymbol();
					Sentence* sentence = new Sentence(reductPro.left);
					sentence->nextList = if_sentence->nextList;
					pushSymbol(sentence);
					break;
				}
				case 21: //sentence ::= while_sentence
				{
					WhileSentence* while_sentence = (WhileSentence*)popSymbol();
					Sentence* sentence = new Sentence(reductPro.left);
					sentence->nextList = while_sentence->nextList;
					pushSymbol(sentence);
					break;
				}
				case 22: //sentence ::= return_sentence
				{
					Symbol* return_sentence = popSymbol();
					Sentence* sentence = new Sentence(reductPro.left);
					pushSymbol(sentence);
					break;
				}
				case 23: //sentence ::= assign_sentence
				{
					Symbol* assign_sentence = popSymbol();
					Sentence* sentence = new Sentence(reductPro.left);
					pushSymbol(sentence);
					break;
				}
				case 24: //assign_sentence ::= ID = expression ;
				{
					Symbol* comma = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* assign = popSymbol();
					Id* ID = (Id*)popSymbol();
					Symbol* assign_sentence = new Symbol(reductPro.left);
					code.emit("=", expression->name, "_", ID->name);
					pushSymbol(assign_sentence);
					break;
				}
				case 25: //return_sentence ::= return ;
				{
					Symbol* comma = popSymbol();
					Symbol* _return = popSymbol();
					code.emit("return", "_", "_", "_");
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
				case 26: //return_sentence ::= return expression ;
				{
					Symbol* comma = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* _return = popSymbol();
					code.emit("return", expression->name, "_", "_");
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
				case 27: //while_sentence ::= while M ( expression ) A sentence_block
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
					code.emit("j", "_", "_", to_string(m->quad));
					pushSymbol(while_sentence);
					break;
				}
				case 28: //if_sentence ::= if ( expression ) A sentence_block
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
				case 29: //if_sentence ::= if ( expression ) A1 sentence_block1 N else M A2 sentence_block2
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
				case 30: //N ::= 
				{
					N* n = new N(reductPro.left);
					n->nextList.push_back(code.nextQuad());
					code.emit("j", "_", "_", "-1");
					pushSymbol(n);
					break;
				}
				case 31: //M ::=
				{
					M* m = new M(reductPro.left);
					m->quad = code.nextQuad();
					pushSymbol(m);
					break;
				}
				case 32: //expression ::= add_expression
				{
					AddExpression* add_expression = (AddExpression*)popSymbol();
					Expression* expression = new Expression(reductPro.left);
					expression->name = add_expression->name;
					pushSymbol(expression);
					break;
				}
				case 33: //expression ::= add_expression1 > add_expression2
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
				case 34: //expression ::= add_expression1 < add_expression2
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
				case 35: //expression ::= add_expression1 == add_expression2
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
				case 36: //expression ::= add_expression1 >= add_expression2
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
				case 37: //expression ::= add_expression1 <= add_expression2
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
				case 38: //expression ::= add_expression1 != add_expression2
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
				case 39: //add_expression ::= item
				{
					Nomial* item = (Nomial*)popSymbol();
					AddExpression* add_expression = new AddExpression(reductPro.left);
					add_expression->name = item->name;
					pushSymbol(add_expression);
					break;
				}
				case 40: //add_expression1 ::= item + add_expression2
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* add = popSymbol();
					Nomial* item = (Nomial*)popSymbol();
					AddExpression* add_expression1 = new AddExpression(reductPro.left);
					add_expression1->name = nt.newTemp();
					code.emit("+", item->name, add_expression2->name, add_expression1->name);
					pushSymbol(add_expression1);
					break;
				}
				case 41: //add_expression ::= item - add_expression
				{
					AddExpression* add_expression2 = (AddExpression*)popSymbol();
					Symbol* sub = popSymbol();
					Nomial* item = (Nomial*)popSymbol();
					AddExpression* add_expression1 = new AddExpression(reductPro.left);
					add_expression1->name = nt.newTemp();
					code.emit("-", item->name, add_expression2->name, add_expression1->name);
					pushSymbol(add_expression1);
					break;
				}
				case 42: //item ::= factor
				{
					Factor* factor = (Factor*)popSymbol();
					Nomial* item = new Nomial(reductPro.left);
					item->name = factor->name;
					pushSymbol(item);
					break;
				}
				case 43: //item1 ::= factor * item2
				{
					Nomial* item2 = (Nomial*)popSymbol();
					Symbol* mul = popSymbol();
					Factor* factor = (Factor*)popSymbol();
					Nomial* item1 = new Nomial(reductPro.left);
					item1->name = nt.newTemp();
					code.emit("*", factor->name, item2->name, item1->name);
					pushSymbol(item1);
					break;
				}
				case 44: //item1 ::= factor / item2
				{
					Nomial* item2 = (Nomial*)popSymbol();
					Symbol* div = popSymbol();
					Factor* factor = (Factor*)popSymbol();
					Nomial* item1 = new Nomial(reductPro.left);
					item1->name = nt.newTemp();
					code.emit("/", factor->name, item2->name, item1->name);
					pushSymbol(item1);
					break;
				}
				case 45: //factor ::= NUM
				{
					Num* num = (Num*)popSymbol();
					Factor* factor = new Factor(reductPro.left);
					factor->name = num->number;
					pushSymbol(factor);
					break;
				}
				case 46: //factor ::= ( expression )
				{
					Symbol* rparen = popSymbol();
					Expression* expression = (Expression*)popSymbol();
					Symbol* lparen = popSymbol();
					Factor* factor = new Factor(reductPro.left);
					factor->name = expression->name;
					pushSymbol(factor);
					break;
				}
				case 47: //factor ::= ID ( argument_list )
				{
					Symbol* rparen = popSymbol();
					ArgumentList* argument_list = (ArgumentList*)popSymbol();
					Symbol* lparen = popSymbol();
					Id* ID = (Id*)popSymbol();
					Factor* factor = new Factor(reductPro.left);
					Func* f = lookUpFunc(ID->name);
					if (!f) {
						outputError(string("gramma error, undeclared function")+ ID->name + "in line" + to_string(lineCount));
					}
					else if (!march(argument_list->alist, f->paramTypes)) {
						outputError(string("gramma error, inputed parameter do not match with decleration of function") + ID->name + "in line" + to_string(lineCount));
					}
					else {
						for (list<string>::iterator iter = argument_list->alist.begin(); iter != argument_list->alist.end(); iter++) {
							code.emit("par", *iter, "_", "_");
						}
						factor->name = nt.newTemp();
						code.emit("call", ID->name,"_", "_");
						code.emit("=", "@RETURN_PLACE", "_", factor->name);
						
						pushSymbol(factor);
					}
					break;
				}
				case 48: //factor ::= ID
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
				case 49: //argument_list ::= 
				{
					ArgumentList* argument_list = new ArgumentList(reductPro.left);
					pushSymbol(argument_list);
					break;
				}
				case 50: //argument_list ::= expression
				{
					Expression* expression = (Expression*)popSymbol();
					ArgumentList* argument_list = new ArgumentList(reductPro.left);
					argument_list->alist.push_back(expression->name);
					pushSymbol(argument_list);
					break;
				}
				case 51: //argument_list1 ::= expression , argument_list2
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
				default: // other production, just pop the symbol stack and push the left symbol
				{
					for (int i = 0; i < popSymNum; i++) {
						popSymbol();
					}
					pushSymbol(new Symbol(reductPro.left));
					break;
				}
			}
		}
		else if (bh.behavior == accept) {//P ::= N declare_list
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