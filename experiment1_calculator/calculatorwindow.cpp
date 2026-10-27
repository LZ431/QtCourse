#include "calculatorwindow.h"

#include "ui_calculatorwindow.h"

#include <QKeyEvent>

CalculatorWindow::CalculatorWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CalculatorWindow)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
}

CalculatorWindow::~CalculatorWindow()
{
    delete ui;
}

void CalculatorWindow::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

