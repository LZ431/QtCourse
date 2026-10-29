#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutdialog.h"
#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"

#include <QAction>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QGuiApplication>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QMessageBox>
#include <QSaveFile>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>

#include <algorithm>

namespace {
constexpr qint64 MaximumTextFileSize = 32 * 1024 * 1024;
constexpr int AutoSaveIntervalMilliseconds = 60 * 1000;

QString messageText(const QString &detail)
{
    return detail;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_autoSaveTimer(new QTimer(this))
{
    ui->setupUi(this);
    m_editor = ui->editor;

    setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));
    setMinimumSize(640, 440);
    resize(800, 600);

    QFont editorFont(QStringLiteral("Consolas"), 12);
    editorFont.setStyleHint(QFont::Monospace);
    m_editor->setFont(editorFont);
    m_editor->setEditorBackgroundColor(Qt::white);
    m_editor->setLineNumbersVisible(true);
    m_editor->setLineWrapMode(QPlainTextEdit::WidgetWidth);

    statusBar()->setSizeGripEnabled(true);
    m_lengthLabel = new QLabel(this);
    m_linesLabel = new QLabel(this);
    m_positionLabel = new QLabel(this);
    m_userLabel = new QLabel(QStringLiteral("刘泽"), this);
    statusBar()->addPermanentWidget(m_lengthLabel);
    statusBar()->addPermanentWidget(m_linesLabel);
    statusBar()->addPermanentWidget(m_positionLabel);
    statusBar()->addPermanentWidget(m_userLabel);

    connect(m_editor, &QPlainTextEdit::textChanged,
            this, &MainWindow::updateStatusBar);
    connect(m_editor, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateStatusBar);
    connect(m_editor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::updateWindowTitle);
    connect(m_editor, &QPlainTextEdit::undoAvailable,
            ui->actionUndo, &QAction::setEnabled);
    connect(m_editor, &QPlainTextEdit::redoAvailable,
            ui->actionRedo, &QAction::setEnabled);
    connect(m_editor, &QPlainTextEdit::copyAvailable,
            this, &MainWindow::updateActionStates);
    connect(QGuiApplication::clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::updateActionStates);

    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);

    connect(ui->actionUndo, &QAction::triggered, m_editor, &QPlainTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered, m_editor, &QPlainTextEdit::redo);
    connect(ui->actionCut, &QAction::triggered, m_editor, &QPlainTextEdit::cut);
    connect(ui->actionCopy, &QAction::triggered, m_editor, &QPlainTextEdit::copy);
    connect(ui->actionPaste, &QAction::triggered, m_editor, &QPlainTextEdit::paste);
    connect(ui->actionSelectAll, &QAction::triggered,
            m_editor, &QPlainTextEdit::selectAll);

    connect(ui->actionFind, &QAction::triggered,
            this, &MainWindow::showFindDialog);
    connect(ui->actionReplace, &QAction::triggered,
            this, &MainWindow::showReplaceDialog);
    connect(ui->actionFont, &QAction::triggered,
            this, &MainWindow::chooseFont);
    connect(ui->actionTextColor, &QAction::triggered,
            this, &MainWindow::chooseTextColor);
    connect(ui->actionTextBackground, &QAction::triggered,
            this, &MainWindow::chooseTextBackgroundColor);
    connect(ui->actionEditorBackground, &QAction::triggered,
            this, &MainWindow::chooseEditorBackgroundColor);
    connect(ui->actionWordWrap, &QAction::toggled,
            this, &MainWindow::toggleWordWrap);
    connect(ui->actionLineNumbers, &QAction::toggled,
            this, &MainWindow::toggleLineNumbers);

    ui->actionWordWrap->setChecked(true);
    ui->actionLineNumbers->setChecked(true);
    ui->actionToolbar->setChecked(true);
    ui->actionStatusBar->setChecked(true);
    connect(ui->actionToolbar, &QAction::toggled,
            ui->mainToolBar, &QToolBar::setVisible);
    connect(ui->actionStatusBar, &QAction::toggled,
            statusBar(), &QWidget::setVisible);

    m_autoSaveTimer->setInterval(AutoSaveIntervalMilliseconds);
    connect(m_autoSaveTimer, &QTimer::timeout,
            this, &MainWindow::autoSave);
    m_autoSaveTimer->start();

    createDialogs();
    m_editor->document()->setModified(false);
    updateWindowTitle();
    updateStatusBar();
    updateActionStates();
    m_editor->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

CodeEditor *MainWindow::editor() const
{
    return m_editor;
}

QString MainWindow::currentFilePath() const
{
    return m_currentFilePath;
}

QString MainWindow::plainText() const
{
    return m_editor->toPlainText();
}

bool MainWindow::hasUnsavedChanges() const
{
    return m_editor->document()->isModified();
}

