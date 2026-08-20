// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QtRemoteObjects/qremoteobjectnode.h>
#include <QtTest/QtTest>

#include "../../../shared/testutils.h"

class tst_Proxy_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRun()
    {
        m_hostNode.reset(new QRemoteObjectHost);
        m_hostNode->setHostUrl(QUrl(QStringLiteral("tcp://127.0.0.1:65213")));
        m_hostNode->proxy(QUrl(LOCAL_SOCKET ":testRegistry"));

        // our proxied object should be added, and then later removed when the server shuts down
        QSignalSpy addSpy(m_hostNode.data(), &QRemoteObjectNode::remoteObjectAdded);
        QSignalSpy removeSpy(m_hostNode.data(), &QRemoteObjectNode::remoteObjectRemoved);
        QVERIFY(addSpy.wait());
        QVERIFY(removeSpy.wait());
    }

private:
    QScopedPointer<QRemoteObjectHost> m_hostNode;
};

QTEST_MAIN(tst_Proxy_Process)

#include "main.moc"
