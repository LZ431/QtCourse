#pragma once

#include <QChar>
#include <QString>

class CalculatorLogic
{
public:
    void clear();
    void inputDigit(int digit);
    void inputDecimalPoint();
    void backspace();
    bool selectOperator(QChar operation);
    void evaluate();

    [[nodiscard]] QString displayText() const;
    [[nodiscard]] QString expressionText() const;
    [[nodiscard]] bool hasError() const;
    [[nodiscard]] bool hasPendingOperation() const;

    static QString operatorSymbol(QChar operation);

private:
    static constexpr int maximumInputDigits = 15;

    void prepareForNewEntry();
    void updatePendingExpression();
    bool calculate(double rightOperand);
    double currentValue() const;
    static QString formatNumber(double value);

    QString m_display = QStringLiteral("0");
    QString m_expression;
    QString m_leftOperandText;
    double m_accumulator = 0.0;
    QChar m_pendingOperator;
    bool m_waitingForOperand = false;
    bool m_justEvaluated = false;
    bool m_error = false;
};