bool MainWindow::loadFileFromPath(const QString &path,
                                  QString *errorMessage)
{
    const QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("文件不存在或不是普通文件：%1")
                                .arg(QDir::toNativeSeparators(path));
        }
        return false;
    }

    if (fileInfo.size() > MaximumTextFileSize) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("文件超过 32 MB，未打开该文件。");
        }
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("无法读取文件：%1\n%2")
                                .arg(QDir::toNativeSeparators(path),
                                     file.errorString());
        }
        return false;
    }

    const QByteArray data = file.readAll();
    if (data.contains('\0')) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("所选文件包含二进制数据，不是有效的文本文件。");
        }
        return false;
    }

    QString text = QString::fromUtf8(data);
    if (text.contains(QChar::ReplacementCharacter)) {
        text = QString::fromLocal8Bit(data);
    }

    m_editor->setPlainText(text);
    m_editor->document()->setModified(false);
    m_editor->document()->clearUndoRedoStacks();
    m_currentFilePath = fileInfo.canonicalFilePath();
    updateWindowTitle();
    updateStatusBar();
    updateActionStates();
    return true;
}

bool MainWindow::saveFileToPath(const QString &path, QString *errorMessage)
{
    if (path.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("保存路径不能为空。");
        }
        return false;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("无法写入文件：%1\n%2")
                                .arg(QDir::toNativeSeparators(path),
                                     file.errorString());
        }
        return false;
    }

    const QByteArray data = m_editor->toPlainText().toUtf8();
    if (file.write(data) != data.size()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("写入文件失败：%1").arg(file.errorString());
        }
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("保存文件失败：%1").arg(file.errorString());
        }
        return false;
    }

    m_currentFilePath = QFileInfo(path).absoluteFilePath();
    m_editor->document()->setModified(false);
    updateWindowTitle();
    updateStatusBar();
    return true;
}

bool MainWindow::findText(const QString &text,
                          Qt::CaseSensitivity caseSensitivity,
                          bool forward,
                          bool showFeedback)
{
    if (text.isEmpty()) {
        if (showFeedback) {
            showInformation(QStringLiteral("查找"),
                            QStringLiteral("请输入要查找的内容。"));
        }
        return false;
    }

    QTextDocument *document = m_editor->document();
    QTextDocument::FindFlags flags;
    if (!forward) {
        flags |= QTextDocument::FindBackward;
    }
    if (caseSensitivity == Qt::CaseSensitive) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    const QTextCursor original = m_editor->textCursor();
    const int characterCount = std::max(1, document->characterCount() - 1);
    QTextCursor found;

    if (forward) {
        int start = original.hasSelection()
                        ? original.selectionEnd()
                        : original.position();
        found = document->find(text, start, flags);
        if (found.isNull()) {
            const int wrappedStart = 0;
            found = document->find(text, wrappedStart, flags);
        }
    } else {
        int start = original.hasSelection()
                        ? original.selectionStart()
                        : original.position();
        found = document->find(text, start, flags);
        if (found.isNull()) {
            found = document->find(text, characterCount, flags);
        }
    }

    if (found.isNull()) {
        if (showFeedback) {
            showInformation(QStringLiteral("查找"),
                            QStringLiteral("未找到“%1”。").arg(text));
        }
        return false;
    }

    m_editor->setTextCursor(found);
    m_editor->centerCursor();
    m_editor->setFocus();
    return true;
}

bool MainWindow::replaceCurrentText(const QString &text,
                                    const QString &replacement,
                                    Qt::CaseSensitivity caseSensitivity,
                                    bool forward,
                                    bool showFeedback)
{
    if (text.isEmpty()) {
        if (showFeedback) {
            showInformation(QStringLiteral("替换"),
                            QStringLiteral("请输入要查找的内容。"));
        }
        return false;
    }

    QTextCursor cursor = m_editor->textCursor();
    const Qt::CaseSensitivity matchSensitivity = caseSensitivity;
    const bool selectionMatches =
        cursor.hasSelection()
        && QString::compare(cursor.selectedText(), text,
                            matchSensitivity) == 0;

    if (!selectionMatches) {
        return findText(text, caseSensitivity, forward, showFeedback);
    }

    cursor.insertText(replacement);
    m_editor->setTextCursor(cursor);
    findText(text, caseSensitivity, forward, false);
    if (showFeedback) {
        statusBar()->showMessage(QStringLiteral("已替换 1 处"), 2500);
    }
    return true;
}

