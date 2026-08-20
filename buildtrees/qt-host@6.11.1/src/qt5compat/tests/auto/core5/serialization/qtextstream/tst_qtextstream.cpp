// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QBuffer>
#include <QStringConverter>
#include <QStringRef>
#include <QTextStream>

class tst_QTextStream : public QObject
{
    Q_OBJECT

private slots:
    // text write operators
    void stringref_write_operator_ToDevice();
};

void tst_QTextStream::stringref_write_operator_ToDevice()
{
    QBuffer buf;
    buf.open(QBuffer::WriteOnly);
    QTextStream stream(&buf);
    stream.setEncoding(QStringConverter::Latin1);
    stream.setAutoDetectUnicode(true);

    const QString expected = "No explicit lengthExplicit length";

    stream << QStringRef(&expected).left(18);
    stream << QStringRef(&expected).mid(18);
    stream.flush();
    QCOMPARE(buf.buffer().constData(), "No explicit lengthExplicit length");
}

QTEST_MAIN(tst_QTextStream)
#include "tst_qtextstream.moc"
