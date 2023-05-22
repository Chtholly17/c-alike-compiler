#include "CompilerWidget.h"
#include <QtWidgets/QApplication>
#include "Compiler.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CompilerWidget w;
    w.show();
    return a.exec();
}


// test origin main
//int main() {
//    Compiler compiler = Compiler("productions.txt");
//    compiler.compile("test.txt", "program.asm");
//
//    return 0;
//}
