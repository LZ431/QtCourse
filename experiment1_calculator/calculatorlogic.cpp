#include "calculatorlogic.h"

#include <QRegularExpression>
#include <QtGlobal>

#include <cmath>

void CalculatorLogic::clear()
{
    m_display = QStringLiteral("0");
    m_expression.clear();
    m_leftOperandText.clear();
    m_accumulator = 0.0;
    m_pendingOperator = QChar();
    m_waitingForOperand = false;
    m_justEvaluated = false;
    m_error = false;
}

void CalculatorLogic::inputDigit(int digit)
{
    if (digit < 0 || digit > 9) {
        return;
    }

    if (m_error) {
        clear();
    }

    if (m_waitingForOperand || m_justEvaluated) {
        prepareForNewEntry();
        m_display = QString::number(digit);
    } else {
        const int currentDigits = m_display.count(QRegularExpression(QStringLiteral("[0-9]")));
        if (currentDigits >= maximumInputDigits) {
            return;
        }

        if (m_display == QStringLiteral("0")) {
            m_display = QString::number(digit);
        } else {
            m_display.append(QString::number(digit));
        }
    }

    updatePendingExpression();
}

void CalculatorLogic::inputDecimalPoint()
{
    if (m_error) {
        clear();
    }

    if (m_waitingForOperand || m_justEvaluated) {
        prepareForNewEntry();
        m_display = QStringLiteral("0.");
    } else if (!m_display.contains(QLatin1Char('.'))) {
        m_display.append(QLatin1Char('.'));
    }

    updatePendingExpression();
}

void CalculatorLogic::backspace()
{
    if (m_error) {
        clear();
        return;
    }

    if (m_justEvaluated) {
        clear();
        return;
    }

    if (m_waitingForOperand) {
        return;
    }

    m_display.chop(1);
    if (m_display.isEmpty() || m_display == QStringLiteral("-")) {
        m_display = QStringLiteral("0");
    }

    updatePendingExpression();
}

bool CalculatorLogic::selectOperator(QChar operation)
{
    if (m_error || !QStringLiteral("+-*/").contains(operation)) {
        return false;
    }

    if (!m_pendingOperator.isNull()) {
        if (m_waitingForOperand) {
            m_pendingOperator = operation;
            updatePendingExpression();
            return true;
        }

        if (!calculate(currentValue())) {
            return false;
        }
    } else {
        m_accumulator = currentValue();
        m_leftOperandText = m_display;
        m_justEvaluated = false;
    }

    m_pendingOperator = operation;
    m_leftOperandText = formatNumber(m_accumulator);
    m_waitingForOperand = true;
    updatePendingExpression();
    return true;
}

void CalculatorLogic::evaluate()
{
    if (m_error || m_pendingOperator.isNull()) {
        return;
    }

    const QString leftText = m_leftOperandText;
    const QChar operation = m_pendingOperator;
    const double rightOperand = m_waitingForOperand ? m_accumulator : currentValue();
    const QString rightText =
        m_waitingForOperand ? formatNumber(rightOperand) : m_display;
    const QString completedExpression =
        leftText + QLatin1Char(' ') + operatorSymbol(operation) + QLatin1Char(' ')
        + rightText + QStringLiteral(" =");

    if (!calculate(rightOperand)) {
        m_expression = completedExpression;
        return;
    }

    m_expression = completedExpression;
    m_pendingOperator = QChar();
    m_waitingForOperand = true;
    m_justEvaluated = true;
}

QString CalculatorLogic::displayText() const
{
    return m_display;
}

QString CalculatorLogic::expressionText() const
{
    return m_expression;
}

bool CalculatorLogic::hasError() const
{
    return m_error;
}

bool CalculatorLogic::hasPendingOperation() const
{
    return !m_pendingOperator.isNull();
}

QString CalculatorLogic::operatorSymbol(QChar operation)
{
    if (operation == QLatin1Char('*')) {
        return QStringLiteral("×");
    }
    if (operation == QLatin1Char('/')) {
        return QStringLiteral("÷");
    }
    if (operation == QLatin1Char('-')) {
        return QStringLiteral("−");
    }
    return QString(operation);
}

void CalculatorLogic::prepareForNewEntry()
{
    m_display = QStringLiteral("0");
    m_waitingForOperand = false;
    m_justEvaluated = false;

    if (m_pendingOperator.isNull()) {
        m_expression.clear();
    }
}

void CalculatorLogic::updatePendingExpression()
{
    if (m_pendingOperator.isNull()) {
        return;
    }

    m_expression = m_leftOperandText + QLatin1Char(' ')
        + operatorSymbol(m_pendingOperator);
    if (!m_waitingForOperand) {
        m_expression += QLatin1Char(' ') + m_display;
    }
}

bool CalculatorLogic::calculate(double rightOperand)
{
    double result = 0.0;

    switch (m_pendingOperator.toLatin1()) {
    case '+':
        result = m_accumulator + rightOperand;
        break;
    case '-':
        result = m_accumulator - rightOperand;
        break;
    case '*':
        result = m_accumulator * rightOperand;
        break;
    case '/':
        if (qFuzzyIsNull(rightOperand)) {
            m_display = QStringLiteral("错误");
            m_accumulator = 0.0;
            m_pendingOperator = QChar();
            m_waitingForOperand = true;
            m_justEvaluated = true;
            m_error = true;
            return false;
        }
        result = m_accumulator / rightOperand;
        break;
    default:
        return false;
    }

    if (!std::isfinite(result)) {
        m_display = QStringLiteral("数值超出范围");
        m_accumulator = 0.0;
        m_pendingOperator = QChar();
        m_waitingForOperand = true;
        m_justEvaluated = true;
        m_error = true;
        return false;
    }

    m_accumulator = result;
    m_display = formatNumber(result);
    m_leftOperandText = m_display;
    m_waitingForOperand = false;
    return true;
}

double CalculatorLogic::currentValue() const
{
    bool ok = false;
    const double value = m_display.toDouble(&ok);
    return ok ? value : 0.0;
}

QString CalculatorLogic::formatNumber(double value)
{
    if (qFuzzyIsNull(value)) {
        value = 0.0;
    }

    QString text = QString::number(value, 'g', 15);
    text.replace(QLatin1Char('e'), QLatin1Char('E'));
    return text;
}

