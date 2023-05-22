#include "CompilerWidget.h"

/**
 * @brief constructor
 * @details  set window title as "Compiler", connect slots
 */
CompilerWidget::CompilerWidget(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    // set window title as "Compiler"
    setWindowTitle(tr("Compiler"));
    // init compiler
    compiler = nullptr;
    sourceFile = "";
    // set all the browser have a vertical scroll bar
    ui.infoBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui.messageBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui.errorBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // connect slots
    // change the source file label when the source file changed
    connect(ui.actionSelect_file, SIGNAL(triggered()), this, SLOT(selectFile()));
    connect(ui.actionClean_file, SIGNAL(triggered()), this, SLOT(cleanText()));
    connect(ui.actionSave_file, SIGNAL(triggered()), this, SLOT(saveFile()));
    connect(ui.sourceEdit, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(ui.actionClose_file, SIGNAL(triggered()), this, SLOT(closeFile()));
    connect(ui.actionSave_as, SIGNAL(triggered()), this, SLOT(saveFileAs()));
    connect(ui.actionRun_Compiler, SIGNAL(triggered()), this, SLOT(compile()));
    connect(ui.actionTokens, SIGNAL(triggered()), this, SLOT(showTokens()));
    connect(ui.actionState_Stack, SIGNAL(triggered()), this, SLOT(showStateStack()));
    connect(ui.actionSymbol_Stack, SIGNAL(triggered()), this, SLOT(showSymbolStack()));
    connect(ui.actionIntermediate_Code, SIGNAL(triggered()), this, SLOT(showIntermediateCode()));
    connect(ui.actionAssemble_Code, SIGNAL(triggered()), this, SLOT(showObjectCode()));
    connect(ui.actionProduction, SIGNAL(triggered()), this, SLOT(showProductions()));
    connect(ui.actionDFA, SIGNAL(triggered()), this, SLOT(showDFA()));
}

/**
 * @brief change source file
 */
void CompilerWidget::changeSourceFile(QString sourceFile){
    this->sourceFile = sourceFile;
    // the substring of the source file after the last '/'
    string fileName = sourceFile.toStdString().substr(sourceFile.toStdString().find_last_of('/') + 1);
    // emit source file changed
    if(sourceFile == "")
        ui.label->setText("No file selected");
    else
        ui.label->setText(QString::fromStdString(fileName));
}


/**
 * @brief select file
 * @details  open a file select dialog and select a file, get the file path and show it in the text edit
 */
void CompilerWidget::selectFile(){
    // open a file select dialog and select a file(cpp c or txt)
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("C++ Files (*.cpp *.c *.txt)"));
    // set the source file of the compiler
    // sourceFile = fileName;
    changeSourceFile(fileName);
    // get the file path and show it in the text edit
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        QTextStream in(&file);
        ui.sourceEdit->setText(in.readAll());
        file.close();
    }
    // set the name of sourcetab as "Source Code"
    ui.tabWidget_2->setTabText(0, "Source Code");
}

/**
 * @brief clean text
 * @details  clean the text edit
 */
void CompilerWidget::cleanText(){
    ui.sourceEdit->clear();
    // set the name of sourcetab as "Source Code*"
    ui.tabWidget_2->setTabText(0, "Source Code*");
}

/**
 * @brief save file
 * @details save the source code or information to a file
 */
void CompilerWidget::saveFileAs(){
    // open a file save dialog and select a file
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text Files (*.txt)"));
    // get the file path and save the text edit to the file
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        QTextStream out(&file);
        // if the source tab is selected, save the source code
        // otherwise, save the information tab
        if (ui.tabWidget_2->currentIndex() == 0) {
            changeSourceFile(fileName);
            out << ui.sourceEdit->toPlainText();
        }
        else {
            out << ui.infoBrowser->toPlainText();
        }

        file.close();
    }
    // set the name of sourcetab as "Source Code*"
    ui.tabWidget_2->setTabText(0, "Source Code");
}

/**
 * @brief save file 
 */
