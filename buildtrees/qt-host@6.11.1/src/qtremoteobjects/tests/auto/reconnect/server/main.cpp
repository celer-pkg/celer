// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QtTest/QtTest>
#include <QtRemoteObjects/qremoteobjectnode.h>
#include <QtRemoteObjects/qremoteobjectsource.h>


class SourceObj : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void ping() { ++m_pingCount; };

Q_SIGNALS:
    void pong();

public:
    int m_pingCount = 0;
};

class tst_Server_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRun()
    {
        const QString url = qEnvironmentVariable("RO_URL");
        QRemoteObjectHost *srcNode = new QRemoteObjectHost(QUrl(url));
        SourceObj so;
        so.setObjectName(QStringLiteral("SourceObj"));
        srcNode->enableRemoting(&so);

        QTRY_VERIFY(so.m_pingCount == 1);
        emit so.pong();
        QTRY_VERIFY(so.m_pingCount == 2);
        delete srcNode;

        srcNode = new QRemoteObjectHost(QUrl(url));
        srcNode->enableRemoting(&so);

        QTRY_VERIFY(so.m_pingCount == 3);
        emit so.pong();
        QTRY_VERIFY(so.m_pingCount == 4);
        delete srcNode;
    }
};

QTEST_MAIN(tst_Server_Process)

#include "main.moc"
