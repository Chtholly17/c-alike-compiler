#include "Compiler.h"

/**
 * @brief Construct a new Compiler:: Compiler object
 * @param ProductionFile : the path of the production file
 */
Compiler::Compiler(const char* ProductionFile)
{
    errorMessage = "";
    this->state = CompilerState::Wait;
    this->lexicalAnalyser = nullptr;
    this->parser = nullptr;
    this->analyseTable = new AnalyseTable(ProductionFile);
    this->analyseTable->outputDFA("DFA.txt");
    this->errorMes = "";
    if(errorMessage != "")
    {
        this->errorMes = errorMessage;
    }
}

/**
 * @brief get the state of the compiler
 * @return CompilerState 
 */
CompilerState Compiler::getState()
{
    return this->state;
}

/**
 * @brief get the error message
 * @return string 
 */
string Compiler::getErrorMessage()
{
    return this->errorMes;
}


/**
 * @brief set the source file
 * @param sourceFile 
 */
void Compiler::setSourceFile(string sourceFile)
{
    this->sourceFile = sourceFile;
}

/**
 * @brief get the source file
 * @return string 
 */
string Compiler::getSourceFile(){
    return this->sourceFile;
}

/**
 * @brief lexical analyse
 */
void Compiler::lexicalAnalyse(){
    this->state = CompilerState::LexicalAnalyse;
    this->lexicalAnalyser = new LexicalAnalyser(this->sourceFile.c_str());
    this->lexicalAnalyser->analyse();
    this->lexicalAnalyser->outputToFile("lexical.txt");
    this->errorMes = errorMessage;
    if (errorMessage != "")
    {
        this->state = CompilerState::Error;
    }
}

/**
 * @brief syntax analyse
 */
void Compiler::syntaxAnalyse(){
    this->state = CompilerState::SyntaxAnalyse;
    list<Token> tokens = this->lexicalAnalyser->getResult();
    this->parser = new Parser(this->analyseTable);
    this->parser->analyse(tokens, "symbol.txt", "state.txt");
    this->errorMes = errorMessage;
    if (errorMessage != "")
    {
        this->state = CompilerState::Error;
    }
}

/**
 * @brief object code generate
 */
void Compiler::objectCodeGenerate(){
    this->state = CompilerState::ObjectCodeGenerate;
    IntermediateCode* code = this->parser->getIntermediateCode();
    code->divideBlocks(this->parser->getFuncEnter());

    ObjectCodeGenerator objectCodeGenerator;
	objectCodeGenerator.analyseBlock(code->getFuncBlock());
    this->state = CompilerState::ObjectCodeGenerate;
	objectCodeGenerator.outputIBlocks("intermediate.txt");
	objectCodeGenerator.generateCode();
	objectCodeGenerator.outputObjectCode();
	objectCodeGenerator.outputObjectCode("object.asm");
    this->errorMes = errorMessage;
    if (errorMessage != "")
    {
        this->state = CompilerState::Error;
    }
    else
    {
        this->state = CompilerState::Finish;
    }
}

/**
 * @brief use the compiler to compile the source file and generate the object file
 */
void Compiler::compile(const char* sourceFile)
{
    this->setSourceFile(sourceFile);
    lexicalAnalyse();
    syntaxAnalyse();
    objectCodeGenerate();
}

/**
 * @brief destroy the Compiler object
 */
Compiler::~Compiler(){}