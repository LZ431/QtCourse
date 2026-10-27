#include "calculatorwindow.h"

#include "ui_calculatorwindow.h"

#include <QKeyEvent>
#include <QPushButton>

CalculatorWindow::CalculatorWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CalculatorWindow)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    connectMouseButtons();
    refreshDisplay();
}

CalculatorWindow::~CalculatorWindow()
{
    delete ui;
}

void CalculatorWindow::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void CalculatorWindow::connectMouseButtons()
{
    const struct DigitButton {
        QPushButton *button;
        int digit;
    } digitButtons[] = {
        {ui->digit0Button, 0},
        {ui->digit1Button, 1},
        {ui->digit2Button, 2},
        {ui->digit3Button, 3},
        {ui->digit4Button, 4},
        {ui->digit5Button, 5},
        {ui->digit6Button, 6},
        {ui->digit7Button, 7},
        {ui->digit8Button, 8},
        {ui->digit9Button, 9},
    };

    for (const DigitButton &entry : digitButtons) {
        entry.button->setFocusPolicy(Qt::NoFocus);
        connect(entry.button, &QPushButton::clicked, this,
                [this, digit = entry.digit] { inputDigit(digit); });
    }

    const struct OperatorButton {
        QPushButton *button;
        QChar operation;
    } operatorButtons[] = {
        {ui->addButton, QLatin1Char('+')},
        {ui->subtractButton, QLatin1Char('-')},
        {ui->multiplyButton, QLatin1Char('*')},
        {ui->divideButton, QLatin1Char('/')},
    };

    for (const OperatorButton &entry : operatorButtons) {
        entry.button->setFocusPolicy(Qt::NoFocus);
        connect(entry.button, &QPushButton::clicked, this,
                [this, operation = entry.operation] { selectOperator(operation); });
    }

    for (QPushButton *button : {
             ui->clearButton,
             ui->backspaceButton,
             ui->decimalButton,
             ui->equalsButton,
         }) {
        button->setFocusPolicy(Qt::NoFocus);
    }

    connect(ui->clearButton, &QPushButton::clicked, this, &CalculatorWindow::clearAll);
    connect(ui->backspaceButton, &QPushButton::clicked,
            this, &CalculatorWindow::backspace);
    connect(ui->decimalButton, &QPushButton::clicked,
            this, &CalculatorWindow::inputDecimalPoint);
    connect(ui->equalsButton, &QPushButton::clicked,
            this, &CalculatorWindow::evaluate);
}

void CalculatorWindow::inputDigit(int digit)
{
    m_logic.inputDigit(digit);
    refreshDisplay();
}

void CalculatorWindow::inputDecimalPoint()
{
    m_logic.inputDecimalPoint();
    refreshDisplay();
}

void CalculatorWindow::backspace()
{
    m_logic.backspace();
    refreshDisplay();
}

void CalculatorWindow::clearAll()
{
    m_logic.clear();
    refreshDisplay();
}

void CalculatorWindow::selectOperator(QChar operation)
{
    m_logic.selectOperator(operation);
    refreshDisplay();
}

void CalculatorWindow::evaluate()
{
    m_logic.evaluate();
    refreshDisplay();
}

void CalculatorWindow::refreshDisplay()
{
    ui->displayLabel->setText(m_logic.displayText());
    ui->expressionLabel->setText(m_logic.expressionText());
}

