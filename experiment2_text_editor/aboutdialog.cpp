#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QIcon>
#include <QPushButton>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setWindowTitle(QStringLiteral("关于 小牛文本编辑器"));
    setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));

    connect(ui->okButton, &QPushButton::clicked,
            this, &QDialog::accept);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

