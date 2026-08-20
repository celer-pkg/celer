// Copyright (C) 2022q The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNEARFIELDTARGET_PCSC_P_H
#define QNEARFIELDTARGET_PCSC_P_H

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

#include "qnearfieldtarget_p.h"

QT_BEGIN_NAMESPACE

class QNearFieldTargetPrivateImpl : public QNearFieldTargetPrivate
{
    Q_OBJECT
public:
    QNearFieldTargetPrivateImpl(const QByteArray &uid,
                                QNearFieldTarget::AccessMethods accessMethods, int maxInputLength,
                                QObject *parent);
    ~QNearFieldTargetPrivateImpl() override;

    QByteArray uid() const override;
    QNearFieldTarget::Type type() const override;
    QNearFieldTarget::AccessMethods accessMethods() const override;

    bool disconnect() override;

    int maxCommandLength() const override;
    QNearFieldTarget::RequestId sendCommand(const QByteArray &command) override;
    QNearFieldTarget::RequestId readNdefMessages() override;
    QNearFieldTarget::RequestId writeNdefMessages(const QList<QNdefMessage> &messages) override;

private:
    const QByteArray m_uid;
    QNearFieldTarget::AccessMethods m_accessMethods;
    int m_maxInputLength;
    bool m_connected = false;
    bool m_isValid = true;

public Q_SLOTS:
    void onDisconnected();
    void onInvalidated();
    void onRequestCompleted(const QNearFieldTarget::RequestId &request,
                            QNearFieldTarget::Error reason, const QVariant &result);
    void onNdefMessageRead(const QNdefMessage &message);

Q_SIGNALS:
    void disconnectRequest();
    void sendCommandRequest(const QNearFieldTarget::RequestId &request, const QByteArray &command);
    void readNdefMessagesRequest(const QNearFieldTarget::RequestId &request);
    void writeNdefMessagesRequest(const QNearFieldTarget::RequestId &request,
                                  const QList<QNdefMessage> &messages);
    void targetLost(QNearFieldTargetPrivate *target);
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_PCSC_P_H