void CompilerWidget::saveFile(){
    // if the source tab is selected
    if(ui.tabWidget_2->currentIndex() == 0 && sourceFile != ""){
        QFile file(sourceFile);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        QTextStream out(&file);
        out << ui.sourceEdit->toPlainText();
        file.close();
    }
    else{
        saveFileAs();
    }
    // set the name of sourcetab as "Source Code*"
    ui.tabWidget_2->setTabText(0, "Source Code");
}

/**
 * @brief close file
 */
void CompilerWidget::closeFile(){
    // if the source file is not empty, save the source code to the source file
    if(sourceFile != ""){
        saveFile();
        changeSourceFile("");
    }
    cleanText();
    // set the name of sourcetab as "Source Code*"
    ui.tabWidget_2->setTabText(0, "Source Code");
}

/**
 * @brief when the text in the text edit changed
 * 
 */
void CompilerWidget::textChanged(){
    // set the name of sourcetab as "Source Code*"
    ui.tabWidget_2->setTabText(0, "Source Code*");
}

/**
 * @brief read production file
 */ 
void CompilerWidget::readProductionFile(){
    // read productions from productions.txt
    ui.messageBrowser->append("Reading productions from productions.txt...");
    // if the productions.txt is not exist, show an error message
    if(compiler->getErrorMessage() != ""){
        ui.messageBrowser->append("Error occurs when reading productions from productions.txt, see the error message for details.");
        // show the error message
        ui.errorBrowser->append(QString::fromStdString(compiler->getErrorMessage()));
        QMessageBox::critical(this, tr("Error"), tr("File productions.txt open error"));
        return;
    }
    ui.messageBrowser->append("Successfully read productions from productions.txt.");
}

/**
 * @brief lexical analyse
 */
void CompilerWidget::lexicalAnalyse(){
    if(this->compiler->getState() == CompilerState::Error){
        return;
    }
    // lexical analyse
    ui.messageBrowser->append("Running lexical analysis...");
    compiler->lexicalAnalyse();
    // if the lexical analysis failed, show an error message
    if(compiler->getErrorMessage() != ""){
        ui.messageBrowser->append("Error occurs when running lexical analysis, see the error message for details.");
        // show the error message
        ui.errorBrowser->append(QString::fromStdString(compiler->getErrorMessage()));
        QMessageBox::critical(this, tr("Error"), tr("Lexical analysis error"));
        return;
    }
    ui.messageBrowser->append("Successfully run lexical analysis.");
}

/**
 * @brief syntax analyse
 */
void CompilerWidget::syntaxAnalyse(){
    if(this->compiler->getState() == CompilerState::Error){
        return;
    }
    // syntax analyse
    ui.messageBrowser->append("Running syntax analysis...");
    compiler->syntaxAnalyse();
    // if the syntax analysis failed, show an error message
    if(compiler->getErrorMessage() != ""){
        ui.messageBrowser->append("Error occurs when running syntax analysis, see the error message for details.");
        // show the error message
        ui.errorBrowser->append(QString::fromStdString(compiler->getErrorMessage()));
        QMessageBox::critical(this, tr("Error"), tr("Syntax analysis error"));
        return;
    }
    ui.messageBrowser->append("Successfully run syntax analysis.");
}

/**
 * @brief object code generation
 */
void CompilerWidget::objectCodeGeneration(){
    if(this->compiler->getState() == CompilerState::Error){
        return;
    }
    // object code generation
    ui.messageBrowser->append("Running object code generation...");
    compiler->objectCodeGenerate();
    // if the object code generation failed, show an error message
    if(compiler->getErrorMessage() != ""){
        ui.messageBrowser->append("Error occurs when running object code generation, see the error message for details.");
        // show the error message
        ui.errorBrowser->append(QString::fromStdString(compiler->getErrorMessage()));
        QMessageBox::critical(this, tr("Error"), tr("Object code generation error"));
        return;
    }
    ui.messageBrowser->append("Successfully run object code generation.");
}

/**
 * @brief compile the source code
 */
