#include "LexicalAnalyser.h"
#include "Parser.h"
#include "ObjectCodeGenerator.h"
int main() {
	LexicalAnalyser lexicalAnalyser("test.txt");
	lexicalAnalyser.analyse();
	
	AnalyseTable analyseTable("productions.txt");
	Parser parserAndSemanticAnalyser(&analyseTable);
	//parserAndSemanticAnalyser.outputDFA("DFA.txt");
	parserAndSemanticAnalyser.analyse(lexicalAnalyser.getResult(), "LR1_analyse.txt");
	//parserAndSemanticAnalyser.outputIntermediateCode();
	
	IntermediateCode* code = parserAndSemanticAnalyser.getIntermediateCode();
	code->divideBlocks(parserAndSemanticAnalyser.getFuncEnter());
	//code->outputBlocks();

	ObjectCodeGenerator objectCodeGenerator;
	objectCodeGenerator.analyseBlock(code->getFuncBlock());
	objectCodeGenerator.outputIBlocks();
	objectCodeGenerator.generateCode();
	objectCodeGenerator.outputObjectCode();
	objectCodeGenerator.outputObjectCode("program.asm");

	return 0;
}