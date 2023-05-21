#pragma once
#include "utils.h"
#include "IntermediateCode.h"

/**
 * @brief the next use and active information of a variable
 */
struct VarInfomation {
	int next;			// next use
	bool active;		// active or not

	// constructor
	VarInfomation(int next, bool active);
	VarInfomation(const VarInfomation&other);
	VarInfomation();
	// output the information
	void output(ostream& out);
};

/**
 * @brief the variable information of all variables in a quaternary
 */
struct QuaternaryWithInfo {
public:
	Quaternary q;
	VarInfomation info1;
	VarInfomation info2;
	VarInfomation info3;
	// constructor
	QuaternaryWithInfo(Quaternary q, VarInfomation info1, VarInfomation info2, VarInfomation info3);
	// output the information
	void output(ostream& out);
};

/**
 * @brief the basic block with information
 */
struct BlockWithInfo {
	string name;
	vector<QuaternaryWithInfo> codes;
	int next1;
	int next2;
};

/**
 * @brief the object code generator
 * @detail generate the object code from the intermediate code,
 * 	   using register t0 and t1 to store the temporary constant,
 * 	   using register v0 to store the return value of a function
 */
class ObjectCodeGenerator {
private:
	map<string,vector<BlockWithInfo> >funcIBlocks;		// the basic blocks of each function
	map<string, set<string> >Avalue;					// the A value, Avalue[var] = {var, reg1, reg2, ...}, indicating loactions of a variable(temporary or not)
	map<string, set<string> >Rvalue;					// the R value, Rvalue[Reg] = {var1, var2, ...}, indicating the variables stored in a register
	map<string, int>varOffset;							// the offset in the stack of each variable
	int top;											// the top of the stack
	list<string>freeReg;								// the free registers		
	map<string, vector<set<string> > >funcOUTL;			// the out live variables of each function
	map<string, vector<set<string> > >funcINL;			// the in live variables of each function
	string nowFunc;										// the function name of the current basic block
	vector<BlockWithInfo>::iterator nowIBlock;			// the current basic block
	vector<QuaternaryWithInfo>::iterator nowQuatenary;	// the current quaternary
	vector<string>objectCodes;							// the object codes

	void outputIBlocks(ostream& out);
	void outputObjectCode(ostream& out);
	void storeVar(string reg, string var);
	void storeOutLiveVar(set<string>&outl);
	void releaseVar(string var);
	string getReg();
	string selectReg();
	string allocateReg(string var);

	void generateCodeForFuncBlocks(map<string, vector<BlockWithInfo> >::iterator &fiter);
	void generateCodeForBaseBlocks(int nowBaseBlockIndex);
	void generateCodeForQuatenary(int nowBaseBlockIndex, int &arg_num, list<pair<string, bool> > &par_list);
public:
	ObjectCodeGenerator();
	void generateCode();
	void analyseBlock(map<string, vector<Block> >*funcBlocks);
	void outputIBlocks();
	void outputIBlocks(const char* fileName);
	void outputObjectCode();
	void outputObjectCode(const char* fileName);
};