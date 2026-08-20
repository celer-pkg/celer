// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    connect(ui->send, &QPushButton::clicked, this, &Dialog::clicked);
}

void Dialog::displayMessage(const QString &message)
{
    ui->output->appendPlainText(message);
}

void Dialog::clicked()
{
    const QString text = ui->input->text();

    if (text.isEmpty())
        return;

    emit sendText(text);
    displayMessage(tr("Sent message: %1").arg(text));

    ui->input->clear();
}
