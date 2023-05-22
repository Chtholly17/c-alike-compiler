#include "ObjectCodeGenerator.h"

/**
 * @brief Construct a new Var Infomation:: Var Infomation object
 * @param next 
 * @param active 
 */
VarInfomation::VarInfomation(int next, bool active): next(next), active(active) {};

/**
 * @brief Construct a new Var Infomation:: Var Infomation object
 * @param other 
 */
VarInfomation::VarInfomation(const VarInfomation&other): next(other.next), active(other.active) {}

/**
 * @brief Construct a new Var Infomation:: Var Infomation object
 */
VarInfomation::VarInfomation(): next(-1), active(false) {}

/**
 * @brief output the information
 * @param out 
 */
void VarInfomation::output(ostream& out) {
	out << "(";
	if (next == -1)
		out << "^";
	else
		out << next;
	out << ",";
	if (active)
		out << "y";
	else
		out << "^";

	out << ")";
}

/**
 * @brief Construct a new Quaternary With Info:: Quaternary With Info object
 * @param q 
 * @param info1 
 * @param info2 
 * @param info3 
 */
QuaternaryWithInfo::QuaternaryWithInfo(Quaternary q, VarInfomation info1, VarInfomation info2, VarInfomation info3):
	q(q), info1(info1), info2(info2), info3(info3) {}

/**
 * @brief output the information
 * @param out 
 */
void QuaternaryWithInfo::output(ostream& out) {
	out << "(" << q.op << "," << q.src1 << "," << q.src2 << "," << q.des << ")";
	info1.output(out);
	info2.output(out);
	info3.output(out);
}

ObjectCodeGenerator::ObjectCodeGenerator() {}

/**
 * @brief store a register in a location
 * @param reg: the register which stores the variable
 * @param var: the variable name
 * @details generating the code: sw $reg offset($sp), offset = varOffset[var]
 */
void ObjectCodeGenerator::storeVar(string reg, string var) {
	// there is already space for var in the stack
	if (varOffset.find(var) != varOffset.end()) {
		objectCodes.push_back(string("sw ") + reg + " " + to_string(varOffset[var]) + "($sp)");
	}
	// there is no space for var in the stack, store
	else {
		varOffset[var] = top;
		top += 4;
		objectCodes.push_back(string("sw ") + reg + " " + to_string(varOffset[var]) + "($sp)");
	}
	// the Avalue of var contains itself and the register
	Avalue[var].insert(var);
}

/**
 * @brief release a variable from a register
 * @param var: the variable name, indicating a memory location
 */
void ObjectCodeGenerator::releaseVar(string var) {
	// for all the registers which store var, release var from the register
	for (set<string>::iterator iter = Avalue[var].begin(); iter != Avalue[var].end(); iter++) {
		// if the register is a real register, not a the variable itself
		if ((*iter)[0] == '$') {
			Rvalue[*iter].erase(var);
			// if the register is empty, add it to the free register list
			if (Rvalue[*iter].size() == 0 && (*iter)[1] == 's') {
				freeReg.push_back(*iter);
			}
		}
	}
	// clear the Rvalue of var, because var is not in any register now
	Avalue[var].clear();
}

/**
 * @brief select a register for a variable
 * @return string: the register name
 */
