// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "logwidget.h"
#include <QVBoxLayout>

LogWidget::LogWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));

    editor = new QPlainTextEdit(this);
    verticalLayout->addWidget(editor);
}

void LogWidget::appendLog(const QString &line)
{
    editor->appendPlainText(line);
}
