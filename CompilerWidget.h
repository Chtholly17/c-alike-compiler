#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CompilerWidget.h"

class CompilerWidget : public QMainWindow
{
    Q_OBJECT

public:
    CompilerWidget(QWidget *parent = nullptr);
    ~CompilerWidget();

private:
    Ui::CompilerWidgetClass ui;
};