string ObjectCodeGenerator::selectReg() {
	// if there is a free register, return it
	string ret;
	if (freeReg.size()) {
		ret = freeReg.back();
		freeReg.pop_back();
		return ret;
	}

	/**
	 * @details select a register following the rules:
	 * 		1. if all the variable is also stored in other places, select the register directly
	 * 		2. otherwise, select the register which will be used latest
	 */
	int maxNextPos = 0;
	// traverse all the registers
	for (map<string, set<string> >::iterator iter = Rvalue.begin(); iter != Rvalue.end(); iter++) {
		int nextpos = INT32_MAX;
		// traverse all the variables stored in the register
		for (set<string>::iterator viter = iter->second.begin(); viter != iter->second.end(); viter++) {
			// if the variable is not stored in other places
			bool inFlag = false;
			// traverse all the places where these variable is stored
			for (set<string>::iterator aiter = Avalue[*viter].begin(); aiter != Avalue[*viter].end(); aiter++) {
				// if the variable is not only stored in this register, set inFlag to true
				if (*aiter != iter->first) {
					inFlag = true;
					break;
				}
			}
			// if the variable is only stored in this register, see where it will be used in the future
			if (!inFlag) {
				// traverse all the quaternaries in the current basic block
				for (vector<QuaternaryWithInfo>::iterator cIter = nowQuatenary; cIter != nowIBlock->codes.end(); cIter++) {
					// if the variable is used in the future, set nextpos to the position of the quaternary
					if (*viter == cIter->q.src1 || *viter == cIter->q.src2) {
						nextpos = cIter - nowQuatenary;
					}
					// if the variable is reasigned in the future, break
					else if (*viter == cIter->q.des) {
						break;
					}
				}
			}
		}
		// if the variable is not used in the future, select correspond register directly
		if (nextpos == INT32_MAX) {
			ret = iter->first;
			break;
		}
		// if this variable updates the latest, select correspond register
		else if (nextpos > maxNextPos) {
			maxNextPos = nextpos;
			ret = iter->first;
		}
	}

	/**
	 * @brief for all the variables stored in the selected register, release them from the register
	 */
	for (set<string>::iterator iter = Rvalue[ret].begin(); iter != Rvalue[ret].end(); iter++) {
		// realse the selected register from Avalue of the variables stored in it
		Avalue[*iter].erase(ret);
		// if these variables are also stored in other places, we do not need to write them back in the memory
		if (Avalue[*iter].size() > 0) {
			continue;
		}
		// if the variable is not used in the future, we do not need to write it back in the memory
		// if the variable is not used in the future or not 
		bool storeFlag = true;
		// traverse all the quaternaries in the current basic block
		vector<QuaternaryWithInfo>::iterator cIter;
		for (cIter = nowQuatenary; cIter != nowIBlock->codes.end(); cIter++) {
			// if the variable is used in the current Basic Block, set storeFlag to true, we need to write it back in the memory
			if (cIter->q.src1 == *iter || cIter->q.src2 == *iter) {
				storeFlag = true;
				break;
			}
			// if the variable is reasigned in the future, set storeFlag to false, we do not need to write it back in the memory
			if (cIter->q.des == *iter) {
				storeFlag = false;
				break;
			}
		}
		// if the variable is not used and not reasigned in the current Basic Block
		if (cIter == nowIBlock->codes.end()) {
			// we need to see whether the variable is out live
			int index = nowIBlock - funcIBlocks[nowFunc].begin();
			// if the variable is out live, we need to write it back in the memory
			if (funcOUTL[nowFunc][index].count(*iter) == 1) {
				storeFlag = true;
			}
			else {
				storeFlag = false;
			}
		}
		// store the reister back in the memory
		if (storeFlag) {
			storeVar(ret, *iter);
		}
	}
	// clear the Rvalue of the selected register, no variable is stored in it now
	Rvalue[ret].clear();
	return ret;
}

/**
 * @brief allocate a register for a variable
 * @param var: the variable name
 * @return string: the allocated register name
 */
string ObjectCodeGenerator::allocateReg(string var) {
	// if the variable is not in the memory(immidiate number), store it in a register directly
	if (isNum(var)) {
		string ret = selectReg();
		objectCodes.push_back(string("addi ") + ret + " $zero " + var);
		return ret;
	}

	// if the variable already has a register, return the register
	for (set<string>::iterator iter = Avalue[var].begin(); iter != Avalue[var].end(); iter++) {
		// if the variable is stored in a register
		if ((*iter)[0] == '$') {
			return *iter;
		}
	}

	// otherwise, select a register for the variable
	string ret = selectReg();
	// load the variable from the memory
	objectCodes.push_back(string("lw ") + ret + " " + to_string(varOffset[var]) + "($sp)");
	// update the Avalue and Rvalue
	Avalue[var].insert(ret);
	Rvalue[ret].insert(var);
	return ret;
}

