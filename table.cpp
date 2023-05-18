#include "table.h"

/**
 * @brief reload operator < for Item
 * @details compare the production id and the position of the point
 * @param one 
 * @param other 
 * @return true 
 * @return false 
 */
bool operator < (const Item&one, const Item& other) {
	return pair<int, int>(one.pro, one.pointPos) < pair<int, int>(other.pro, other.pointPos);
}

/**
 * @brief reload operator == for Item
 * @details compare the production id and the position of the point
 * @param one 
 * @param other 
 * @return true 
 * @return false 
 */
bool operator ==(const Item&one, const Item& other) {
	return (one.pro == other.pro) && (one.pointPos == other.pointPos);
}

/**
 * @brief read productions from file
 * @param fileName the name of the production
 */
void AnalyseTable::readProductions(const char*fileName) {
	// open file
	ifstream fin;
	fin.open(fileName, ios::in);
	if (!fin.is_open()) {
		outputError("fail to open file" + string(fileName));
	}

	// read productions
	int index = 0;			// production id
	char buf[1024];			// char buffer
	while (fin >> buf) {
		Production p;
		// assign production id
		p.id = index++;		

		//read left part of the production, it is a non-terminal symbol
		p.left = Symbol{ false,string(buf) };

		//there must be a "::=" in the middle
		fin >> buf;
		assert(strcmp(buf, "::=") == 0);

		//read other part of the line, consist of a series of symbols
		fin.getline(buf, 1024);
		stringstream sstream(buf);
		string temp;
		while (sstream >> temp) {
			// every symbol is separated by a space
			p.right.push_back(Symbol{ isVT(temp),string(temp) });
		}

		// add this production to the production list
		productions.push_back(p);
	}
	fin.close();
}


/**
 * @brief get the first set of each symbol
 */
void AnalyseTable::getFirst() {
	// if there are any change in the first set of each symbol
	bool updateFlag = true;
	// loop until the first set of each symbol does not change
	while (updateFlag) {
		updateFlag = false;
		//tranverse each production
		for (vector<Production>::iterator iter = productions.begin(); iter != productions.end(); iter++) {
			vector<Symbol>::iterator symbolIter;
			//tranverse each symbol in the right part of the production
			for (symbolIter = iter->right.begin(); symbolIter != iter->right.end(); symbolIter++) {
				//this symbol is a terminal symbol, add it to the first set of the left symbol
				if (symbolIter->isVt) {
					// if the first set of the left symbol has not initialed
					if (first.count(iter->left) == 0) {
						first[iter->left] = set<Symbol>();
					}
					// if the first set of the left symbol does not contain this terminal symbol
					if (first[iter->left].insert(*symbolIter).second == true) {
						updateFlag = true;
					}
					break;
				}
				// this symbol is a non-terminal symbol
				// if we need to consider the first set of the next symbol
				bool continueFlag = false;
				//tranverse the first set of the symbol
				for (set<Symbol>::iterator firstIter = first[*symbolIter].begin(); firstIter != first[*symbolIter].end(); firstIter++) {
					//if the first set of the symbol contains EMPTY, we need to consider first set of next symbol
					if (firstIter->content == "EMPTY") {
						continueFlag = true;
					}
					//if current symbol is not in the first set of the left symbol, add it to the first set of the left symbol
					else if (first[iter->left].find(*firstIter) == first[iter->left].end()) {
						if (first.count(iter->left) == 0) {
							first[iter->left] = set<Symbol>();
						}
						first[iter->left].insert(*firstIter);
						updateFlag = true;
					}
				}
				// if the first set of the symbol does not contain EMPTY, we do not need to consider the first set of the next symbol
				if (!continueFlag) {
					break;
				}
			}
			// EMPTY is contained in the first set of all symbols in the right part of the production
			// then add EMPTY to the first set of the left symbol
			if (symbolIter == iter->right.end()) {
				if (first.count(iter->left) == 0) {
					first[iter->left] = set<Symbol>();
				}
				if (first[iter->left].insert(Symbol{ true,"EMPTY" }).second == true) {
					updateFlag = true;
				}
			}
		}

	}
}

/**
 * @brief get the follow set of each non-terminal symbol
 */
