// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PREVIEWFORM_H
#define PREVIEWFORM_H

#include <QDialog>
#include <QList>

QT_BEGIN_NAMESPACE
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QTabWidget;
class QTextCodec;
QT_END_NAMESPACE

class PreviewForm : public QDialog
{
    Q_OBJECT

public:
    explicit PreviewForm(QWidget *parent = nullptr);

    void setCodecList(const QList<QTextCodec *> &list);
    void setEncodedData(const QByteArray &data);
    QString decodedString() const { return decodedStr; }

private slots:
    void updateTextEdit();

private:
    void reset();

    QByteArray encodedData;
    QString decodedStr;

    QPushButton *okButton;
    QTabWidget *tabWidget;
    QComboBox *encodingComboBox;
    QPlainTextEdit *textEdit;
    QPlainTextEdit *hexDumpEdit;
    QLabel *statusLabel;
};

#endif