/**
 * @brief allocate a register for the destination operand of the current quaternary
 * @details select the register following the rules:
 * 		1. if the source operand 1 is a variable, and it is only stored in a register, allocate the register to the destination operand
 * 		2. otherwise, realloc a register for the destination operand
 */
string ObjectCodeGenerator::getReg() {
	// if source operand 1 a variable
	if (!isNum(nowQuatenary->q.src1)) {
		// traverse all the registers that store the source operand 1
		set<string>&src1pos = Avalue[nowQuatenary->q.src1];
		for (set<string>::iterator iter = src1pos.begin(); iter != src1pos.end(); iter++) {
			if ((*iter)[0] == '$') {
				// if this register only stores the source operand 1, allocate it to the destination operand
				if (Rvalue[*iter].size() == 1) {
					if (nowQuatenary->q.des == nowQuatenary->q.src1 || !nowQuatenary->info1.active) {
						Avalue[nowQuatenary->q.des].insert(*iter);
						Rvalue[*iter].insert(nowQuatenary->q.des);
						return *iter;
					}
				}
			}
		}
	}

	// allocate a register for the destination operand
	string ret = selectReg();
	Avalue[nowQuatenary->q.des].insert(ret);
	Rvalue[ret].insert(nowQuatenary->q.des);
	return ret;
}

/**
 * @brief analyse the basic blocks of each function
 * @param funcBlocks: the basic blocks of each function, we can get the intermediate code of each function from it
 */
