#pragma once

#include <QDialog>

namespace Ui {
class ReplaceDialog;
}

class ReplaceDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent = nullptr);
    ~ReplaceDialog() override;

    QString searchText() const;
    QString replacementText() const;
    void setSearchText(const QString &text);
    Qt::CaseSensitivity caseSensitivity() const;
    bool forward() const;
    void focusSearchField();

signals:
    void findRequested(const QString &text,
                       Qt::CaseSensitivity caseSensitivity,
                       bool forward);
    void replaceRequested(const QString &text,
                          const QString &replacement,
                          Qt::CaseSensitivity caseSensitivity,
                          bool forward);
    void replaceAllRequested(const QString &text,
                             const QString &replacement,
                             Qt::CaseSensitivity caseSensitivity);

private slots:
    void requestFind();
    void requestReplace();
    void requestReplaceAll();

private:
    Ui::ReplaceDialog *ui;
};

