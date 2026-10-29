#include "mainwindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("小牛文本编辑器"));
    application.setApplicationDisplayName(QStringLiteral("小牛文本编辑器"));
    application.setOrganizationName(QStringLiteral("DGUT"));
    application.setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));

    MainWindow window;
    window.show();
    return application.exec();
}