void ObjectCodeGenerator::analyseBlock(map<string, vector<Block> >*funcBlocks) {
	// traverse all the functions
	for (map<string, vector<Block> >::iterator fbiter = funcBlocks->begin(); fbiter != funcBlocks->end(); fbiter++) {
		vector<BlockWithInfo> iBlocks;
		vector<Block>& blocks = fbiter->second;
		// outlive, inlive, def, use
		// outlive: the out live variables of each basic block
		// inlive: the in live variables of each basic block
		// def: the variables defined in each basic block
		// use: the variables used in each basic block
		vector<set<string> >INL, OUTL, DEF, USE;

		// find the DEF and USE set of each basic block
		// traverse all the basic blocks of this function
		for (vector<Block>::iterator biter = blocks.begin(); biter != blocks.end(); biter++) {
			set<string>def, use;
			// traverse all the quaternaries in the current basic block
			for (vector<Quaternary>::iterator citer = biter->codes.begin(); citer != biter->codes.end(); citer++) {
				// if the quaternary is a jump or a function call, pass
				if (citer->op == "j" || citer->op == "call") {
					continue;
				}
				// if the quaternary is a conditional jump
				// if operator is a variable, and it is not defined before, add it to the use set
				else if (citer->op[0] == 'j') {
					if (isVar(citer->src1) && def.count(citer->src1) == 0) {
						use.insert(citer->src1);
					}
					if (isVar(citer->src2) && def.count(citer->src2) == 0) {
						use.insert(citer->src2);
					}
				}
				// if it is a normal assignment
				else {
					// if the source operand 1 and 2 is a variable, and it is not defined before, add it to the use set
					if (isVar(citer->src1) && def.count(citer->src1) == 0) {
						use.insert(citer->src1);
					}
					if (isVar(citer->src2) && def.count(citer->src2) == 0) {
						use.insert(citer->src2);
					}
					// if the destination operand is a variable, and it is not used before, add it to the def set
					if (isVar(citer->des) && use.count(citer->des) == 0) {
						def.insert(citer->des);
					}
				}
			}
			// after analysing all the quaternaries in the current basic block
			// add the use set into Inlive set, def set into DEF set
			INL.push_back(use);
			DEF.push_back(def);
			USE.push_back(use);
			// no outlive variable at the beginning
			OUTL.push_back(set<string>());
		}

		// find the outlive and inlive variables of each basic block
		// is there any change in this iteration
		bool change = true;
		while (change) {
			change = false;
			int blockIndex = 0;
			// traverse all the basic blocks of this function
			for (vector<Block>::iterator biter = blocks.begin(); biter != blocks.end(); biter++, blockIndex++) {
				int next1 = biter->next1;
				int next2 = biter->next2;
				// analyse the next basic block of the current basic block
				if (next1 != -1) {
					// traverse all the outlive variables of the next basic block
					for (set<string>::iterator inlIter = INL[next1].begin(); inlIter != INL[next1].end(); inlIter++) {
						// insert the inlive variables of the next basic block into the outlive variables of the current basic block
						if (OUTL[blockIndex].insert(*inlIter).second == true) {
							// if the variable is not defined in the current basic block, insert it into the inlive variables of the current basic block
							if (DEF[blockIndex].count(*inlIter) == 0) {
								INL[blockIndex].insert(*inlIter);
							}
							change = true;
						}
					}
				}
				// same as above
				if (next2 != -1) {
					for (set<string>::iterator inlIter = INL[next2].begin(); inlIter != INL[next2].end(); inlIter++) {
						if (OUTL[blockIndex].insert(*inlIter).second == true) {
							if (DEF[blockIndex].count(*inlIter) == 0) {
								INL[blockIndex].insert(*inlIter);
							}
							change = true;
						}
					}
				}
			}
		}
		// set the outlive and inlive variables of this basic block
		funcOUTL[fbiter->first] = OUTL;
		funcINL[fbiter->first] = INL;

		// generate the basic block with information
		for (vector<Block>::iterator iter = blocks.begin(); iter != blocks.end(); iter++) {
			BlockWithInfo iBlock;
			iBlock.next1 = iter->next1;
			iBlock.next2 = iter->next2;
			iBlock.name = iter->name;
			for (vector<Quaternary>::iterator qIter = iter->codes.begin(); qIter != iter->codes.end(); qIter++) {
				// all the variables are not active at the beginning
				iBlock.codes.push_back(QuaternaryWithInfo(*qIter, VarInfomation(-1, false), VarInfomation(-1, false), VarInfomation(-1, false)));
			}
			iBlocks.push_back(iBlock);
		}

		// there is a variable table for each basic block
		vector<map<string, VarInfomation> > symTables;
		for (vector<Block>::iterator biter = blocks.begin(); biter != blocks.end(); biter++) {
			// initialize the variable table for each basic block
			map<string, VarInfomation>symTable;
			// add all the variables in the quaternary into the variable table
			for (vector<Quaternary>::iterator citer = biter->codes.begin(); citer != biter->codes.end(); citer++) {
	
				if (citer->op == "j" || citer->op == "call") {
					continue;
				}
			
				else if (citer->op[0] == 'j') {//j>= j<=,j==,j!=,j>,j<
					if (isVar(citer->src1)) {
						symTable[citer->src1] = VarInfomation{ -1,false };
					}
					if (isVar(citer->src2)) {
						symTable[citer->src2] = VarInfomation{ -1,false };
					}
				}
				else {
					if (isVar(citer->src1)) {
						symTable[citer->src1] = VarInfomation{ -1,false };
					}
					if (isVar(citer->src2)) {
						symTable[citer->src2] = VarInfomation{ -1,false };
					}
					if (isVar(citer->des)) {
						symTable[citer->des] = VarInfomation{ -1,false };
					}
				}
			}
			symTables.push_back(symTable);
		}

		// for every basic block, set the variables in the outlive variables as active in its variable table
		int blockIndex = 0;
		for (vector<set<string> >::iterator iter = OUTL.begin(); iter != OUTL.end(); iter++, blockIndex++) {
			for (set<string>::iterator viter = iter->begin(); viter != iter->end(); viter++) {
				symTables[blockIndex][*viter] = VarInfomation{ -1,true };
			}

		}

		blockIndex = 0;
		// update the active and next information of each variable in each basic block
		for (vector<BlockWithInfo>::iterator ibiter = iBlocks.begin(); ibiter != iBlocks.end(); ibiter++, blockIndex++) {//遍历每一个基本块
			int codeIndex = ibiter->codes.size() - 1;
			// traverse all the quaternaries in the current basic block in reverse order
			for (vector<QuaternaryWithInfo>::reverse_iterator citer = ibiter->codes.rbegin(); citer != ibiter->codes.rend(); citer++, codeIndex--) {//逆序遍历基本块中的代码
				if (citer->q.op == "j" || citer->q.op == "call") {
					continue;
				}
				// the conditional jump, the source operand 1 and 2 is active, and it will be used in this code
				else if (citer->q.op[0] == 'j') {
					if (isVar(citer->q.src1)) {
						citer->info1 = symTables[blockIndex][citer->q.src1];
						symTables[blockIndex][citer->q.src1] = VarInfomation{ codeIndex,true };
					}
					if (isVar(citer->q.src2)) {
						citer->info2 = symTables[blockIndex][citer->q.src2];
						symTables[blockIndex][citer->q.src2] = VarInfomation{ codeIndex,true };
					}
				}
				// other codes
				// if the source operand 1 is a variable, and it is active, and it will be used in this code
				// for the destination operand, set it as inactive
				else {
					if (isVar(citer->q.src1)) {
						citer->info1 = symTables[blockIndex][citer->q.src1];
						symTables[blockIndex][citer->q.src1] = VarInfomation{ codeIndex,true };
					}
					if (isVar(citer->q.src2)) {
						citer->info2 = symTables[blockIndex][citer->q.src2];
						symTables[blockIndex][citer->q.src2] = VarInfomation{ codeIndex,true };
					}
					if (isVar(citer->q.des)) {
						citer->info3 = symTables[blockIndex][citer->q.des];
						symTables[blockIndex][citer->q.des] = VarInfomation{ -1,false };
					}
				}
			}
		}
		// load these basic blocks into the function blocks table
		funcIBlocks[fbiter->first] = iBlocks;
	}
}


