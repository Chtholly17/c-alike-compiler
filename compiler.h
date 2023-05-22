#include "LexicalAnalyser.h"
#include "Parser.h"
#include "ObjectCodeGenerator.h"

#ifndef COMPILER_H
#define COMPILER_H

extern string errorMessage;
extern string infoMessage;

enum class CompilerState
{
    Wait,
    LexicalAnalyse,
    SyntaxAnalyse,
    IntermediateCodeGenerate,
    ObjectCodeGenerate,
    Finish,
    Error
};

class Compiler
{
private:
    string errorMes;
    CompilerState state;
    LexicalAnalyser* lexicalAnalyser;
    AnalyseTable* analyseTable;
    Parser* parser;
    string sourceFile;
public:  
    Compiler(const char* ProductionFile);
    Compiler();
    ~Compiler();
    void compile(const char* sourceFile);
    CompilerState getState();
    string getErrorMessage();
    string getSourceFile();
    void setSourceFile(string sourceFile);
    void lexicalAnalyse();
    void syntaxAnalyse();
    void objectCodeGenerate();
};



#endif // !COMPILER_H