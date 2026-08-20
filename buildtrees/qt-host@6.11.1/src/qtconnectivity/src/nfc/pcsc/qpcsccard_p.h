// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPCSCCARD_P_H
#define QPCSCCARD_P_H

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

#include "qpcsc_p.h"
#include "qndefmessage.h"
#include "qnearfieldtarget.h"
#include "ndef/qndefaccessfsm_p.h"

QT_BEGIN_NAMESPACE

class QTimer;

class QPcscCard : public QObject
{
    Q_OBJECT
public:
    QPcscCard(SCARDHANDLE handle, DWORD protocol, QObject *parent = nullptr);
    ~QPcscCard();

    bool isValid() const { return m_isValid; }
    void invalidate();

    bool checkCardPresent();
    Q_INVOKABLE void enableAutodelete();

    QByteArray readUid();
    int readMaxInputLength();

    bool supportsNdef() const { return m_supportsNdef; }

private:
    SCARDHANDLE m_handle;
    SCARD_IO_REQUEST m_ioPci;
    bool m_isValid = true;
    bool m_supportsNdef;
    bool m_autodelete = false;
    // Indicates that an _automatic_ transaction was started
    bool m_inAutoTransaction = false;
    QTimer *m_keepAliveTimer;

    std::unique_ptr<QNdefAccessFsm> m_tagDetectionFsm;

    enum AutoTransaction { NoAutoTransaction, StartAutoTransaction };

    QPcsc::RawCommandResult sendCommand(const QByteArray &command, AutoTransaction autoTransaction);
    void performNdefDetection();

    class Transaction
    {
    public:
        Transaction(QPcscCard *card);
        ~Transaction();

    private:
        QPcscCard *m_card;
        bool m_initiated = false;
    };

public Q_SLOTS:
    void onDisconnectRequest();
    void onTargetDestroyed();
    void onSendCommandRequest(const QNearFieldTarget::RequestId &request,
                              const QByteArray &command);
    void onReadNdefMessagesRequest(const QNearFieldTarget::RequestId &request);
    void onWriteNdefMessagesRequest(const QNearFieldTarget::RequestId &request,
                                    const QList<QNdefMessage> &messages);

private Q_SLOTS:
    void onKeepAliveTimeout();

Q_SIGNALS:
    void disconnected();
    void invalidated();

    void requestCompleted(const QNearFieldTarget::RequestId &request,
                          QNearFieldTarget::Error reason, const QVariant &result);
    void ndefMessageRead(const QNdefMessage &message);
};

QT_END_NAMESPACE

#endif // QPCSCCARD_P_H