int MainWindow::replaceAllText(const QString &text,
                               const QString &replacement,
                               Qt::CaseSensitivity caseSensitivity,
                               bool showFeedback)
{
    if (text.isEmpty()) {
        if (showFeedback) {
            showInformation(QStringLiteral("替换"),
                            QStringLiteral("请输入要查找的内容。"));
        }
        return 0;
    }

    QTextDocument *document = m_editor->document();
    QTextDocument::FindFlags flags;
    if (caseSensitivity == Qt::CaseSensitive) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    QTextCursor editCursor(document);
    editCursor.beginEditBlock();
    QTextCursor found = document->find(text, 0, flags);
    int replacedCount = 0;
    while (!found.isNull()) {
        found.insertText(replacement);
        ++replacedCount;
        found = document->find(text, found, flags);
    }
    editCursor.endEditBlock();

    if (showFeedback) {
        if (replacedCount == 0) {
            showInformation(QStringLiteral("替换"),
                            QStringLiteral("未找到“%1”。").arg(text));
        } else {
            showInformation(
                QStringLiteral("替换"),
                QStringLiteral("已在全部文档中替换 %1 处。").arg(replacedCount));
        }
    }
    return replacedCount;
}

FindDialog *MainWindow::findDialogWidget() const
{
    return m_findDialog;
}

ReplaceDialog *MainWindow::replaceDialogWidget() const
{
    return m_replaceDialog;
}

AboutDialog *MainWindow::aboutDialogWidget() const
{
    return m_aboutDialog;
}

void MainWindow::showFindDialog()
{
    refreshDialogSearchText();
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
    m_findDialog->focusSearchField();
}

void MainWindow::showReplaceDialog()
{
    refreshDialogSearchText();
    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
    m_replaceDialog->focusSearchField();
}

void MainWindow::showAboutDialog()
{
    m_aboutDialog->exec();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newFile()
{
    if (!maybeSave()) {
        return;
    }

    m_editor->clear();
    m_editor->document()->clearUndoRedoStacks();
    m_editor->document()->setModified(false);
    m_currentFilePath.clear();
    updateWindowTitle();
    updateStatusBar();
    updateActionStates();
}

void MainWindow::openFile()
{
    if (!maybeSave()) {
        return;
    }

    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("打开文本文件"), QString(),
        QStringLiteral("文本文件 (*.txt *.md *.log *.cpp *.h *.json);;所有文件 (*)"));
    if (path.isEmpty()) {
        return;
    }

    QString error;
    if (!loadFileFromPath(path, &error)) {
        showWarning(QStringLiteral("打开文件"), messageText(error));
    }
}

bool MainWindow::saveFile()
{
    if (m_currentFilePath.isEmpty()) {
        return saveFileAs();
    }
    return saveToCurrentPath();
}

bool MainWindow::saveFileAs()
{
    QString suggestedPath = m_currentFilePath;
    if (suggestedPath.isEmpty()) {
        suggestedPath = QDir::homePath() + QStringLiteral("/新建文本文件.txt");
    }

    const QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("另存为"), suggestedPath,
        QStringLiteral("文本文件 (*.txt);;所有文件 (*)"));
    if (path.isEmpty()) {
        return false;
    }

    QString error;
    if (!saveFileToPath(path, &error)) {
        showWarning(QStringLiteral("保存文件"), messageText(error));
        return false;
    }
    statusBar()->showMessage(QStringLiteral("文件已保存"), 2500);
    return true;
}

void MainWindow::chooseFont()
{
    bool accepted = false;
    const QFont font = QFontDialog::getFont(
        &accepted, m_editor->font(), this,
        QStringLiteral("设置字体"));
    if (!accepted) {
        return;
    }

    QTextCharFormat format;
    format.setFont(font);
    applyCharacterFormat(format);
}

void MainWindow::chooseTextColor()
{
    const QColor color = QColorDialog::getColor(
        Qt::black, this, QStringLiteral("字体颜色"));
    if (!color.isValid()) {
        return;
    }

    QTextCharFormat format;
    format.setForeground(color);
    applyCharacterFormat(format);
}

void MainWindow::chooseTextBackgroundColor()
{
    const QColor color = QColorDialog::getColor(
        Qt::yellow, this, QStringLiteral("字体背景色"));
    if (!color.isValid()) {
        return;
    }

    QTextCharFormat format;
    format.setBackground(color);
    applyCharacterFormat(format);
}

void MainWindow::chooseEditorBackgroundColor()
{
    const QColor color = QColorDialog::getColor(
        m_editor->palette().color(QPalette::Base), this,
        QStringLiteral("编辑器背景色"));
    if (!color.isValid()) {
        return;
    }

    m_editor->setEditorBackgroundColor(color);
}

void MainWindow::toggleWordWrap(bool enabled)
{
    m_editor->setLineWrapMode(enabled
                                  ? QPlainTextEdit::WidgetWidth
                                  : QPlainTextEdit::NoWrap);
}

void MainWindow::toggleLineNumbers(bool enabled)
{
    m_editor->setLineNumbersVisible(enabled);
}

