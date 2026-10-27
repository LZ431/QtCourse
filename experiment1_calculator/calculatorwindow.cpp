#include "calculatorwindow.h"

#include "ui_calculatorwindow.h"

#include <QKeyEvent>
#include <QPushButton>

namespace {
QChar normalizeOperator(QChar character)
{
    if (character == QChar(0x00D7) || character == QLatin1Char('x')
        || character == QLatin1Char('X')) {
        return QLatin1Char('*');
    }
    if (character == QChar(0x00F7)) {
        return QLatin1Char('/');
    }
    if (character == QChar(0x2212)) {
        return QLatin1Char('-');
    }
    if (character == QChar(0xFF0B)) {
        return QLatin1Char('+');
    }
    return character;
}
}

CalculatorWindow::CalculatorWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CalculatorWindow)
{
    ui->setupUi(this);
    setStyleSheet(QStringLiteral(R"(
        QWidget#CalculatorWindow {
            background: #eef4fa;
        }
        QFrame#displayFrame {
            background: #ffffff;
            border: 1px solid #dbe6f0;
            border-radius: 24px;
        }
        QLabel#expressionLabel {
            background: transparent;
            color: #718397;
            font-size: 18px;
            padding: 1px 4px;
        }
        QLabel#displayLabel {
            background: transparent;
            color: #142b3d;
            font-size: 42px;
            font-weight: 600;
            padding: 2px 4px;
        }
        QPushButton {
            background: #ffffff;
            border: 1px solid #e1eaf2;
            border-radius: 16px;
            color: #1d3345;
            font-size: 22px;
            font-weight: 500;
        }
        QPushButton:hover {
            background: #f6faff;
            border-color: #b9d7ef;
        }
        QPushButton:pressed {
            background: #dcecf9;
        }
        QPushButton#clearButton,
        QPushButton#backspaceButton {
            background: #e8f2fb;
            border-color: #d6e7f6;
            color: #1d79c4;
            font-size: 20px;
        }
        QPushButton#divideButton,
        QPushButton#multiplyButton,
        QPushButton#subtractButton,
        QPushButton#addButton {
            background: #f1f7fd;
            color: #1074c8;
            font-size: 26px;
        }
        QPushButton#equalsButton {
            background: #1c84db;
            border-color: #1c84db;
            color: #ffffff;
            font-size: 27px;
            font-weight: 600;
        }
        QPushButton#equalsButton:hover {
            background: #1675c5;
        }
        QPushButton#equalsButton:pressed {
            background: #1265a9;
        }
    )"));
    setFocusPolicy(Qt::StrongFocus);
    connectMouseButtons();
    refreshDisplay();
    setFocus(Qt::OtherFocusReason);
}

CalculatorWindow::~CalculatorWindow()
{
    delete ui;
}

void CalculatorWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier)
        || event->modifiers().testFlag(Qt::AltModifier)
        || event->modifiers().testFlag(Qt::MetaModifier)) {
        QWidget::keyPressEvent(event);
        return;
    }

    if (event->isAutoRepeat()) {
        event->accept();
        return;
    }

    const QString text = event->text();
    if (!text.isEmpty()) {
        const QChar character = text.front();
        const int digit = character.digitValue();
        if (digit >= 0 && digit <= 9) {
            inputDigit(digit);
            event->accept();
            return;
        }

        if (character == QLatin1Char('.') || character == QChar(0xFF0E)) {
            inputDecimalPoint();
            event->accept();
            return;
        }

        const QChar operation = normalizeOperator(character);
        if (QStringLiteral("+-*/").contains(operation)) {
            selectOperator(operation);
            event->accept();
            return;
        }

        if (character == QLatin1Char('=')) {
            evaluate();
            event->accept();
            return;
        }

        if (character == QLatin1Char('c') || character == QLatin1Char('C')) {
            clearAll();
            event->accept();
            return;
        }
    }

    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Equal:
        evaluate();
        event->accept();
        return;
    case Qt::Key_Backspace:
        backspace();
        event->accept();
        return;
    case Qt::Key_Escape:
    case Qt::Key_Delete:
        clearAll();
        event->accept();
        return;
    default:
        break;
    }

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

