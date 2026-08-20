// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 Intel Corporation.
// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QtDebug>
#include <QtTest/QtTest>
#include <qstringref.h>

class tst_QDebug: public QObject
{
    Q_OBJECT

private slots:
    void qDebugQStringRef() const;
};

static QtMsgType s_msgType;
static QString s_msg;
static QByteArray s_file;
static int s_line;
static QByteArray s_function;

static void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    s_msg = msg;
    s_msgType = type;
    s_file = context.file;
    s_line = context.line;
    s_function = context.function;
}

// Helper class to ensure that the testlib message handler gets
// restored at the end of each test function, even if the test
// fails or throws an exception.
class MessageHandlerSetter
{
public:
    MessageHandlerSetter(QtMessageHandler newMessageHandler)
        : oldMessageHandler(qInstallMessageHandler(newMessageHandler))
    { }

    ~MessageHandlerSetter()
    {
        qInstallMessageHandler(oldMessageHandler);
    }

private:
    QtMessageHandler oldMessageHandler;
};

void tst_QDebug::qDebugQStringRef() const
{
    /* Use a basic string. */
    {
        QString file, function;
        int line = 0;
        const QString in(QLatin1String("input"));
        const QStringRef inRef(&in);

        MessageHandlerSetter mhs(myMessageHandler);
        { qDebug() << inRef; }
#ifndef QT_NO_MESSAGELOGCONTEXT
        file = __FILE__; line = __LINE__ - 2; function = Q_FUNC_INFO;
#endif
        QCOMPARE(s_msgType, QtDebugMsg);
        QCOMPARE(s_msg, QString::fromLatin1("\"input\""));
        QCOMPARE(QString::fromLatin1(s_file), file);
        QCOMPARE(s_line, line);
        QCOMPARE(QString::fromLatin1(s_function), function);
    }

    /* Use a null QStringRef. */
    {
        QString file, function;
        int line = 0;

        const QStringRef inRef;

        MessageHandlerSetter mhs(myMessageHandler);
        { qDebug() << inRef; }
#ifndef QT_NO_MESSAGELOGCONTEXT
        file = __FILE__; line = __LINE__ - 2; function = Q_FUNC_INFO;
#endif
        QCOMPARE(s_msgType, QtDebugMsg);
        QCOMPARE(s_msg, QString::fromLatin1("\"\""));
        QCOMPARE(QString::fromLatin1(s_file), file);
        QCOMPARE(s_line, line);
        QCOMPARE(QString::fromLatin1(s_function), function);
    }
}

QTEST_MAIN(tst_QDebug);
#include "tst_qdebug.moc"
