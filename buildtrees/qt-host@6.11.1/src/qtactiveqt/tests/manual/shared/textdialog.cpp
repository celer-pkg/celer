// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "textdialog.h"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QVBoxLayout>

#include <QtGui/QFontDatabase>

TextDialog::TextDialog(const QString &text, QWidget *parent) : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    auto layout = new QVBoxLayout(this);
    auto pe = new QPlainTextEdit(text, this);
    pe->setReadOnly(true);
    pe->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    layout->addWidget(pe);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
