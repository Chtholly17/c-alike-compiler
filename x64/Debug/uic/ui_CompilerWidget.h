/********************************************************************************
** Form generated from reading UI file 'CompilerWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMPILERWIDGET_H
#define UI_COMPILERWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CompilerWidgetClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *CompilerWidgetClass)
    {
        if (CompilerWidgetClass->objectName().isEmpty())
            CompilerWidgetClass->setObjectName(QString::fromUtf8("CompilerWidgetClass"));
        CompilerWidgetClass->resize(600, 400);
        menuBar = new QMenuBar(CompilerWidgetClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        CompilerWidgetClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(CompilerWidgetClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        CompilerWidgetClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(CompilerWidgetClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        CompilerWidgetClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(CompilerWidgetClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        CompilerWidgetClass->setStatusBar(statusBar);

        retranslateUi(CompilerWidgetClass);

        QMetaObject::connectSlotsByName(CompilerWidgetClass);
    } // setupUi

    void retranslateUi(QMainWindow *CompilerWidgetClass)
    {
        CompilerWidgetClass->setWindowTitle(QCoreApplication::translate("CompilerWidgetClass", "CompilerWidget", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CompilerWidgetClass: public Ui_CompilerWidgetClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMPILERWIDGET_H
