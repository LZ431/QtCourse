#include "calculatorwindow.h"

#include <QApplication>
#include <QDir>
#include <QKeyEvent>

namespace {
void sendText(CalculatorWindow &window, const QString &text)
{
    for (const QChar character : text) {
        const QString characterText(character);
        QKeyEvent event(QEvent::KeyPress, 0, Qt::NoModifier, characterText);
        QApplication::sendEvent(&window, &event);
    }
}

void pressKey(CalculatorWindow &window, int key)
{
    QKeyEvent event(QEvent::KeyPress, key, Qt::NoModifier);
    QApplication::sendEvent(&window, &event);
}

bool saveScreenshot(CalculatorWindow &window, const QString &path)
{
    QApplication::processEvents();
    return window.grab().save(path, "PNG");
}
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    const QString outputDirectory =
        application.arguments().value(1, QStringLiteral("captures"));

    if (!QDir().mkpath(outputDirectory)) {
        qCritical("Cannot create screenshot directory: %s",
                  qPrintable(outputDirectory));
        return 1;
    }

    CalculatorWindow window;
    window.resize(420, 620);
    window.show();
    QApplication::processEvents();

    sendText(window, QStringLiteral("845+996="));
    if (!saveScreenshot(window, outputDirectory + QStringLiteral("/calculator-main.png"))) {
        return 2;
    }

    pressKey(window, Qt::Key_Escape);
    sendText(window, QStringLiteral("12.5*2.4="));
    if (!saveScreenshot(window, outputDirectory + QStringLiteral("/calculator-decimal.png"))) {
        return 3;
    }

    pressKey(window, Qt::Key_Escape);
    sendText(window, QStringLiteral("9/0="));
    if (!saveScreenshot(window, outputDirectory + QStringLiteral("/calculator-error.png"))) {
        return 4;
    }

    return 0;
}
