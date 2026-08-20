// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QTextFormat>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QLineEdit;
class QPushButton;
class QLabel;
QT_END_NAMESPACE

//![0]
class Window : public QWidget
{
    Q_OBJECT

public:
    enum { SvgTextFormat = QTextFormat::UserObject + 1 };
    enum SvgProperties { SvgData = 1 };

    Window();

private slots:
    void insertTextObject();

private:
    void setupTextObject();
    void setupGui();

private:
    QTextEdit *textEdit;
    QLabel *fileNameLabel;
    QLineEdit *fileNameLineEdit;
    QPushButton *insertTextObjectButton;
};
//![0]

#endif
