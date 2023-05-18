#include "compiler.h"

/**
 * @brief Construct a new Compiler:: Compiler object
 * @param ProductionFile : the path of the production file
 */
Compiler::Compiler(const char* ProductionFile)
{
    this->lexicalAnalyser = nullptr;
    this->parser = nullptr;
    this->analyseTable = new AnalyseTable(ProductionFile);
}

/**
 * @brief use the compiler to compile the source file and generate the object file
 */
void Compiler::compile(const char* sourceFile, const char* objectFile)
{
    this->lexicalAnalyser = new LexicalAnalyser(sourceFile);
    this->lexicalAnalyser->analyse();
    list<Token> tokens = this->lexicalAnalyser->getResult();
    this->parser = new Parser(this->analyseTable);
    this->parser->analyse(tokens, "LR1_analyse.txt");
    IntermediateCode* code = this->parser->getIntermediateCode();
    code->divideBlocks(this->parser->getFuncEnter());
	//code->outputBlocks();

	ObjectCodeGenerator objectCodeGenerator;
	objectCodeGenerator.analyseBlock(code->getFuncBlock());
	objectCodeGenerator.outputIBlocks();
	objectCodeGenerator.generateCode();
	objectCodeGenerator.outputObjectCode();
	objectCodeGenerator.outputObjectCode(objectFile);
}