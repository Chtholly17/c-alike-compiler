#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QTCore/QTextStream>
#include "ui_CompilerWidget.h"
#include "compiler.h"

class CompilerWidget : public QMainWindow
{
    Q_OBJECT

public:
    CompilerWidget(QWidget *parent = nullptr);
    ~CompilerWidget();
private:
    Compiler* compiler;
    // source file
    QString sourceFile;
    // change source file
    void changeSourceFile(QString sourceFile);
    // read production file
    void readProductionFile();
    // lexical analyse
    void lexicalAnalyse();
    // syntax analyse
    void syntaxAnalyse();
    // object code generation
    void objectCodeGeneration();


// slots
public slots:
    // actionSelect_file
    void selectFile();
    // clean text
    void cleanText();
    // close file
    void closeFile();
    // save file as
    void saveFileAs();
    // save file
    void saveFile();
    // compile
    void compile();
    // when the text in the text edit changed
    void textChanged();
    // show Tokens
    void showTokens();
    // show state stack
    void showStateStack();
    // show symbol stack
    void showSymbolStack();
    // show intermediate code
    void showIntermediateCode();
    // show object code
    void showObjectCode();
    // show productions
    void showProductions();
    // show DFA
    void showDFA();

private:
    Ui::CompilerWidgetClass ui;
};
