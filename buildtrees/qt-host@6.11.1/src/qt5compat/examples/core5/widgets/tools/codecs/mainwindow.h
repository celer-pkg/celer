// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QTextCodec;
class QPlainTextEdit;
QT_END_NAMESPACE

class EncodingDialog;
class PreviewForm;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void open();
    void save();
    void about();
    void aboutToShowSaveAsMenu();
    void encodingDialog();

private:
    void findCodecs();
    void createMenus();

    QList<QAction *> saveAsActs;
    QPlainTextEdit *textEdit;
    PreviewForm *previewForm;
    QList<QTextCodec *> codecs;
    EncodingDialog *m_encodingDialog = nullptr;
};

#endif
