#ifndef UTILS_H
#define UTILS_H
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <utility>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <cstdlib>
#include <cassert>
#include <functional>

using namespace std;

/**
 * @file utils.h
 * @author chtholly
 * @brief some common definitions and functions
 */

/**
 * @brief the type of Quaternary
 */
struct Quaternary {
	string op;					// operator
	string src1;				// source operator1
	string src2;				// source operator2
	string des;					// destination operator
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

/**
 * @brief the different type of declaration
 */
enum DecType {
	DEC_VAR,					// variable declaration
	DEC_FUN						// function declaration
};

/**
 * @brief the different type of expression
 */
enum DType {
	D_VOID, 					// void type
	D_INT 						// int type
};

/**
 * @brief the type of variable
 */
struct Var {
	string name;				// variable name
	DType type;					// variable type
	int level;					// variable level	
};

/**
 * @brief the type of function
 */
struct Func {
	string name;				// function name
	DType returnType;			// function return type
	list<DType> paramTypes;		// function parameter type
	int enterPoint;				// function enter point
};

/**
 * @brief output the error message
 * @param err 
 */
void outputError(string err);

/**
 * @brief Construct a new list<int>merge object
 * @param l1 
 * @param l2 
 */
list<int>merge(list<int>&l1, list<int>&l2);

/**
 * @brief if the string is end symbol
 * 
 * @param s 
 * @return true 
 * @return false 
 */
bool isVT(string s);

// the behavior of a status in LR1 table
enum tableBehave { reduct, shift, accept, error};


#endif // !UTILS_H
