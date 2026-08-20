// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <grpccommonoptions.h>

#include <QtGrpc/qgrpcchanneloptions.h>
#include <QtGrpc/qgrpcserializationformat.h>

#include <QtTest/qtest.h>

#include <cstring>

class QGrpcChannelOptionsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void hasSpecialMemberFunctions() const { common.hasSpecialMemberFunctions(); }
    void hasImplicitQVariant() const { common.hasImplicitQVariant(); }
    void hasMemberSwap() const { common.hasMemberSwap(); }
#if QT_DEPRECATED_SINCE(6, 13)
    void deprecatedPropertyMetadata() const { common.deprecatedPropertyMetadata(); }
    void propertyMetadataCompat() const { common.propertyMetadataCompat(); }
#endif
    void propertyMetadata() const { common.propertyMetadata(); }
    void propertyDeadline() const { common.propertyDeadline(); }
    void propertyFilterServerMetadata() const { common.propertyFilterServerMetadata(); }
    void streamsToDebug() const { common.streamsToDebug(); }
    void comparesEqual() const { common.comparesEqual(); }

    void propertySerializationFormat() const;
#if QT_CONFIG(ssl)
    void propertySslConfiguration() const;
#endif

private:
    GrpcCommonOptionsTest<QGrpcChannelOptions> common;
};

void QGrpcChannelOptionsTest::propertySerializationFormat() const
{
    QGrpcSerializationFormat fmt(QtGrpc::SerializationFormat::Json);

    QGrpcChannelOptions o1;
    auto o1Detach = o1;
    QVERIFY(o1.serializationFormat().suffix().isEmpty());
    o1.setSerializationFormat(fmt);
    QVERIFY(!o1.serializationFormat().suffix().isEmpty());
    QCOMPARE_EQ(o1.serializationFormat().suffix(), fmt.suffix());
    QCOMPARE_NE(o1.serializationFormat().suffix(), o1Detach.serializationFormat().suffix());
}

#if QT_CONFIG(ssl)
void QGrpcChannelOptionsTest::propertySslConfiguration() const
{
    QSslConfiguration sslConfig;
    sslConfig.setSessionTicket("test");

    QGrpcChannelOptions o1;
    auto o1Detach = o1;
    QVERIFY(!o1.sslConfiguration());
    o1.setSslConfiguration(sslConfig);
    QVERIFY(o1.sslConfiguration());
    QCOMPARE_EQ(o1.sslConfiguration()->sessionTicket(), sslConfig.sessionTicket());
    QCOMPARE_NE(o1.sslConfiguration(), o1Detach.sslConfiguration());
}
#endif

QTEST_MAIN(QGrpcChannelOptionsTest)

#include "tst_qgrpcchanneloptions.moc"
