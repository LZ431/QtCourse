#include "mainwindow.h"
#include "codeeditor.h"

#include <QAction>
#include <QClipboard>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTextCursor>
#include <QtTest>

class EditorTests final : public QObject
{
    Q_OBJECT

private slots:
    void saveAndReopenRoundTrip();
    void rejectsMissingAndBinaryFiles();
    void searchesForwardBackwardAndWraps();
    void replaceAllHandlesCaseAndMissingText();
    void lineNumberVisibilityChangesViewportMargin();
    void editActionsFollowSelectionAndHistory();
};

void EditorTests::saveAndReopenRoundTrip()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());
    const QString path =
        temporaryDirectory.filePath(QStringLiteral("章节-01.txt"));

    MainWindow window;
    window.editor()->setPlainText(
        QStringLiteral("第一行\n第二行\nUTF-8：文本编辑器"));
    window.editor()->document()->setModified(true);
    QVERIFY(window.hasUnsavedChanges());

    QString error;
    QVERIFY2(window.saveFileToPath(path, &error), qPrintable(error));
    QVERIFY(QFile::exists(path));
    QCOMPARE(window.currentFilePath(), QFileInfo(path).absoluteFilePath());
    QVERIFY(!window.hasUnsavedChanges());

    window.editor()->clear();
    QVERIFY2(window.loadFileFromPath(path, &error), qPrintable(error));
    QCOMPARE(window.plainText(),
             QStringLiteral("第一行\n第二行\nUTF-8：文本编辑器"));
    QCOMPARE(window.editor()->blockCount(), 3);
    QVERIFY(!window.hasUnsavedChanges());
}

void EditorTests::rejectsMissingAndBinaryFiles()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    MainWindow window;
    QString error;
    QVERIFY(!window.loadFileFromPath(
        temporaryDirectory.filePath(QStringLiteral("missing.txt")), &error));
    QVERIFY(error.contains(QStringLiteral("不存在")));

    const QString binaryPath =
        temporaryDirectory.filePath(QStringLiteral("invalid.dat"));
    QFile binaryFile(binaryPath);
    QVERIFY(binaryFile.open(QIODevice::WriteOnly));
    binaryFile.write(QByteArray("text\0binary", 11));
    binaryFile.close();

    error.clear();
    QVERIFY(!window.loadFileFromPath(binaryPath, &error));
    QVERIFY(error.contains(QStringLiteral("二进制")));
    QCOMPARE(window.plainText(), QString());
}

void EditorTests::searchesForwardBackwardAndWraps()
{
    MainWindow window;
    window.editor()->setPlainText(
        QStringLiteral("Alpha one\nbeta Alpha\ngamma"));

    QTextCursor cursor = window.editor()->textCursor();
    cursor.movePosition(QTextCursor::End);
    window.editor()->setTextCursor(cursor);

    QVERIFY(window.findText(QStringLiteral("Alpha"),
                            Qt::CaseSensitive, true, false));
    QCOMPARE(window.editor()->textCursor().selectionStart(), 0);
    QCOMPARE(window.editor()->textCursor().selectedText(),
             QStringLiteral("Alpha"));

    QVERIFY(window.findText(QStringLiteral("ALPHA"),
                            Qt::CaseInsensitive, true, false));
    QCOMPARE(window.editor()->textCursor().selectionStart(), 15);

    QVERIFY(window.findText(QStringLiteral("Alpha"),
                            Qt::CaseSensitive, false, false));
    QCOMPARE(window.editor()->textCursor().selectionStart(), 0);
}

void EditorTests::replaceAllHandlesCaseAndMissingText()
{
    MainWindow window;
    window.editor()->setPlainText(
        QStringLiteral("Alpha alpha ALPHA\n完成"));

    QCOMPARE(window.replaceAllText(QStringLiteral("alpha"),
                                   QStringLiteral("文本"),
                                   Qt::CaseInsensitive, false),
             3);
    QCOMPARE(window.plainText(), QStringLiteral("文本 文本 文本\n完成"));
    QVERIFY(window.hasUnsavedChanges());

    QCOMPARE(window.replaceAllText(QStringLiteral("不存在"),
                                   QStringLiteral("x"),
                                   Qt::CaseInsensitive, false),
             0);
}

void EditorTests::lineNumberVisibilityChangesViewportMargin()
{
    MainWindow window;
    window.show();
    QCoreApplication::processEvents();

    CodeEditor *editor = window.editor();
    QVERIFY(editor->lineNumbersVisible());
    QVERIFY(editor->lineNumberAreaWidth() > 0);
    const int visibleMargin = editor->viewport()->pos().x();
    QVERIFY(visibleMargin > 0);

    editor->setLineNumbersVisible(false);
    QVERIFY(!editor->lineNumbersVisible());
    QCOMPARE(editor->lineNumberAreaWidth(), 0);
    QVERIFY(editor->viewport()->pos().x() < visibleMargin);
}

void EditorTests::editActionsFollowSelectionAndHistory()
{
    MainWindow window;
    window.show();
    window.editor()->insertPlainText(QStringLiteral("可撤销的文本"));
    QCoreApplication::processEvents();

    auto *copyAction = window.findChild<QAction *>(QStringLiteral("actionCopy"));
    auto *cutAction = window.findChild<QAction *>(QStringLiteral("actionCut"));
    auto *undoAction = window.findChild<QAction *>(QStringLiteral("actionUndo"));
    auto *redoAction = window.findChild<QAction *>(QStringLiteral("actionRedo"));
    QVERIFY(copyAction);
    QVERIFY(cutAction);
    QVERIFY(undoAction);
    QVERIFY(redoAction);

    window.editor()->selectAll();
    QCoreApplication::processEvents();
    QVERIFY(copyAction->isEnabled());
    QVERIFY(cutAction->isEnabled());
    QVERIFY(undoAction->isEnabled());

    cutAction->trigger();
    QCoreApplication::processEvents();
    QCOMPARE(window.plainText(), QString());
    QVERIFY(undoAction->isEnabled());
    undoAction->trigger();
    QCoreApplication::processEvents();
    QCOMPARE(window.plainText(), QStringLiteral("可撤销的文本"));
    QVERIFY(redoAction->isEnabled());
    redoAction->trigger();
    QCoreApplication::processEvents();
    QCOMPARE(window.plainText(), QString());
}

QTEST_MAIN(EditorTests)

#include "test_editor.moc"