void CompilerWidget::compile(){
    
    // if the sourceEdit is empty show an error message
    if(ui.sourceEdit->toPlainText() == ""){
        QMessageBox::critical(this, tr("Error"), tr("No source code"));
        return;
    }
    // clean all the browser
    ui.messageBrowser->clear();
    ui.messageBrowser->clear();
    ui.errorBrowser->clear();
    // save the source code to the source file
    saveFile();
    string errMeg;

    // reconstruct a compiler
    if(compiler != nullptr){
        delete compiler;
    }
    this->compiler = new Compiler("productions.txt");

    // set the source file of the compiler
    compiler->setSourceFile(sourceFile.toStdString());
    // compile
    ui.messageBrowser->append("Compiling...");
    readProductionFile();
    lexicalAnalyse();
    syntaxAnalyse();
    objectCodeGeneration();
    ui.messageBrowser->append("");
    if(this->compiler->getState() == CompilerState::Error){
        return;
    }
    ui.messageBrowser->append("Successfully compile the source code!");
}

/**
 * @brief show tokens
 * @detail show the tokens in the info browser, read the tokens from lexical.txt
 */
void CompilerWidget::showTokens(){
    // if the compiler is not constructed, show an error message
    if(compiler == nullptr || compiler->getState() != CompilerState::Finish){
        QMessageBox::critical(this, tr("Error"), tr("Compile the source code first"));
        return;
    }
    // read tokens from lexical.txt
    ui.infoBrowser->clear();
    ui.infoBrowser->append("Tokens:");
    ui.infoBrowser->append("");
    // read file lexical.txt
    ifstream in("lexical.txt", ios::in);
    if(!in.is_open()){
        QMessageBox::critical(this, tr("Error"), tr("File lexical.txt open error"));
        return;
    }
    string line;
    while(getline(in, line)){
        ui.infoBrowser->append(QString::fromStdString(line));
    }
    in.close();
    // set the name of infotab as "Tokens"
    ui.tabWidget_2->setTabText(1, "Tokens");
}

/**
 * @brief show state stack
 * @details show the state stack in the info browser, read the state stack from stateStack.txt
 */
void CompilerWidget::showStateStack(){
    // if the compiler is not constructed, show an error message
    if(compiler == nullptr || compiler->getState() != CompilerState::Finish){
        QMessageBox::critical(this, tr("Error"), tr("Compile the source code first"));
        return;
    }
    // read state stack from stateStack.txt
    ui.infoBrowser->clear();
    ui.infoBrowser->append("State Stack:");
    ui.infoBrowser->append("");
    // read file stateStack.txt
    ifstream in("state.txt", ios::in);
    if(!in.is_open()){
        QMessageBox::critical(this, tr("Error"), tr("File state.txt open error"));
        return;
    }
    string line;
    while(getline(in, line)){
        ui.infoBrowser->append(QString::fromStdString(line));
    }
    in.close();
    // set the name of infotab as "State Stack"
    ui.tabWidget_2->setTabText(1, "State Stack");
}

/**
 * @brief show symbol stack
 * @details show the symbol stack in the info browser, read the symbol stack from symbolStack.txt
 */
void CompilerWidget::showSymbolStack(){
    // if the compiler is not constructed, show an error message
    if(compiler == nullptr || compiler->getState() != CompilerState::Finish){
        QMessageBox::critical(this, tr("Error"), tr("Compile the source code first"));
        return;
    }
    // read symbol stack from symbolStack.txt
    ui.infoBrowser->clear();
    ui.infoBrowser->append("Symbol Stack:");
    ui.infoBrowser->append("");
    // read file symbolStack.txt
    ifstream in("symbol.txt", ios::in);
    if(!in.is_open()){
        QMessageBox::critical(this, tr("Error"), tr("File symbol.txt open error"));
        return;
    }
    string line;
    while(getline(in, line)){
        ui.infoBrowser->append(QString::fromStdString(line));
    }
    in.close();
    // set the name of infotab as "Symbol Stack"
    ui.tabWidget_2->setTabText(1, "Symbol Stack");
}

/**
 * @brief show intermediate code
 * @details show the intermediate code in the info browser, read the intermediate code from intermediate.txt
 */