/**
 * @brief store all the outlive variables in the memory at the end of each block
 * @param outl 
 */
void ObjectCodeGenerator::storeOutLiveVar(set<string>&outl) {
	// tranverse all locations where a outlive variable is stored
	for (set<string>::iterator oiter = outl.begin(); oiter != outl.end(); oiter++) {
		string reg;
		bool inFlag = false;
		for (set<string>::iterator aiter = Avalue[*oiter].begin(); aiter != Avalue[*oiter].end(); aiter++) {
			if ((*aiter)[0] != '$') {
				inFlag = true;
				break;
			}
			else {
				reg = *aiter;
			}
		}
		// if the variable is not stored in memory, store it in the memory
		if (!inFlag) {
			storeVar(reg, *oiter);
		}
	}
}

/**
 * @brief generate the object code for a quaternary
 * @param nowBaseBlockIndex: the index of the current basic block in the function
 * @param arg_num: the number of arguments
 * @param par_list: the list of parameters
 */
void ObjectCodeGenerator::generateCodeForQuatenary(int nowBaseBlockIndex, int &arg_num, list<pair<string, bool> > &par_list) {
	// if the source operand is not initialized, output error
	if (nowQuatenary->q.op[0] != 'j'&&nowQuatenary->q.op != "call") {
		if (isVar(nowQuatenary->q.src1) && Avalue[nowQuatenary->q.src1].empty()) {
			outputError(string("variable ") + nowQuatenary->q.src1 + " is not initialized before use");
			return;
		}
		if (isVar(nowQuatenary->q.src2) && Avalue[nowQuatenary->q.src2].empty()) {
			outputError(string("variable ") + nowQuatenary->q.src2 + " is not initialized before use");
			return;
		}
	}

	// jump instruction
	// unconditional jump
	if (nowQuatenary->q.op == "j") {
		objectCodes.push_back(nowQuatenary->q.op + " " + nowQuatenary->q.des);
	}
	// conditional jump
	else if (nowQuatenary->q.op[0] == 'j') {
		string op;
		if (nowQuatenary->q.op == "j>=")
			op = "bge";
		else if (nowQuatenary->q.op == "j>")
			op = "bgt";
		else if (nowQuatenary->q.op == "j==")
			op = "beq";
		else if (nowQuatenary->q.op == "j!=")
			op = "bne";
		else if (nowQuatenary->q.op == "j<")
			op = "blt";
		else if (nowQuatenary->q.op == "j<=")
			op = "ble";
		// allocate a register to the source operand 1 and 2
		string pos1 = allocateReg(nowQuatenary->q.src1);
		string pos2 = allocateReg(nowQuatenary->q.src2);
		objectCodes.push_back(op + " " + pos1 + " " + pos2 + " " + nowQuatenary->q.des);
		// if the source is not active, release it from the register
		if (!nowQuatenary->info1.active) {
			releaseVar(nowQuatenary->q.src1);
		}
		if (!nowQuatenary->info2.active) {
			releaseVar(nowQuatenary->q.src2);
		}
	}
	// a parameter in a function call
	else if (nowQuatenary->q.op == "par") {
		par_list.push_back(pair<string, bool>(nowQuatenary->q.src1, nowQuatenary->info1.active));
	}
	// function call
	else if (nowQuatenary->q.op == "call") {
		// store all the parameters into the stack
		for (list<pair<string, bool> >::iterator aiter = par_list.begin(); aiter != par_list.end(); aiter++) {
			// get the input argument
			string pos = allocateReg(aiter->first);
			// push it into the stack
			objectCodes.push_back(string("sw ") + pos + " " + to_string(top + 4 * (++arg_num + 1)) + "($sp)");
			// release incative variables from the register
			if (!aiter->second) {
				releaseVar(aiter->first);
			}
		}
		
		// store the current stack pointer into the buttom of stack, and set the current top as the stack pointer
		objectCodes.push_back(string("sw $sp ") + to_string(top) + "($sp)");
		objectCodes.push_back(string("addi $sp $sp ") + to_string(top));

		// jump to the specific function
		objectCodes.push_back(string("jal ") + nowQuatenary->q.src1);

		// restore the stack pointer after the function call
		objectCodes.push_back(string("lw $sp 0($sp)"));
	}
	// return
	else if (nowQuatenary->q.op == "return") {
		// the return value is a immidiate number, store it in $v0
		if (isNum(nowQuatenary->q.src1)) {
			objectCodes.push_back("addi $v0 $zero " + nowQuatenary->q.src1);
		}
		// the return value is a variable, find the register where it is stored, and store it in $v0
		else if (isVar(nowQuatenary->q.src1)) {
			set<string>::iterator piter = Avalue[nowQuatenary->q.src1].begin();
			if ((*piter)[0] == '$') {
				objectCodes.push_back(string("add $v0 $zero ") + *piter);
			}
			else {
				objectCodes.push_back(string("lw $v0 ") + to_string(varOffset[*piter]) + "($sp)");
			}
		}
		// return of the main function, jump to the end of the program
		if (nowFunc == "main") {
			objectCodes.push_back("j end");
		}
		// return of other functions, restore the return address from the stack, and jump to it
		else {
			objectCodes.push_back("lw $ra 4($sp)");
			objectCodes.push_back("jr $ra");
		}
	}
	// decleration of paremeters
	// set the offset in the stack of each parameters
	else if (nowQuatenary->q.op == "get") {
		// get the variable offset in the stack
		varOffset[nowQuatenary->q.des] = top;
		top += 4;
		Avalue[nowQuatenary->q.des].insert(nowQuatenary->q.des);
	}
	else if (nowQuatenary->q.op == "=") {
		//Avalue[nowQuatenary->q.des] = set<string>();
		string src1Pos;
		// if right value of the expression is return value of a function, it is stored in $v0
		if (nowQuatenary->q.src1 == "@RETURN_PLACE") {
			src1Pos = "$v0";
		}
		// else, allocate a register for the right value of the expression
		else {
			src1Pos = allocateReg(nowQuatenary->q.src1);
		}
		// update the Rvalue and Avalue
		Rvalue[src1Pos].insert(nowQuatenary->q.des);
		Avalue[nowQuatenary->q.des].insert(src1Pos);
	}
	// arithmetic expression
	else {
		// allocate a register for the source operand 1 and 2, and the destination operand
		string src1Pos = allocateReg(nowQuatenary->q.src1);
		string src2Pos = allocateReg(nowQuatenary->q.src2);
		string desPos = getReg();
		if (nowQuatenary->q.op == "+") {
			objectCodes.push_back(string("add ") + desPos + " " + src1Pos + " " + src2Pos);
		}
		else if (nowQuatenary->q.op == "-") {
			objectCodes.push_back(string("sub ") + desPos + " " + src1Pos + " " + src2Pos);
		}
		else if (nowQuatenary->q.op == "*") {
			objectCodes.push_back(string("mul ") + desPos + " " + src1Pos + " " + src2Pos);
		}
		else if (nowQuatenary->q.op == "/") {
			objectCodes.push_back(string("div ") + src1Pos + " " + src2Pos);
			objectCodes.push_back(string("mflo ") + desPos);
		}
		// if the source operand is not active, release it from the register
		if (!nowQuatenary->info1.active) {
			releaseVar(nowQuatenary->q.src1);
		}
		if (!nowQuatenary->info2.active) {
			releaseVar(nowQuatenary->q.src2);
		}
	}
}