void MainWindow::updateWindowTitle()
{
    const QString displayName = m_currentFilePath.isEmpty()
                                    ? QStringLiteral("新建文本文件")
                                    : QFileInfo(m_currentFilePath).fileName();
    const QString modifiedMark =
        m_editor->document()->isModified() ? QStringLiteral("*") : QString();
    setWindowTitle(QStringLiteral("%1%2 - 编辑器")
                       .arg(modifiedMark, displayName));
}

void MainWindow::updateStatusBar()
{
    const QTextCursor cursor = m_editor->textCursor();
    const int line = cursor.blockNumber() + 1;
    const int column = cursor.positionInBlock() + 1;

    m_lengthLabel->setText(
        QStringLiteral("length: %1").arg(m_editor->toPlainText().size()));
    m_linesLabel->setText(
        QStringLiteral("lines: %1").arg(m_editor->blockCount()));
    m_positionLabel->setText(
        QStringLiteral("Ln: %1   Col: %2").arg(line).arg(column));
}

void MainWindow::updateActionStates()
{
    const bool hasSelection = m_editor->textCursor().hasSelection();
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);
    ui->actionPaste->setEnabled(
        QGuiApplication::clipboard()->mimeData()
        && QGuiApplication::clipboard()->mimeData()->hasText());
    ui->actionUndo->setEnabled(m_editor->document()->isUndoAvailable());
    ui->actionRedo->setEnabled(m_editor->document()->isRedoAvailable());
}

void MainWindow::autoSave()
{
    if (m_currentFilePath.isEmpty()
        || !m_editor->document()->isModified()) {
        return;
    }

    QString error;
    if (saveFileToPath(m_currentFilePath, &error)) {
        statusBar()->showMessage(QStringLiteral("已自动保存"), 3000);
    } else {
        statusBar()->showMessage(QStringLiteral("自动保存失败"), 4000);
    }
}

void MainWindow::createDialogs()
{
    m_findDialog = new FindDialog(this);
    m_replaceDialog = new ReplaceDialog(this);
    m_aboutDialog = new AboutDialog(this);

    connect(m_findDialog, &FindDialog::findRequested,
            this, [this](const QString &text,
                         Qt::CaseSensitivity sensitivity,
                         bool forward) {
                findText(text, sensitivity, forward, true);
            });
    connect(m_replaceDialog, &ReplaceDialog::findRequested,
            this, [this](const QString &text,
                         Qt::CaseSensitivity sensitivity,
                         bool forward) {
                findText(text, sensitivity, forward, true);
            });
    connect(m_replaceDialog, &ReplaceDialog::replaceRequested,
            this, [this](const QString &text, const QString &replacement,
                         Qt::CaseSensitivity sensitivity, bool forward) {
                replaceCurrentText(text, replacement,
                                   sensitivity, forward, true);
            });
    connect(m_replaceDialog, &ReplaceDialog::replaceAllRequested,
            this, [this](const QString &text, const QString &replacement,
                         Qt::CaseSensitivity sensitivity) {
                replaceAllText(text, replacement, sensitivity, true);
            });
}

bool MainWindow::maybeSave()
{
    if (!m_editor->document()->isModified()) {
        return true;
    }

    const QMessageBox::StandardButton choice = QMessageBox::warning(
        this, QStringLiteral("未保存的更改"),
        QStringLiteral("文档已被修改，是否保存更改？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);

    if (choice == QMessageBox::Save) {
        return saveFile();
    }
    if (choice == QMessageBox::Cancel) {
        return false;
    }
    return true;
}

bool MainWindow::saveToCurrentPath()
{
    QString error;
    if (!saveFileToPath(m_currentFilePath, &error)) {
        showWarning(QStringLiteral("保存文件"), messageText(error));
        return false;
    }
    statusBar()->showMessage(QStringLiteral("文件已保存"), 2500);
    return true;
}

void MainWindow::applyCharacterFormat(const QTextCharFormat &format)
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        cursor.mergeCharFormat(format);
        m_editor->setTextCursor(cursor);
    } else {
        m_editor->mergeCurrentCharFormat(format);
    }
    m_editor->setFocus();
}

void MainWindow::refreshDialogSearchText()
{
    const QString selectedText = m_editor->textCursor().selectedText();
    if (selectedText.isEmpty() || selectedText.contains(QChar::ParagraphSeparator)) {
        return;
    }
    m_findDialog->setSearchText(selectedText);
    m_replaceDialog->setSearchText(selectedText);
}

void MainWindow::showWarning(const QString &title, const QString &message)
{
    QMessageBox::warning(this, title, message);
}

void MainWindow::showInformation(const QString &title, const QString &message)
{
    QMessageBox::information(this, title, message);
}

