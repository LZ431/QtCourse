#include "aboutdialog.h"
#include "codeeditor.h"
#include "finddialog.h"
#include "mainwindow.h"
#include "replacedialog.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QPointer>
#include <QStatusBar>
#include <QTextCursor>

namespace {
void settleEvents(int rounds = 4)
{
    for (int index = 0; index < rounds; ++index) {
        QCoreApplication::processEvents();
    }
}

void saveWidget(QWidget *widget, const QString &path)
{
    settleEvents();
    widget->grab().save(path);
}

void saveMenu(MainWindow &window, const QString &objectName,
              const QString &path)
{
    auto *menu = window.findChild<QMenu *>(objectName);
    if (!menu) {
        return;
    }
    menu->popup(window.mapToGlobal(QPoint(12, 52)));
    settleEvents();
    menu->grab().save(path);
    menu->hide();
    QApplication::sync();
}
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    const QString outputDirectory = argc > 1
                                        ? QString::fromLocal8Bit(argv[1])
                                        : QDir::current().filePath(
                                              QStringLiteral("docs/screenshots"));
    QDir().mkpath(outputDirectory);

    MainWindow window;
    window.resize(900, 620);
    window.editor()->setPlainText(
        QStringLiteral(
            "小牛文本编辑器\n"
            "\n"
            "这是一个使用 Qt Widgets 完成的简单文本编辑器。\n"
            "它支持文件读写、格式设置、查找替换和行号显示。\n"
            "\n"
            "编辑提示：\n"
            "1. 新建或打开文档后即可开始编辑；\n"
            "2. 修改内容后窗口标题会显示 *；\n"
            "3. 关闭文档前会检查是否需要保存。\n"
            "\n"
            "当前实验：Qt 应用程序开发 · 实验二\n"));
    window.editor()->document()->setModified(false);
    QTextCursor cursor = window.editor()->textCursor();
    cursor.movePosition(QTextCursor::End);
    window.editor()->setTextCursor(cursor);
    window.show();
    settleEvents(8);

    saveWidget(&window, QDir(outputDirectory).filePath(
                            QStringLiteral("editor-main.png")));

    saveMenu(window, QStringLiteral("menuFile"),
             QDir(outputDirectory).filePath(QStringLiteral("menu-file.png")));
    saveMenu(window, QStringLiteral("menuEdit"),
             QDir(outputDirectory).filePath(QStringLiteral("menu-edit.png")));
    saveMenu(window, QStringLiteral("menuFormat"),
             QDir(outputDirectory).filePath(QStringLiteral("menu-format.png")));
    saveMenu(window, QStringLiteral("menuView"),
             QDir(outputDirectory).filePath(QStringLiteral("menu-view.png")));
    saveMenu(window, QStringLiteral("menuHelp"),
             QDir(outputDirectory).filePath(QStringLiteral("menu-help.png")));

    FindDialog *findDialog = window.findDialogWidget();
    findDialog->setSearchText(QStringLiteral("文本编辑器"));
    findDialog->show();
    settleEvents();
    saveWidget(findDialog, QDir(outputDirectory).filePath(
                               QStringLiteral("dialog-find.png")));

    ReplaceDialog *replaceDialog = window.replaceDialogWidget();
    replaceDialog->setSearchText(QStringLiteral("文本编辑器"));
    replaceDialog->show();
    settleEvents();
    saveWidget(replaceDialog, QDir(outputDirectory).filePath(
                                  QStringLiteral("dialog-replace.png")));

    AboutDialog *aboutDialog = window.aboutDialogWidget();
    aboutDialog->show();
    settleEvents();
    saveWidget(aboutDialog, QDir(outputDirectory).filePath(
                                QStringLiteral("dialog-about.png")));

    replaceDialog->hide();
    findDialog->hide();
    aboutDialog->hide();
    window.editor()->selectAll();
    settleEvents();
    saveWidget(&window, QDir(outputDirectory).filePath(
                            QStringLiteral("selection-actions.png")));

    qInfo().noquote()
        << QStringLiteral("Screenshots written to %1").arg(outputDirectory);
    return 0;
}

