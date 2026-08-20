// Copyright (C) 2016 - 2012 Research In Motion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNDEFNFCSMARTPOSTERRECORD_P_H
#define QNDEFNFCSMARTPOSTERRECORD_P_H

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

QT_BEGIN_NAMESPACE

class QNdefNfcActRecord : public QNdefRecord
{
public:
    Q_DECLARE_NDEF_RECORD(QNdefNfcActRecord, QNdefRecord::NfcRtd, "act", QByteArray(0, char(0)))

    void setAction(QNdefNfcSmartPosterRecord::Action action);
    QNdefNfcSmartPosterRecord::Action action() const;
};

class QNdefNfcSizeRecord : public QNdefRecord
{
public:
    Q_DECLARE_NDEF_RECORD(QNdefNfcSizeRecord, QNdefRecord::NfcRtd, "s", QByteArray(0, char(0)))

    void setSize(quint32 size);
    quint32 size() const;
};

class QNdefNfcTypeRecord : public QNdefRecord
{
public:
    Q_DECLARE_NDEF_RECORD(QNdefNfcTypeRecord, QNdefRecord::NfcRtd, "t", QByteArray(0, char(0)))

    void setTypeInfo(const QString &type);
    QString typeInfo() const;
};

class QNdefNfcSmartPosterRecordPrivate : public QSharedData
{
public:
    QNdefNfcSmartPosterRecordPrivate() {}
    ~QNdefNfcSmartPosterRecordPrivate();

    void cleanup();

public:
    QList<QNdefNfcTextRecord> m_titleList;
    QNdefNfcUriRecord *m_uri = nullptr;
    QNdefNfcActRecord *m_action = nullptr;
    QList<QNdefNfcIconRecord> m_iconList;
    QNdefNfcSizeRecord *m_size = nullptr;
    QNdefNfcTypeRecord *m_type = nullptr;
};

QT_END_NAMESPACE

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcActRecord, QNdefRecord::NfcRtd, "act")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcSizeRecord, QNdefRecord::NfcRtd, "s")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcTypeRecord, QNdefRecord::NfcRtd, "t")

#endif // QNDEFNFCSMARTPOSTERRECORD_P_H