void CompilerWidget::showIntermediateCode(){
    // if the compiler is not constructed, show an error message
    if(compiler == nullptr || compiler->getState() != CompilerState::Finish){
        QMessageBox::critical(this, tr("Error"), tr("Compile the source code first"));
        return;
    }
    // read intermediate code from intermediate.txt
    ui.infoBrowser->clear();
    ui.infoBrowser->append("Intermediate Code:");
    ui.infoBrowser->append("");
    // read file intermediate.txt
    ifstream in("intermediate.txt", ios::in);
    if(!in.is_open()){
        QMessageBox::critical(this, tr("Error"), tr("File intermediate.txt open error"));
        return;
    }
    string line;
    while(getline(in, line)){
        ui.infoBrowser->append(QString::fromStdString(line));
    }
    in.close();
    // set the name of infotab as "Intermediate Code"
    ui.tabWidget_2->setTabText(1, "Intermediate Code");
}

/**
 * @brief show object code
 * @details show the object code in the info browser, read the object code from objectCode.txt
 */
void CompilerWidget::showObjectCode(){
    // if the compiler is not constructed, show an error message
    if(compiler == nullptr || compiler->getState() != CompilerState::Finish){
        QMessageBox::critical(this, tr("Error"), tr("Compile the source code first"));
        return;
    }
    // read object code from objectCode.txt
    ui.infoBrowser->clear();
    ui.infoBrowser->append("");
    // read file objectCode.txt
    ifstream in("object.asm", ios::in);
    if(!in.is_open()){
        QMessageBox::critical(this, tr("Error"), tr("File object.asm open error"));
        return;
    }
    string line;
    while(getline(in, line)){
        ui.infoBrowser->append(QString::fromStdString(line));
    }
    in.close();
    // set the name of infotab as "Object Code"
    ui.tabWidget_2->setTabText(1, "Object Code");
}

/**
 * @brief show productions
 * @details show the productions in the info browser, read the productions from productions.txt
 */
void CompilerWidget::showProductions(){
    // read productions from productions.txt
    ui.infoBrowser->clear();
    ui.infoBrowser->append("Productions:");
    ui.infoBrowser->append("");
    // read file productions.txt
    ifstream in("productions.txt", ios::in);
    if(!in.is_open()){
        QMessageBox::critical(this, tr("Error"), tr("File productions.txt open error"));
        return;
    }
    string line;
    while(getline(in, line)){
        ui.infoBrowser->append(QString::fromStdString(line));
    }
    in.close();
    // set the name of infotab as "Productions"
    ui.tabWidget_2->setTabText(1, "Productions");
}

/**
 * @brief show DFA
 * @details show the DFA in the info browser, read the DFA from DFA.txt
 */
void CompilerWidget::showDFA(){
    if (compiler == nullptr ) {
        this->compiler = new Compiler("productions.txt");
    }
    // read DFA from DFA.txt
    ui.infoBrowser->clear();
    ui.infoBrowser->append("DFA:");
    ui.infoBrowser->append("");
    // read file DFA.txt
    ifstream in("DFA.txt", ios::in);
    if(!in.is_open()){
        QMessageBox::critical(this, tr("Error"), tr("File DFA.txt open error"));
        return;
    }
    string line;
    while(getline(in, line)){
        ui.infoBrowser->append(QString::fromStdString(line));
    }
    in.close();
    // set the name of infotab as "DFA"
    ui.tabWidget_2->setTabText(1, "DFA");
    delete this->compiler;
    this->compiler = nullptr;
}



/**
 * @brief destructor
 */
CompilerWidget::~CompilerWidget() {
    // delete intermediate.txt, lexical.txt, objectCode.txt, productions.txt, stateStack.txt, symbolStack.txt
    remove("intermediate.txt");
    remove("lexical.txt");
    remove("object.asm");
    remove("state.txt");
    remove("symbol.txt");
    remove("DFA.txt");
    // delete compiler
    if(compiler != nullptr){
        delete compiler;
    }
}
