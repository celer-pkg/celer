// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <grpccommonoptions.h>

#include <QtGrpc/qgrpccalloptions.h>

#include <QtTest/qtest.h>

class QGrpcCallOptionsTest : public QObject
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

private:
    GrpcCommonOptionsTest<QGrpcCallOptions> common;
};

QTEST_MAIN(QGrpcCallOptionsTest)

#include "tst_qgrpccalloptions.moc"
