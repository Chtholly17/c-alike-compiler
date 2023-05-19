#pragma once
#include "utils.h"

/**
 * @brief the type of Quaternary
 */
struct Quaternary {
	string op;					// operator
	string src1;				// source operator1
	string src2;				// source operator2
	string des;					// destination quaternary
};


/**
 * @brief the type of code block
 */
struct Block {
	string name; 				// block name
	vector<Quaternary> codes; 	// contain a list of quaternary
	int next1;					// next block index
	int next2;
};

class IntermediateCode {
private:
	vector<Quaternary> code;				// generated intermediate code
	map<string, vector<Block> >funcBlocks;	// function blocks
	int tempIndex;							// index of temporary variable
	int labelIndex;							// index of label

	void output(ostream& out);
	void outputBlocks(ostream& out);
public:
	IntermediateCode();
	string newLabel();
	string newTemp();
	void emit(Quaternary q);
	void emit(string op, string src1, string src2, string des);
	void back_patch(list<int>nextList,int quad);
	void output();
	void output(const char* fileName);
	void divideBlocks(vector<pair<int, string> > funcEnter);
	void outputBlocks();
	void outputBlocks(const char* fileName);
	map<string, vector<Block> >*getFuncBlock();
	int nextQuad();
};