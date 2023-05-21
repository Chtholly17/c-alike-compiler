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
#include <algorithm>
#include <string.h>

using namespace std;

/**
 * @file utils.h
 * @author chtholly
 * @brief some common definitions and functions
 */

/**
 * @brief the type of table behavior
 */
enum tableBehave { 
	reduct,
	shift, 
	accept, 
	error
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

/**
 * @brief if the string is variable
 * 
 * @param s 
 * @return true 
 * @return false 
 */
bool isVar(string name);

/**
 * @brief if the string is number
 * 
 * @param s 
 * @return true 
 * @return false 
 */
bool isNum(string name);

/**
 * @brief if the string is control operator
 * 
 * @param op 
 * @return true 
 * @return false 
 */
bool isControlOp(string op);


#endif // !UTILS_H
