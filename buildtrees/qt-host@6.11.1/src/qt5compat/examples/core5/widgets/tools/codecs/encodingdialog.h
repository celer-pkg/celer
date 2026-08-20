// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ENCODINGDIALOG_H
#define ENCODINGDIALOG_H

#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QLineEdit)

class EncodingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EncodingDialog(QWidget *parent = nullptr);

    enum Encoding { Unicode, Utf8, Utf16, Utf32, Latin1, EncodingCount };

private slots:
    void textChanged(const QString &t);

private:
    QLineEdit *m_lineEdits[EncodingCount];
};

#endif // ENCODINGDIALOG_H
