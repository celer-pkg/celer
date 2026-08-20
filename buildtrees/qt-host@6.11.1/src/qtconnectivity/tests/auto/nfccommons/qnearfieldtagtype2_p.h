// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QNEARFIELDTAGTYPE2_H
#define QNEARFIELDTAGTYPE2_H

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

class QNearFieldTagType2Private;

class QNearFieldTagType2 : public QNearFieldTargetPrivate
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(QNearFieldTagType2)

public:
    explicit QNearFieldTagType2(QObject *parent = nullptr);
    ~QNearFieldTagType2();

    QNearFieldTarget::Type type() const override { return QNearFieldTarget::NfcTagType2; }

    bool hasNdefMessage() override;
    QNearFieldTarget::RequestId readNdefMessages() override;
    QNearFieldTarget::RequestId writeNdefMessages(const QList<QNdefMessage> &messages) override;

    quint8 version();
    int memorySize();

    virtual QNearFieldTarget::RequestId readBlock(quint8 blockAddress);
    virtual QNearFieldTarget::RequestId writeBlock(quint8 blockAddress, const QByteArray &data);
    virtual QNearFieldTarget::RequestId selectSector(quint8 sector);

    void timerEvent(QTimerEvent *event) override;

protected:
    void setResponseForRequest(const QNearFieldTarget::RequestId &id, const QVariant &response, bool emitRequestCompleted = true) override;

private:
    QNearFieldTagType2Private *d_ptr;
};

QT_END_NAMESPACE

#endif // QNEARFIELDTAGTYPE2_H
