#include "calculatorlogic.h"
#include "calculatorwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QtTest>

namespace {
void enterNumber(CalculatorLogic &calculator, const QString &number)
{
    for (const QChar character : number) {
        if (character == QLatin1Char('.')) {
            calculator.inputDecimalPoint();
        } else {
            calculator.inputDigit(character.digitValue());
        }
    }
}

void enterOperation(CalculatorLogic &calculator, const QString &sequence)
{
    for (const QChar character : sequence) {
        if (character.isDigit() || character == QLatin1Char('.')) {
            if (character == QLatin1Char('.')) {
                calculator.inputDecimalPoint();
            } else {
                calculator.inputDigit(character.digitValue());
            }
        } else if (character == QLatin1Char('=')) {
            calculator.evaluate();
        } else {
            calculator.selectOperator(character);
        }
    }
}
}

class CalculatorTest final : public QObject
{
    Q_OBJECT

private slots:
    void arithmetic_data();
    void arithmetic();
    void repeatedDecimalPointIsIgnored();
    void repeatedOperatorReplacesPreviousOperator();
    void operationsEvaluateContinuously();
    void resultCanStartNewCalculation();
    void resultCanContinueWithAnotherOperator();
    void divisionByZeroCanRecover();
    void equalsWithoutRightOperandReusesLeftValue();
    void leadingZeroAndBackspaceAreHandled();
    void keyboardAndMouseUseConsistentResults();
    void keyboardClearAndBackspaceWork();
};

void CalculatorTest::arithmetic_data()
{
    QTest::addColumn<double>("left");
    QTest::addColumn<QChar>("operation");
    QTest::addColumn<double>("right");
    QTest::addColumn<QString>("expected");

    QTest::newRow("addition") << 2.5 << QChar('+') << 1.25 << QStringLiteral("3.75");
    QTest::newRow("subtraction") << 8.0 << QChar('-') << 3.0 << QStringLiteral("5");
    QTest::newRow("multiplication") << 1.5 << QChar('*') << 4.0 << QStringLiteral("6");
    QTest::newRow("division") << 9.0 << QChar('/') << 4.0 << QStringLiteral("2.25");
}

void CalculatorTest::arithmetic()
{
    QFETCH(double, left);
    QFETCH(QChar, operation);
    QFETCH(double, right);
    QFETCH(QString, expected);

    CalculatorLogic calculator;
    enterNumber(calculator, QString::number(left));
    calculator.selectOperator(operation);
    enterNumber(calculator, QString::number(right));
    calculator.evaluate();

    QCOMPARE(calculator.displayText(), expected);
    QVERIFY(!calculator.hasError());
}

void CalculatorTest::repeatedDecimalPointIsIgnored()
{
    CalculatorLogic calculator;
    enterOperation(calculator, QStringLiteral("12.3.4"));

    QCOMPARE(calculator.displayText(), QStringLiteral("12.34"));
}

void CalculatorTest::repeatedOperatorReplacesPreviousOperator()
{
    CalculatorLogic calculator;
    enterOperation(calculator, QStringLiteral("8+*2="));

    QCOMPARE(calculator.displayText(), QStringLiteral("16"));
    QCOMPARE(calculator.expressionText(), QStringLiteral("8 × 2 ="));
}

void CalculatorTest::operationsEvaluateContinuously()
{
    CalculatorLogic calculator;
    enterOperation(calculator, QStringLiteral("6+3*2="));

    QCOMPARE(calculator.displayText(), QStringLiteral("18"));
}

void CalculatorTest::resultCanStartNewCalculation()
{
    CalculatorLogic calculator;
    enterOperation(calculator, QStringLiteral("2+3="));
    calculator.inputDigit(8);

    QCOMPARE(calculator.displayText(), QStringLiteral("8"));
    QCOMPARE(calculator.expressionText(), QString());
}

void CalculatorTest::resultCanContinueWithAnotherOperator()
{
    CalculatorLogic calculator;
    enterOperation(calculator, QStringLiteral("2+3=*2="));

    QCOMPARE(calculator.displayText(), QStringLiteral("10"));
    QCOMPARE(calculator.expressionText(), QStringLiteral("5 × 2 ="));
}

void CalculatorTest::divisionByZeroCanRecover()
{
    CalculatorLogic calculator;
    enterOperation(calculator, QStringLiteral("5/0="));

    QVERIFY(calculator.hasError());
    QCOMPARE(calculator.displayText(), QStringLiteral("错误"));
    QCOMPARE(calculator.expressionText(), QStringLiteral("5 ÷ 0 ="));

    calculator.inputDigit(7);
    QVERIFY(!calculator.hasError());
    QCOMPARE(calculator.displayText(), QStringLiteral("7"));
}

void CalculatorTest::equalsWithoutRightOperandReusesLeftValue()
{
    CalculatorLogic calculator;
    enterOperation(calculator, QStringLiteral("5+="));

    QCOMPARE(calculator.displayText(), QStringLiteral("10"));
}

void CalculatorTest::leadingZeroAndBackspaceAreHandled()
{
    CalculatorLogic calculator;
    enterOperation(calculator, QStringLiteral("0007.5"));
    QCOMPARE(calculator.displayText(), QStringLiteral("7.5"));

    calculator.backspace();
    QCOMPARE(calculator.displayText(), QStringLiteral("7."));

    calculator.backspace();
    QCOMPARE(calculator.displayText(), QStringLiteral("7"));

    calculator.clear();
    QCOMPARE(calculator.displayText(), QStringLiteral("0"));
    QCOMPARE(calculator.expressionText(), QString());
}

void CalculatorTest::keyboardAndMouseUseConsistentResults()
{
    CalculatorWindow window;
    window.show();
    window.activateWindow();
    window.setFocus();
    QApplication::processEvents();

    auto *display = window.findChild<QLabel *>(QStringLiteral("displayLabel"));
    auto *multiply = window.findChild<QPushButton *>(QStringLiteral("multiplyButton"));
    QVERIFY(display);
    QVERIFY(multiply);

    QTest::keyClicks(&window, QStringLiteral("12.5"));
    QTest::keyClick(&window, Qt::Key_Plus);
    QTest::keyClicks(&window, QStringLiteral("2"));
    QTest::keyClick(&window, Qt::Key_Return);
    QCOMPARE(display->text(), QStringLiteral("14.5"));

    QTest::mouseClick(multiply, Qt::LeftButton);
    QTest::keyClicks(&window, QStringLiteral("2"));
    QTest::keyClick(&window, Qt::Key_Return);
    QCOMPARE(display->text(), QStringLiteral("29"));
}

void CalculatorTest::keyboardClearAndBackspaceWork()
{
    CalculatorWindow window;
    window.show();
    window.activateWindow();
    window.setFocus();
    QApplication::processEvents();

    auto *display = window.findChild<QLabel *>(QStringLiteral("displayLabel"));
    QVERIFY(display);

    QTest::keyClicks(&window, QStringLiteral("123"));
    QTest::keyClick(&window, Qt::Key_Backspace);
    QCOMPARE(display->text(), QStringLiteral("12"));

    QTest::keyClick(&window, Qt::Key_Escape);
    QCOMPARE(display->text(), QStringLiteral("0"));
}

QTEST_MAIN(CalculatorTest)
#include "test_calculator.moc"
