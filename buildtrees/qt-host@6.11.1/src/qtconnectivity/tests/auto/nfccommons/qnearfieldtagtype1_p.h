// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QNEARFIELDTAGTYPE1_H
#define QNEARFIELDTAGTYPE1_H

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

#include <QtNfc/private/qnearfieldtarget_p.h>

QT_BEGIN_NAMESPACE

class QNearFieldTagType1Private;

class QNearFieldTagType1 : public QNearFieldTargetPrivate
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(QNearFieldTagType1)

public:
    enum WriteMode {
        EraseAndWrite,
        WriteOnly
    };
    Q_ENUM(WriteMode)

    explicit QNearFieldTagType1(QObject *parent = nullptr);
    ~QNearFieldTagType1();

    QNearFieldTarget::Type type() const override { return QNearFieldTarget::NfcTagType1; }

    bool hasNdefMessage() override;
    QNearFieldTarget::RequestId readNdefMessages() override;
    QNearFieldTarget::RequestId writeNdefMessages(const QList<QNdefMessage> &messages) override;

    quint8 version();
    virtual int memorySize();

    // DIGPROTO
    virtual QNearFieldTarget::RequestId readIdentification();

    // static memory functions
    virtual QNearFieldTarget::RequestId readAll();
    virtual QNearFieldTarget::RequestId readByte(quint8 address);
    virtual QNearFieldTarget::RequestId writeByte(quint8 address, quint8 data, WriteMode mode = EraseAndWrite);

    // dynamic memory functions
    virtual QNearFieldTarget::RequestId readSegment(quint8 segmentAddress);
    virtual QNearFieldTarget::RequestId readBlock(quint8 blockAddress);
    virtual QNearFieldTarget::RequestId writeBlock(quint8 blockAddress, const QByteArray &data,
                                                   WriteMode mode = EraseAndWrite);

protected:
    void setResponseForRequest(const QNearFieldTarget::RequestId &id, const QVariant &response, bool emitRequestCompleted = true) override;

private:
    QNearFieldTagType1Private *d_ptr;
};

QT_END_NAMESPACE

#endif // QNEARFIELDTAGTYPE1_H
