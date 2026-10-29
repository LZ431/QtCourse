#include "replacedialog.h"
#include "ui_replacedialog.h"

#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>

ReplaceDialog::ReplaceDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReplaceDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setModal(false);

    connect(ui->findButton, &QPushButton::clicked,
            this, &ReplaceDialog::requestFind);
    connect(ui->replaceButton, &QPushButton::clicked,
            this, &ReplaceDialog::requestReplace);
    connect(ui->replaceAllButton, &QPushButton::clicked,
            this, &ReplaceDialog::requestReplaceAll);
    connect(ui->cancelButton, &QPushButton::clicked,
            this, &QDialog::hide);
    connect(ui->searchLineEdit, &QLineEdit::returnPressed,
            this, &ReplaceDialog::requestFind);
    connect(ui->replacementLineEdit, &QLineEdit::returnPressed,
            this, &ReplaceDialog::requestReplace);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

QString ReplaceDialog::searchText() const
{
    return ui->searchLineEdit->text();
}

QString ReplaceDialog::replacementText() const
{
    return ui->replacementLineEdit->text();
}

void ReplaceDialog::setSearchText(const QString &text)
{
    ui->searchLineEdit->setText(text);
    if (!text.isEmpty()) {
        ui->searchLineEdit->selectAll();
    }
}

Qt::CaseSensitivity ReplaceDialog::caseSensitivity() const
{
    return ui->caseSensitiveCheckBox->isChecked()
               ? Qt::CaseSensitive
               : Qt::CaseInsensitive;
}

bool ReplaceDialog::forward() const
{
    return ui->forwardRadioButton->isChecked();
}

void ReplaceDialog::focusSearchField()
{
    ui->searchLineEdit->setFocus();
    ui->searchLineEdit->selectAll();
}

void ReplaceDialog::requestFind()
{
    emit findRequested(searchText(), caseSensitivity(), forward());
}

void ReplaceDialog::requestReplace()
{
    emit replaceRequested(searchText(), replacementText(),
                          caseSensitivity(), forward());
}

void ReplaceDialog::requestReplaceAll()
{
    emit replaceAllRequested(searchText(), replacementText(),
                             caseSensitivity());
}

