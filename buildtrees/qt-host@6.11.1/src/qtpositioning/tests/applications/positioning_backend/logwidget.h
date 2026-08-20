// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qplaintextedit.h>

class LogWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LogWidget(QWidget *parent = nullptr);

    void appendLog(const QString &line);

private:
    QPlainTextEdit *editor;
};

#endif // LOGWIDGET_H
