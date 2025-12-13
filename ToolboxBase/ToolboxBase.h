#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ToolboxBase.h"

class ToolboxBase : public QMainWindow
{
    Q_OBJECT

public:
    ToolboxBase(QWidget *parent = nullptr);
    ~ToolboxBase();

private:
    Ui::ToolboxBaseClass ui;
};

