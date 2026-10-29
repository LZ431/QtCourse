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
            "西游记  第一回  灵根育孕源流出  心性修持大道生\n"
            "\n"
            "诗曰：混沌未分天地乱，茫茫渺渺无人见。"
            "自从盘古破鸿蒙，开辟从兹清浊辨。"
            "覆载群生仰至仁，发明万物皆成善。"
            "欲知造化会元功，须看西游释厄传。"
            "盖闻天地之数，有十二万九千六百岁为一元。"
            "将一元分为十二会，乃子、丑、寅、卯、辰、巳、午、未、申、酉、戌、亥"
            "之十二支也。每会该一万八百岁。且就一日而论：子时得阳气，而丑则鸡鸣；"
            "寅不通光，而卯则日出；辰时食后，而巳则挨排；日午天中，而未则西蹉；"
            "申时晡，而日落酉；戌黄昏，而人定亥。譬于大数，若到戌会之终，"
            "则天地昏蒙而万物否矣。再去五千四百岁，交亥会之初，则当黑暗，"
            "而两间人物俱无矣，故曰混沌。又五千四百岁，亥会将终，贞下起元，"
            "近子之会，而复逐渐开明。邵康节曰：冬至子之半，天心无改移。"
            "一阳初动处，万物未生时。到此，天始有根。再五千四百岁，正当子会，"
            "轻清上腾，有日，有月，有星，有辰。日、月、星、辰，谓之四象。"
            "故曰，天开于子。又经五千四百岁，子会将终，近丑之会，而逐渐坚实。"
            "《易》曰：大哉乾元！至哉坤元！万物资生，乃顺承天。至此，地始凝结。"
            "再五千四百岁，正当丑会，重浊下凝，有水，有火，有山，有石，有土。"
            "水、火、山、石、土，谓之五形。故曰，地辟于丑。"));
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
    findDialog->setSearchText(QStringLiteral("混沌"));
    findDialog->show();
    settleEvents();
    saveWidget(findDialog, QDir(outputDirectory).filePath(
                               QStringLiteral("dialog-find.png")));

    ReplaceDialog *replaceDialog = window.replaceDialogWidget();
    replaceDialog->setSearchText(QStringLiteral("天地"));
    replaceDialog->setReplacementText(QStringLiteral("乾坤"));
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

