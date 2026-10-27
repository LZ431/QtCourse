#pragma once

#include "calculatorlogic.h"

#include <QChar>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class CalculatorWindow;
}
QT_END_NAMESPACE

class CalculatorWindow final : public QWidget
{
    Q_OBJECT

public:
    explicit CalculatorWindow(QWidget *parent = nullptr);
    ~CalculatorWindow() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void connectMouseButtons();
    void inputDigit(int digit);
    void inputDecimalPoint();
    void backspace();
    void clearAll();
    void selectOperator(QChar operation);
    void evaluate();
    void refreshDisplay();

    Ui::CalculatorWindow *ui;
    CalculatorLogic m_logic;
};

