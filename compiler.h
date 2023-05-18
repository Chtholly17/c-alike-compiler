#include "LexicalAnalyser.h"
#include "Parser.h"
#include "ObjectCodeGenerator.h"

#ifndef COMPILER_H
#define COMPILER_H
class Compiler
{
private:
    LexicalAnalyser* lexicalAnalyser;
    AnalyseTable* analyseTable;
    Parser* parser;
public:
    Compiler(const char* ProductionFile);
    void compile(const char* sourceFile, const char* objectFile);
};



#endif // !COMPILER_H