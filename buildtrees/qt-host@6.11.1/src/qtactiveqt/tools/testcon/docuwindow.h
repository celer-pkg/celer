// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef DOCUWINDOW_H
#define DOCUWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE

class QTextBrowser;

class DocuWindow : public QMainWindow
{
    Q_OBJECT
public:
    DocuWindow(const QString& docu, QWidget *parent = nullptr);

public slots:
    void save();
    void print();

private:
    QTextBrowser *browser;
};

QT_END_NAMESPACE

#endif // DOCUWINDOW_H