void AnalyseTable::getFollow() {
	// add # to the follow set of the start symbol
	follow[productions[0].left] = set<Symbol>();
	follow[productions[0].left].insert(Symbol{ true,"#" });
	// if there are any change in the follow set of each non-terminal symbol
	bool updateFlag = true;
	// tranverse until the follow set of each non-terminal symbol does not change
	while (updateFlag) {
		updateFlag = false;
		// tranverse each production
		for (vector<Production>::iterator proIter = productions.begin(); proIter != productions.end(); proIter++) {
			// tranverse each symbol in the right part of the production
			for (vector<Symbol>::iterator symbolIter = proIter->right.begin(); symbolIter != proIter->right.end(); symbolIter++) {
				// tranverse the symbol after the current symbol
				vector<Symbol>::iterator nextsymbolIter;
				for (nextsymbolIter = symbolIter + 1; nextsymbolIter != proIter->right.end(); nextsymbolIter++) {
					Symbol nextSym = *nextsymbolIter;
					// if the next symbol is a terminal symbol, add it to the follow set of the current symbol
					if (nextSym.isVt) {
						if (follow.count(*symbolIter) == 0) {
							follow[*symbolIter] = set<Symbol>();
						}
						// if there is update in the follow set of the current symbol
						// which means that the follow set of the current symbol does not contain this terminal symbol before this iteration
						if (follow[*symbolIter].insert(nextSym).second == true) {
							updateFlag = true;
						}
						break;
					}
					// the next symbol is a non-terminal symbol
					// tranverse the first set of the next symbol
					bool continueFlag = false;
					for (set<Symbol>::iterator fIter = first[nextSym].begin(); fIter != first[nextSym].end(); fIter++) {
						// if the first set of the next symbol contains EMPTY, we need to consider the first set of the next symbol
						if (fIter->content == "EMPTY") {
							continueFlag = true;
						}
						else {
							// add the symbol in the first set of the next symbol to the follow set of the current symbol
							if (follow.count(*symbolIter) == 0) {
								follow[*symbolIter] = set<Symbol>();
							}
							// if there is update in the follow set of the current symbol
							if (follow[*symbolIter].insert(*fIter).second == true) {
								updateFlag = true;
							}
						}
					}
					// if the first set of the next symbol does not contain EMPTY, we do not need to consider the first set of the next symbol
					if (!continueFlag) {
						break;
					}

				}
				// if EMPTY is in the first set of all symbols after the current symbol
				if (nextsymbolIter == proIter->right.end()) {
					// add the follow set of the left symbol of the production to the follow set of the current symbol
					for (set<Symbol>::iterator followIter = follow[proIter->left].begin(); followIter != follow[proIter->left].end(); followIter++) {
						if (follow.count(*symbolIter) == 0) {
							follow[*symbolIter] = set<Symbol>();
						}
						// update flag
						if (follow[*symbolIter].insert(*followIter).second == true) {
							updateFlag = true;
						}
					}
				}
			}
		}
	}
}

/**
 * @brief derive the closure of a item
 * @details derive the closure recursively
 * @param item 
 * @return status 
 */
status AnalyseTable::derive(Item item) {
	status i;
	// insert this item into the closure
	i.insert(item);
	// if the symbol after the point is a non-terminal symbol and the point is not at the end of the production
	// then add the productions whose left part is the symbol after the point to the closure
	if(	productions[item.pro].right.size() != item.pointPos &&
		productions[item.pro].right[item.pointPos].isVt == false){
		// tranverse all productions
		vector<Production>::iterator iter;
		for (iter = productions.begin(); iter < productions.end(); iter++) {
			// if the left part of the production is the symbol after the point
			if (iter->left == productions[item.pro].right[item.pointPos]) {
				// union the closure of this item to the closure of the parameter item
				status temp = derive(Item{ int(iter - productions.begin()),0 });
				set<Item>::iterator siter;
				for (siter = temp.begin(); siter != temp.end(); siter++) {
					i.insert(*siter);
				}
			}
		}
	}

	return i;
}

/**
 * @brief create DFA and LR1 table
 */
