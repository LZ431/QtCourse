#include "finddialog.h"
#include "ui_finddialog.h"

#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setModal(false);

    connect(ui->findButton, &QPushButton::clicked,
            this, &FindDialog::requestFind);
    connect(ui->cancelButton, &QPushButton::clicked,
            this, &QDialog::hide);
    connect(ui->searchLineEdit, &QLineEdit::returnPressed,
            this, &FindDialog::requestFind);
}

FindDialog::~FindDialog()
{
    delete ui;
}

QString FindDialog::searchText() const
{
    return ui->searchLineEdit->text();
}

void FindDialog::setSearchText(const QString &text)
{
    ui->searchLineEdit->setText(text);
    if (!text.isEmpty()) {
        ui->searchLineEdit->selectAll();
    }
}

Qt::CaseSensitivity FindDialog::caseSensitivity() const
{
    return ui->caseSensitiveCheckBox->isChecked()
               ? Qt::CaseSensitive
               : Qt::CaseInsensitive;
}

bool FindDialog::forward() const
{
    return ui->forwardRadioButton->isChecked();
}

void FindDialog::focusSearchField()
{
    ui->searchLineEdit->setFocus();
    ui->searchLineEdit->selectAll();
}

void FindDialog::requestFind()
{
    emit findRequested(searchText(), caseSensitivity(), forward());
}

