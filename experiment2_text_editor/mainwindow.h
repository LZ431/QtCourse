#pragma once

#include <QMainWindow>

class AboutDialog;
class CodeEditor;
class FindDialog;
class QLabel;
class QCloseEvent;
class QTextCharFormat;
class QTimer;
class ReplaceDialog;

namespace Ui {
class MainWindow;
}

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    CodeEditor *editor() const;
    QString currentFilePath() const;
    QString plainText() const;
    bool hasUnsavedChanges() const;

    bool loadFileFromPath(const QString &path, QString *errorMessage = nullptr);
    bool saveFileToPath(const QString &path, QString *errorMessage = nullptr);
    bool findText(const QString &text,
                  Qt::CaseSensitivity caseSensitivity,
                  bool forward,
                  bool showFeedback = true);
    bool replaceCurrentText(const QString &text,
                            const QString &replacement,
                            Qt::CaseSensitivity caseSensitivity,
                            bool forward,
                            bool showFeedback = true);
    int replaceAllText(const QString &text,
                       const QString &replacement,
                       Qt::CaseSensitivity caseSensitivity,
                       bool showFeedback = true);

    FindDialog *findDialogWidget() const;
    ReplaceDialog *replaceDialogWidget() const;
    AboutDialog *aboutDialogWidget() const;
    void showFindDialog();
    void showReplaceDialog();
    void showAboutDialog();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void chooseFont();
    void chooseTextColor();
    void chooseTextBackgroundColor();
    void chooseEditorBackgroundColor();
    void toggleWordWrap(bool enabled);
    void toggleLineNumbers(bool enabled);
    void updateWindowTitle();
    void updateStatusBar();
    void updateActionStates();
    void autoSave();

private:
    void createDialogs();
    bool maybeSave();
    bool saveToCurrentPath();
    void applyCharacterFormat(const QTextCharFormat &format);
    void refreshDialogSearchText();
    void showWarning(const QString &title, const QString &message);
    void showInformation(const QString &title, const QString &message);

    Ui::MainWindow *ui;
    CodeEditor *m_editor;
    QString m_currentFilePath;
    QTimer *m_autoSaveTimer;
    FindDialog *m_findDialog = nullptr;
    ReplaceDialog *m_replaceDialog = nullptr;
    AboutDialog *m_aboutDialog = nullptr;
    QLabel *m_lengthLabel = nullptr;
    QLabel *m_linesLabel = nullptr;
    QLabel *m_positionLabel = nullptr;
    QLabel *m_userLabel = nullptr;
};