void AnalyseTable::createDFA() {
	// index of the status in the DFA
	int nowI = 0;
	// add the closure of the first production to the DFA
	dfa.stas.push_back(derive(Item{ 0,0 }));
	// tranverse each status in the DFA
	for (list<status>::iterator iter = dfa.stas.begin(); iter != dfa.stas.end(); iter++, nowI++) {
		// tranverse each item in the status
		for (set<Item>::iterator itIter = iter->begin(); itIter != iter->end(); itIter++) {
			// this is a reduct item
			if (productions[itIter->pro].right.size() == itIter->pointPos) {
				set<Symbol>FOLLOW = follow[productions[itIter->pro].left];
				// tranverse each symbol in the follow set of the left symbol of the production
				for (set<Symbol>::iterator followIter = FOLLOW.begin(); followIter != FOLLOW.end(); followIter++) {
					// exist conflict in LR1 table
					if (LR1_Table.count(GOTO(nowI, *followIter)) == 1) {
						string err = "is not LR(1) grammar, exist conflict in GOTO(";
						outputError(err);
					}
					// if it is the reduction of the start symbol, then it is accept
					// otherwise it is reduct
					LR1_Table[GOTO(nowI, *followIter)] = Behavior{ (itIter->pro == 0) ? accept : reduct,itIter->pro };

				}
				continue;
			}
			// this is a shift item
			// the symbol after the point
			Symbol nextSymbol = productions[itIter->pro].right[itIter->pointPos];

			// if there is already a GOTO[nowI,nextSymbol], then continue
			if (dfa.goTo.count(GOTO(nowI, nextSymbol)) == 1) {
				continue;
			}

			// otherwise ,create a new DFA status, derive the closure of the corresponding item
			status newI = derive(Item{ itIter->pro,itIter->pointPos + 1 });
			// tranverse each item except the current item in the status
			set<Item>::iterator shiftIter = itIter;
			shiftIter++;
			for (; shiftIter != iter->end(); shiftIter++) {
				// ignore the reduct item
				if (productions[shiftIter->pro].right.size() == shiftIter->pointPos) {
					continue;
				}
				// if the symbol after the point is the same as the symbol after the point of the current item
				// derive the closure of the corresponding item and union it to the new status
				else if (productions[shiftIter->pro].right[shiftIter->pointPos] == nextSymbol) {
					status tempI = derive(Item{ shiftIter->pro,shiftIter->pointPos + 1 });
					newI.insert(tempI.begin(), tempI.end());
				}
			}

			// if this new status is already in the DFA, then just add a new transfer
			bool searchFlag = false;
			int index = 0;
			for (list<status>::iterator iterHave = dfa.stas.begin(); iterHave != dfa.stas.end(); iterHave++, index++) {
				if (*iterHave == newI) {
					// add a new transfer, from nowI to index(newI), with symbol nextSymbol
					dfa.goTo[GOTO(nowI, nextSymbol)] = index;
					// if there is already a behavior in LR1 table, then there is conflict
					if (LR1_Table.count(GOTO(nowI, nextSymbol)) == 1) {
						outputError("confict transition");
					}
					// shift behavior
					LR1_Table[GOTO(nowI, nextSymbol)] = Behavior{ shift,index };
					searchFlag = true;
					break;
				}
			}

			// this new status is not in the DFA
			if (!searchFlag) {
				// create a new status in the DFA, and add a new transfer
				dfa.stas.push_back(newI);
				dfa.goTo[GOTO(nowI, nextSymbol)] = (int)dfa.stas.size() - 1;
				if (LR1_Table.count(GOTO(nowI, nextSymbol)) == 1) {
					outputError("confict");
				}
				LR1_Table[GOTO(nowI, nextSymbol)] = Behavior{ shift,int(dfa.stas.size() - 1) };
			}
		}
	}
}

/**
 * @brief construct analyse table from product.txt
 * @param fileName 
 */
AnalyseTable::AnalyseTable(const char*fileName) {
	readProductions(fileName);
	getFirst();
	getFollow();
	createDFA();
}

/**
 * @brief output the dfa to the screen or file
 * @param out: the output stream
 */
void AnalyseTable::outputDFA(ostream& out) {
	int nowI = 0;
	for (list<status>::iterator iter = dfa.stas.begin(); iter != dfa.stas.end(); iter++, nowI++) {
		out << "I" << nowI << "= [";
		for (set<Item>::iterator itIter = iter->begin(); itIter != iter->end(); itIter++) {
			out << "[";
			Production p = productions[itIter->pro];
			out << p.left.content << " -> ";
			for (vector<Symbol>::iterator symIter = p.right.begin(); symIter != p.right.end(); symIter++) {
				if (symIter - p.right.begin() == itIter->pointPos) {
					out << ". ";
				}
				out << symIter->content << " ";
			}
			if (p.right.size() == itIter->pointPos) {
				out << ". ";
			}
			out << "]";
		}
		out << "]" << endl << endl;
	}
}

/**
 * @brief output the analyse table to the file named fileName
 */
void AnalyseTable::outputDFA(const char* fileName) {
	ofstream fout;
	fout.open(fileName);
	if (!fout.is_open()) {
		outputError("fail to open file" + string(fileName));
	}
	outputDFA(fout);
	fout.close();
}