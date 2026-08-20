// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "util.h"

#include <QtCore/QDebug>
#include <QtCore/QMutexLocker>

QQmlDataTest *QQmlDataTest::m_instance = 0;

QQmlDataTest::QQmlDataTest() :
#ifdef QT_TESTCASE_BUILDDIR
    m_dataDirectory(QTest::qFindTestData("data", QT_QMLTEST_DATADIR, 0, QT_TESTCASE_BUILDDIR)),
#else
    m_dataDirectory(QTest::qFindTestData("data", QT_QMLTEST_DATADIR, 0)),
#endif

    m_dataDirectoryUrl(m_dataDirectory.startsWith(QLatin1Char(':'))
        ? QUrl(QLatin1String("qrc") + m_dataDirectory)
        : QUrl::fromLocalFile(m_dataDirectory + QLatin1Char('/')))
{
    m_instance = this;
}

QQmlDataTest::~QQmlDataTest()
{
    m_instance = 0;
}

void QQmlDataTest::initTestCase()
{
    QVERIFY2(!m_dataDirectory.isEmpty(), "'data' directory not found");
    m_directory = QFileInfo(m_dataDirectory).absolutePath();
    if (m_dataDirectoryUrl.scheme() != QLatin1String("qrc"))
        QVERIFY2(QDir::setCurrent(m_directory), qPrintable(QLatin1String("Could not chdir to ") + m_directory));
}

QString QQmlDataTest::testFile(const QString &fileName) const
{
    if (m_directory.isEmpty())
        qFatal("QQmlDataTest::initTestCase() not called.");
    QString result = m_dataDirectory;
    result += QLatin1Char('/');
    result += fileName;
    return result;
}

Q_GLOBAL_STATIC(QMutex, qQmlTestMessageHandlerMutex)

QQmlTestMessageHandler *QQmlTestMessageHandler::m_instance = 0;

void QQmlTestMessageHandler::messageHandler(QtMsgType, const QMessageLogContext &context, const QString &message)
{
    QMutexLocker locker(qQmlTestMessageHandlerMutex());
    if (QQmlTestMessageHandler::m_instance) {
        if (QQmlTestMessageHandler::m_instance->m_includeCategories)
            QQmlTestMessageHandler::m_instance->m_messages.push_back(QString("%1: %2").arg(context.category, message));
        else
            QQmlTestMessageHandler::m_instance->m_messages.push_back(message);
    }
}

QQmlTestMessageHandler::QQmlTestMessageHandler()
{
    QMutexLocker locker(qQmlTestMessageHandlerMutex());
    Q_ASSERT(!QQmlTestMessageHandler::m_instance);
    QQmlTestMessageHandler::m_instance = this;
    m_oldHandler = qInstallMessageHandler(messageHandler);
    m_includeCategories = false;
}

QQmlTestMessageHandler::~QQmlTestMessageHandler()
{
    QMutexLocker locker(qQmlTestMessageHandlerMutex());
    Q_ASSERT(QQmlTestMessageHandler::m_instance);
    qInstallMessageHandler(m_oldHandler);
    QQmlTestMessageHandler::m_instance = 0;
}
