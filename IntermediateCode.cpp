#include "IntermediateCode.h"

/**
 * @brief Construct a new Intermediate Code:: Intermediate Code object
 */
IntermediateCode::IntermediateCode() {
	tempIndex = 0;
	labelIndex = 0;
}

/**
 * @brief Generate a new label
 * @return string : label name
 */
string IntermediateCode::newLabel() {
	return string("Label") + to_string(labelIndex++);
}

/**
 * @brief Generate a new temporary variable
 * @return string: temporary variable name
 */
string IntermediateCode::newTemp() {
	return string("T") + to_string(tempIndex++);
}

/**
 * @brief Get the next quad object
 * @return int: next quad index
 */
int IntermediateCode::nextQuad() {
	return code.size();
}

/**
 * @brief Get the function blocks object
 * @return map<string, vector<Block> >* 
 */
map<string, vector<Block> >* IntermediateCode::getFuncBlock() {
	return &funcBlocks;
}

/**
 * @brief Generate a new quaternary
 * @param q 
 */
void IntermediateCode::_emit(Quaternary q) {
	code.push_back(q);
}

/**
 * @brief Generate a new quaternary
 * @param op 
 * @param src1 
 * @param src2 
 * @param des 
 */
void IntermediateCode::_emit(string op, string src1, string src2, string des) {
	_emit(Quaternary{ op,src1,src2,des });
}

/**
 * @brief Back patch the quaternary
 * @param nextList 
 * @param quad 
 */
void IntermediateCode::back_patch(list<int>nextList, int quad) {
	for (list<int>::iterator iter = nextList.begin(); iter != nextList.end(); iter++) {
		code[*iter].des = to_string(quad);
	}
}

/**
 * @brief Divide basic blocks for each function
 * @param funcEnter: function enter points
 */
void IntermediateCode::divideBlocks(vector<pair<int, string> > funcEnter) {
	// traverse all functions enter points
	for (vector<pair<int, string> >::iterator iter = funcEnter.begin(); iter != funcEnter.end(); iter++) {
		// all basic blocks
		vector<Block>blocks;
		// get the enter points of each block
		priority_queue<int, vector<int>, greater<int> >block_enter;
		// push the first enter point of each function
		block_enter.push(iter->first);
		// get the end points of each block, if it is the last block(main), the end point is the end of the intermediate code
		// otherwise, the end point is the next function enter point
		int endIndex = iter + 1 == funcEnter.end()? code.size(): (iter + 1)->first;
		for (int i = iter->first; i != endIndex; i++) {
			// jump instruction
			if (code[i].op[0] == 'j') {
				// unconditional jump, push the jump target as the enter point
				if (code[i].op == "j") {
					block_enter.push(atoi(code[i].des.c_str()));
				}
				// conditional jump, push the next instruction and the jump target as the enter point
				else {
					if (i + 1 < endIndex) {
						block_enter.push(i + 1);
					}
					block_enter.push(atoi(code[i].des.c_str()));
				}
			}
			// return or call instruction, push the next instruction as the enter point
			else if (code[i].op == "return" || code[i].op == "call") {
				if (i + 1 < endIndex) {
					block_enter.push(i + 1);
				}
			}
		}

		//devide blocks
		Block block;
		// map from label to enter point
		map<int, string>labelEnter;
		// map from enter point to block index
		map<int, int>enter_block;
		// the first block of a function, name is the function name
		int firstFlag = true;
		// get the last enter point among the block enter points
		int lastEnter = block_enter.top();
		block_enter.pop();
		while (!block_enter.empty()) {
			// current enter point
			int enter = block_enter.top();
			block_enter.pop();
			// if the enter point is the same as the last enter point, continue
			if (enter == lastEnter) {
				continue;
			}
			// fill all the instructions between the last enter point and the current enter point into the block
			for (int i = lastEnter; i != enter; i++) {
				block.codes.push_back(code[i]);
			}
			// if the block is not the first block of the function, generate a new label for the block
			if (!firstFlag) {
				block.name = newLabel();
				labelEnter[lastEnter] = block.name;
			}
			// if the block is the first block of the function, use the function name as the block name
			else {
				block.name = iter->second;
				firstFlag = false;
			}
			// set the enter point of last block, as the index of the current block
			enter_block[lastEnter] = blocks.size();
			// push the block into the block vector
			blocks.push_back(block);
			lastEnter = enter;
			// clear the temp block
			block.codes.clear();
		}

		// setting the last block, similar to the above
		if (!firstFlag) {
			block.name = newLabel();
			labelEnter[lastEnter] = block.name;
		}
		else {
			block.name = iter->second;
			firstFlag = false;
		}
		if (iter + 1 != funcEnter.end()) {
			for (int i = lastEnter; i != (iter+1)->first; i++) {
				block.codes.push_back(code[i]);
			}
		}
		else {
			for (int i = lastEnter; i != code.size(); i++) {
				block.codes.push_back(code[i]);
			}
		}
		enter_block[lastEnter] = blocks.size();
		blocks.push_back(block);
		// set next block for each block
		int blockIndex = 0;
		for (vector<Block>::iterator bIter = blocks.begin(); bIter != blocks.end(); bIter++, blockIndex++) {
			vector<Quaternary>::reverse_iterator lastCode = bIter->codes.rbegin();
			// if the last instruction is a jump instruction
			if (lastCode->op[0] == 'j') {
				// if the jump instruction is unconditional jump, set the next block as the jump target
				if (lastCode->op == "j") {
					bIter->next1 = enter_block[atoi(lastCode->des.c_str())];
					bIter->next2 = -1;
				}
				// if the jump instruction is conditional jump, set the next block as the next instruction and the jump target
				else {
					bIter->next1 = blockIndex + 1;
					bIter->next2 = enter_block[atoi(lastCode->des.c_str())];
					bIter->next2 = bIter->next1 == bIter->next2 ? -1 : bIter->next2;
				}
				// set the jump target as the label
				lastCode->des = labelEnter[atoi(lastCode->des.c_str())];
			}
			// if the last instruction is a return instruction, set the next block as -1
			else if (lastCode->op == "return") {
				bIter->next1 = bIter->next2 = -1;
			}
			// other instructions, set the next block as the next instruction
			else {
				bIter->next1 = blockIndex + 1;
				bIter->next2 = -1;
			}
			
		}
		// set the function name and blocks
		funcBlocks[iter->second] = blocks;
	}
}

