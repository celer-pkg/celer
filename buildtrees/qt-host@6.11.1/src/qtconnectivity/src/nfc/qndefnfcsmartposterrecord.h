// Copyright (C) 2016 - 2012 Research In Motion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNDEFNFCSMARTPOSTERRECORD_H
#define QNDEFNFCSMARTPOSTERRECORD_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtNfc/qtnfcglobal.h>
#include <QtNfc/QNdefRecord>
#include <QtNfc/qndefnfctextrecord.h>
#include <QtNfc/qndefnfcurirecord.h>

QT_FORWARD_DECLARE_CLASS(QUrl)

QT_BEGIN_NAMESPACE

class QNdefNfcSmartPosterRecordPrivate;

#define Q_DECLARE_ISRECORDTYPE_FOR_MIME_NDEF_RECORD(className) \
    QT_BEGIN_NAMESPACE \
    template<> inline bool QNdefRecord::isRecordType<className>() const\
    { \
        return (typeNameFormat() == QNdefRecord::Mime); \
    } \
    QT_END_NAMESPACE

#define Q_DECLARE_MIME_NDEF_RECORD(className, initialPayload) \
    className() : QNdefRecord(QNdefRecord::Mime, "") { setPayload(initialPayload); } \
    className(const QNdefRecord &other) : QNdefRecord(other, QNdefRecord::Mime) { }

class Q_NFC_EXPORT QNdefNfcIconRecord : public QNdefRecord
{
public:
    Q_DECLARE_MIME_NDEF_RECORD(QNdefNfcIconRecord, QByteArray(0, char(0)))

    void setData(const QByteArray &data);
    QByteArray data() const;
};

class Q_NFC_EXPORT QNdefNfcSmartPosterRecord : public QNdefRecord
{
public:
    enum Action {
        UnspecifiedAction = -1,
        DoAction = 0,
        SaveAction = 1,
        EditAction = 2
    };

    QNdefNfcSmartPosterRecord();
    QNdefNfcSmartPosterRecord(const QNdefRecord &other);
    QNdefNfcSmartPosterRecord(const QNdefNfcSmartPosterRecord &other);
    QNdefNfcSmartPosterRecord &operator=(const QNdefNfcSmartPosterRecord &other);
    ~QNdefNfcSmartPosterRecord();

    void setPayload(const QByteArray &payload);

    bool hasTitle(const QString &locale = QString()) const;
    bool hasAction() const;
    bool hasIcon(const QByteArray &mimetype = QByteArray()) const;
    bool hasSize() const;
    bool hasTypeInfo() const;

    qsizetype titleCount() const;
    QNdefNfcTextRecord titleRecord(qsizetype index) const;
    QString title(const QString &locale = QString()) const;
    QList<QNdefNfcTextRecord> titleRecords() const;

    bool addTitle(const QNdefNfcTextRecord &text);
    bool addTitle(const QString &text, const QString &locale, QNdefNfcTextRecord::Encoding encoding);
    bool removeTitle(const QNdefNfcTextRecord &text);
    bool removeTitle(const QString &locale);
    void setTitles(const QList<QNdefNfcTextRecord> &titles);

    QUrl uri() const;
    QNdefNfcUriRecord uriRecord() const;
    void setUri(const QNdefNfcUriRecord &url);
    void setUri(const QUrl &url);

    Action action() const;
    void setAction(Action act);

    qsizetype iconCount() const;
    QNdefNfcIconRecord iconRecord(qsizetype index) const;
    QByteArray icon(const QByteArray& mimetype = QByteArray()) const;

    QList<QNdefNfcIconRecord> iconRecords() const;

    void addIcon(const QNdefNfcIconRecord &icon);
    void addIcon(const QByteArray &type, const QByteArray &data);
    bool removeIcon(const QNdefNfcIconRecord &icon);
    bool removeIcon(const QByteArray &type);
    void setIcons(const QList<QNdefNfcIconRecord> &icons);

    quint32 size() const;
    void setSize(quint32 size);

    QString typeInfo() const;
    void setTypeInfo(const QString &type);

private:
    QSharedDataPointer<QNdefNfcSmartPosterRecordPrivate> d;

    void cleanup();
    void convertToPayload();

    bool addTitleInternal(const QNdefNfcTextRecord &text);
    void addIconInternal(const QNdefNfcIconRecord &icon);
};

QT_END_NAMESPACE

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcSmartPosterRecord, QNdefRecord::NfcRtd, "Sp")
Q_DECLARE_ISRECORDTYPE_FOR_MIME_NDEF_RECORD(QNdefNfcIconRecord)

#endif // QNDEFNFCSMARTPOSTERRECORD_H