/**
 * @brief generate the object code for a basic block
 * @param nowBaseBlockIndex: the index of the current basic block
 */
void ObjectCodeGenerator::generateCodeForBaseBlocks(int nowBaseBlockIndex) {
	// augment number
	int arg_num = 0;
	// parameter list used by function call
	list<pair<string, bool> > par_list;

	// clear the Avalue and Rvalue
	Avalue.clear();
	Rvalue.clear();
	// all the inalive variables of this block
	set<string>& inl = funcINL[nowFunc][nowBaseBlockIndex];
	for (set<string>::iterator iter = inl.begin(); iter != inl.end(); iter++) {
		Avalue[*iter].insert(*iter);
	}

	// all the registers are free at the beginning
	freeReg.clear();
	for (int i = 0; i <= 7; i++) {
		freeReg.push_back(string("$s") + to_string(i));
	}
	// add a label for the basic block
	objectCodes.push_back(nowIBlock->name + ":");
	if (nowBaseBlockIndex == 0) {
		if (nowFunc == "main") {
			top = 8;
		}
		// if the function is not main, store the return address in the stack, and set the top as 8
		// reserve 4($sp) for the return address
		// reserve 0($sp) for the old stack pointer
		else {
			objectCodes.push_back("sw $ra 4($sp)");
			top = 8;
		}
	}
	// for each quaternary in the basic block
	for (vector<QuaternaryWithInfo>::iterator cIter = nowIBlock->codes.begin(); cIter != nowIBlock->codes.end(); cIter++) {//对基本块内的每一条语句
		nowQuatenary = cIter;
		// the last quaternary in the basic block
		if (cIter + 1 == nowIBlock->codes.end()) {
			// if the last quaternary is a control quaternary
			// store all the outlive variables in the memory, and generate the object code for it
			if (isControlOp(cIter->q.op)) {
				storeOutLiveVar(funcOUTL[nowFunc][nowBaseBlockIndex]);
				generateCodeForQuatenary(nowBaseBlockIndex, arg_num, par_list);
			}
			// otherwise, generate the object code for it
			// and store all the outlive variables in the memory
			else {
				generateCodeForQuatenary(nowBaseBlockIndex, arg_num, par_list);
				storeOutLiveVar(funcOUTL[nowFunc][nowBaseBlockIndex]);
			}
		}
		else {
			generateCodeForQuatenary(nowBaseBlockIndex, arg_num, par_list);
		}
	}
}

