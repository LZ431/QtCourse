#pragma once

#include <QDialog>

namespace Ui {
class FindDialog;
}

class FindDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);
    ~FindDialog() override;

    QString searchText() const;
    void setSearchText(const QString &text);
    Qt::CaseSensitivity caseSensitivity() const;
    bool forward() const;
    void focusSearchField();

signals:
    void findRequested(const QString &text,
                       Qt::CaseSensitivity caseSensitivity,
                       bool forward);

private slots:
    void requestFind();

private:
    Ui::FindDialog *ui;
};

