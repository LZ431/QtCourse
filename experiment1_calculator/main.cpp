#include "calculatorwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QApplication::setApplicationName(QStringLiteral("带键盘事件的计算器"));
    QApplication::setOrganizationName(QStringLiteral("DGUT"));

    CalculatorWindow window;
    window.show();

    return application.exec();
}

