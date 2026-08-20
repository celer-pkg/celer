// Copyright (C) 2016 BlackBerry Limited, Copyright (C) 2016 BasysKom GmbH
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNEARFIELDTARGET_NEARD_P_H
#define QNEARFIELDTARGET_NEARD_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QDBusObjectPath>
#include <QDBusVariant>

#include <qnearfieldtarget.h>
#include <qnearfieldtarget_p.h>
#include <qndefrecord.h>
#include <qndefmessage.h>

#include "neard/neard_helper_p.h"
#include "properties_interface.h"
#include "objectmanager_interface.h"
#include "tag_interface.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_NFC_NEARD)

class QNearFieldTargetPrivateImpl : public QNearFieldTargetPrivate
{
public:
    QNearFieldTargetPrivateImpl(QObject *parent, QDBusObjectPath interfacePath);

    ~QNearFieldTargetPrivateImpl();

    bool isValid();
    QByteArray uid() const override;

    QNearFieldTarget::Type type() const override;
    QNearFieldTarget::AccessMethods accessMethods() const override;

    bool hasNdefMessage() override;
    QNearFieldTarget::RequestId readNdefMessages() override;
    QNearFieldTarget::RequestId sendCommand(const QByteArray &command) override;
    QNearFieldTarget::RequestId writeNdefMessages(const QList<QNdefMessage> &messages) override;

private:
    QNdefRecord readRecord(const QDBusObjectPath &path);
    void handleRecordFound(const QDBusObjectPath &path);
    void createNdefMessage();
    void handleReadError();
    void handleWriteRequest();

    QDBusObjectPath m_tagPath;
    QtNfcPrivate::OrgFreedesktopDBusPropertiesInterface *m_dbusProperties;
    QList<QDBusObjectPath> m_recordPaths;
    QTimer m_recordPathsCollectedTimer;
    QTimer m_readErrorTimer;
    QTimer m_delayedWriteTimer;
    QByteArray m_uid;
    QNearFieldTarget::Type m_type;
    bool m_readRequested;
    QNearFieldTarget::RequestId m_currentReadRequestId;
    QNearFieldTarget::RequestId m_currentWriteRequestId;
    QVariantMap m_currentWriteRequestData;
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_NEARD_P_H
