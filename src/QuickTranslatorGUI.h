#pragma once

#include <QtWidgets/QWidget>
#include "ui_QuickTranslatorGUI.h"

class QuickTranslatorGUI : public QWidget
{
    Q_OBJECT

public:
    QuickTranslatorGUI(QWidget *parent = nullptr);
    ~QuickTranslatorGUI();

private:
    Ui::QuickTranslatorGUIClass ui;
};
