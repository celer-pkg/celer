// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>
#include <QObject>
#include <QXmlStreamReader>
#include <QtScxml/qscxmlcompiler.h>
#include <QtScxml/qscxmlstatemachine.h>

class tst_Parser: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void error_data();
    void error();
};

void tst_Parser::error_data()
{
    QTest::addColumn<QString>("scxmlFileName");
    QTest::addColumn<QString>("errorFileName");

    QDir dir(QLatin1String(":/tst_parser/data/"));
    const auto dirEntries = dir.entryList();
    for (const QString &entry : dirEntries) {
        if (!entry.endsWith(QLatin1String(".errors"))) {
            QString scxmlFileName = dir.filePath(entry);
            QTest::newRow(entry.toLatin1().constData())
                    << scxmlFileName << (scxmlFileName + QLatin1String(".errors"));
        }
    }
}

void tst_Parser::error()
{
    QFETCH(QString, scxmlFileName);
    QFETCH(QString, errorFileName);

    QFile errorFile(errorFileName);
    QVERIFY(errorFile.open(QIODevice::ReadOnly | QIODevice::Text));
    const QStringList expectedErrors =
            QString::fromUtf8(errorFile.readAll()).split('\n', Qt::SkipEmptyParts);

    if (!expectedErrors.isEmpty())
        QTest::ignoreMessage(QtWarningMsg, "SCXML document has errors");

    QScopedPointer<QScxmlStateMachine> stateMachine(QScxmlStateMachine::fromFile(scxmlFileName));
    QVERIFY(!stateMachine.isNull());

    const QList<QScxmlError> errors = stateMachine->parseErrors();
    if (errors.size() != expectedErrors.size()) {
        for (const QScxmlError &error : errors) {
            qDebug() << error.toString();
        }
    }
    QCOMPARE(errors.size(), expectedErrors.size());

    for (int i = 0; i < errors.size(); ++i)
        QCOMPARE(errors.at(i).toString(), expectedErrors.at(i));
}

QTEST_MAIN(tst_Parser)

#include "tst_parser.moc"


