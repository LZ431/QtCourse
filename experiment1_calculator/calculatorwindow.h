#pragma once

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
    Ui::CalculatorWindow *ui;
};

