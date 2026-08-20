// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef INPUTTAB_H
#define INPUTTAB_H

#include "converterthread.h"

#include <QWidget>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
QT_END_NAMESPACE

class InputListView;
class SettingsTab;

class InputTab : public QWidget
{
    Q_OBJECT

public:
    explicit InputTab(SettingsTab *settings, QWidget *parent = nullptr);

    QStringList getInputFiles() const;
    QString getOutputPath() const;

    void convert();

private slots:
    void browseInput();
    void browseOutput();
    void removeSelected();
    void selectAll();

    void convertStart(const QString &text);
    void convertUpdate(const QString &text);
    void convertDone(const QString &text);

private:
    SettingsTab *settingsTab = nullptr;
    QLineEdit *outputPathEdit = nullptr;
    InputListView *inputFilesListBox = nullptr;
    QPushButton *convertButton = nullptr;
    QPlainTextEdit *statusText = nullptr;

    QString lastInputPath;
    ConverterThread converterThread;
};

#endif