/**
 * @brief generate the object code for a function
 * @param fiter: the iterator of the function
 */
void ObjectCodeGenerator::generateCodeForFuncBlocks(map<string, vector<BlockWithInfo> >::iterator &fiter) {
	varOffset.clear();
	nowFunc = fiter->first;
	vector<BlockWithInfo>&iBlocks = fiter->second;
	for (vector<BlockWithInfo>::iterator iter = iBlocks.begin(); iter != iBlocks.end(); iter++) {//对每一个基本块
		nowIBlock = iter;
		generateCodeForBaseBlocks(nowIBlock - iBlocks.begin());
	}
}

/**
 * @brief generate the object code for the whole program
 */
void ObjectCodeGenerator::generateCode() {
	// initialize the offset of the global variables, jump to the main function
	objectCodes.push_back("lui $sp,0x1001");
	objectCodes.push_back("j main");
	// generate the object code for each function
	for (map<string, vector<BlockWithInfo> >::iterator fiter = funcIBlocks.begin(); fiter != funcIBlocks.end(); fiter++) {//对每一个函数块
		generateCodeForFuncBlocks(fiter);
	}
	objectCodes.push_back("end:");
}

// output functions

/**
 * @brief output the intermediate code for each block
 * @param out: the output stream
 */
void ObjectCodeGenerator::outputIBlocks(ostream& out) {
	for (map<string, vector<BlockWithInfo> >::iterator iter = funcIBlocks.begin(); iter != funcIBlocks.end(); iter++) {
		out << "[" << iter->first << "]" << endl;
		for (vector<BlockWithInfo>::iterator bIter = iter->second.begin(); bIter != iter->second.end(); bIter++) {
			out << bIter->name << ":" << endl;
			for (vector<QuaternaryWithInfo>::iterator cIter = bIter->codes.begin(); cIter != bIter->codes.end(); cIter++) {
				out << "    ";
				cIter->output(out);
				out << endl;
			}
			out << "    " << "next1 = " << bIter->next1 << endl;
			out << "    " << "next2 = " << bIter->next2 << endl;
		}
		cout << endl;
	}
}