/**
 * @brief Output blocks
 * @param out: output stream
 */
void IntermediateCode::outputBlocks(ostream& out) {
	for (map<string, vector<Block> >::iterator iter = funcBlocks.begin(); iter != funcBlocks.end(); iter++) {
		out << "[" << iter->first << "]" << endl;
		for (vector<Block>::iterator bIter = iter->second.begin(); bIter != iter->second.end(); bIter++) {
			out << bIter->name << ":" << endl;
			for (vector<Quaternary>::iterator cIter = bIter->codes.begin(); cIter != bIter->codes.end(); cIter++) {
				out <<"    "<< "(" << cIter->op << "," << cIter->src1 << "," << cIter->src2 << "," << cIter->des << ")" << endl;
			}
			out << "    " << "next1 = " << bIter->next1 << endl;
			out << "    " << "next2 = " << bIter->next2 << endl;
		}
		cout << endl;
	}
}

/**
 * @brief Output blocks to console
 */
void IntermediateCode::outputBlocks() {
	outputBlocks(cout);
}

/**
 * @brief Output blocks to file
 * @param fileName 
 */
void IntermediateCode::outputBlocks(const char* fileName) {
	ofstream fout;
	fout.open(fileName);
	if (!fout.is_open()) {
		cerr << "file " << fileName << " open error" << endl;
		return;
	}
	outputBlocks(fout);

	fout.close();
}

/**
 * @brief Output intermediate code
 * @param out : output stream
 */
void IntermediateCode::output(ostream& out) {
	int i = 0;
	for (vector<Quaternary>::iterator iter = code.begin(); iter != code.end(); iter++, i++) {
		out << setw(4) << i;
		out << "( " << iter->op << " , ";
		out << iter->src1 << " , ";
		out << iter->src2 << " , ";
		out << iter->des << " )";
		out << endl;
	}
}

/**
 * @brief Output intermediate code to console
 */
void IntermediateCode::output() {
	output(cout);
}

/**
 * @brief Output intermediate code to file
 * @param fileName 
 */
void IntermediateCode::output(const char* fileName) {
	ofstream fout;
	fout.open(fileName);
	if (!fout.is_open()) {
		cerr << "file " << fileName << " open error" << endl;
		return;
	}
	output(fout);

	fout.close();
}