/**
 * @brief output the object code for each block to the standard output
 */
void ObjectCodeGenerator::outputIBlocks() {
	outputIBlocks(cout);
}

/**
 * @brief output the object code for each block to a file
 * @param fileName: the name of the output file
 */
void ObjectCodeGenerator::outputIBlocks(const char* fileName) {
	ofstream fout;
	fout.open(fileName);
	if (!fout.is_open()) {
		cerr << "file " << fileName << " open error" << endl;
		return;
	}
	outputIBlocks(fout);

	fout.close();
}

/**
 * @brief output the object code
 * @param out: the output stream
 */
void ObjectCodeGenerator::outputObjectCode(ostream& out) {
	for (vector<string>::iterator iter = objectCodes.begin(); iter != objectCodes.end(); iter++) {
		out << *iter << endl;
	}
}

/**
 * @brief output the object code to the standard output
 */
void ObjectCodeGenerator::outputObjectCode() {
	outputObjectCode(cout);
}

/**
 * @brief output the object code to a file
 * @param fileName: the name of the output file
 */
void ObjectCodeGenerator::outputObjectCode(const char* fileName) {
	ofstream fout;
	fout.open(fileName);
	if (!fout.is_open()) {
		cerr << "file " << fileName << " open error" << endl;
		return;
	}
	outputObjectCode(fout);
	fout.close();
}