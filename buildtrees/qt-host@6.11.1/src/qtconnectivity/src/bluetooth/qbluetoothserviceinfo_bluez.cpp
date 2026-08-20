// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothserviceinfo.h"
#include "qbluetoothserviceinfo_p.h"

#include "bluez/bluez5_helper_p.h"
#include "bluez/profilemanager1_p.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/QXmlStreamWriter>
#include <QtCore/QAtomicInt>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;
using namespace QtBluetoothPrivate; // for D-Bus wrappers

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

static QAtomicInt pathCounter;

static void writeAttribute(QXmlStreamWriter *stream, const QVariant &attribute)
{
    const QString unsignedFormat(QStringLiteral("0x%1"));

    switch (attribute.typeId()) {
    case QMetaType::Void:
        stream->writeEmptyElement(QStringLiteral("nil"));
        break;
    case QMetaType::UChar:
        stream->writeEmptyElement(QStringLiteral("uint8"));
        stream->writeAttribute(QStringLiteral("value"),
                               unsignedFormat.arg(attribute.value<quint8>(), 2, 16,
                                                  QLatin1Char('0')));
        break;
    case QMetaType::UShort:
        stream->writeEmptyElement(QStringLiteral("uint16"));
        stream->writeAttribute(QStringLiteral("value"),
                               unsignedFormat.arg(attribute.value<quint16>(), 4, 16,
                                                  QLatin1Char('0')));
        break;
    case QMetaType::UInt:
        stream->writeEmptyElement(QStringLiteral("uint32"));
        stream->writeAttribute(QStringLiteral("value"),
                               unsignedFormat.arg(attribute.value<quint32>(), 8, 16,
                                                  QLatin1Char('0')));
        break;
    case QMetaType::Char:
        stream->writeEmptyElement(QStringLiteral("int8"));
        stream->writeAttribute(QStringLiteral("value"),
                               QString::number(attribute.value<qint8>()));
        break;
    case QMetaType::Short:
        stream->writeEmptyElement(QStringLiteral("int16"));
        stream->writeAttribute(QStringLiteral("value"),
                               QString::number(attribute.value<qint16>()));
        break;
    case QMetaType::Int:
        stream->writeEmptyElement(QStringLiteral("int32"));
        stream->writeAttribute(QStringLiteral("value"),
                               QString::number(attribute.value<qint32>()));
        break;
    case QMetaType::QByteArray:
        stream->writeEmptyElement(QStringLiteral("text"));
        stream->writeAttribute(QStringLiteral("value"),
                               QString::fromLatin1(attribute.value<QByteArray>().toHex().constData()));
        stream->writeAttribute(QStringLiteral("encoding"), QStringLiteral("hex"));
        break;
    case QMetaType::QString:
        stream->writeEmptyElement(QStringLiteral("text"));
        stream->writeAttribute(QStringLiteral("value"), attribute.value<QString>());
        stream->writeAttribute(QStringLiteral("encoding"), QStringLiteral("normal"));
        break;
    case QMetaType::Bool:
        stream->writeEmptyElement(QStringLiteral("boolean"));
        if (attribute.value<bool>())
            stream->writeAttribute(QStringLiteral("value"), QStringLiteral("true"));
        else
            stream->writeAttribute(QStringLiteral("value"), QStringLiteral("false"));
        break;
    case QMetaType::QUrl:
        stream->writeEmptyElement(QStringLiteral("url"));
        stream->writeAttribute(QStringLiteral("value"), attribute.value<QUrl>().toString());
        break;
    default:
        if (attribute.userType() == qMetaTypeId<QBluetoothUuid>()) {
            stream->writeEmptyElement(QStringLiteral("uuid"));

            QBluetoothUuid uuid = attribute.value<QBluetoothUuid>();
            switch (uuid.minimumSize()) {
            case 0:
                stream->writeAttribute(QStringLiteral("value"),
                                       unsignedFormat.arg(quint16(0), 4, 16, QLatin1Char('0')));
                break;
            case 2:
                stream->writeAttribute(QStringLiteral("value"),
                                       unsignedFormat.arg(uuid.toUInt16(), 4, 16,
                                                          QLatin1Char('0')));
                break;
            case 4:
                stream->writeAttribute(QStringLiteral("value"),
                                       unsignedFormat.arg(uuid.toUInt32(), 8, 16,
                                                          QLatin1Char('0')));
                break;
            case 16:
                stream->writeAttribute(QStringLiteral("value"), uuid.toString().mid(1, 36));
                break;
            default:
                stream->writeAttribute(QStringLiteral("value"), uuid.toString().mid(1, 36));
            }
        } else if (attribute.userType() == qMetaTypeId<QBluetoothServiceInfo::Sequence>()) {
            stream->writeStartElement(QStringLiteral("sequence"));
            const QBluetoothServiceInfo::Sequence *sequence =
                    static_cast<const QBluetoothServiceInfo::Sequence *>(attribute.data());
            for (const QVariant &v : *sequence)
                writeAttribute(stream, v);
            stream->writeEndElement();
        } else if (attribute.userType() == qMetaTypeId<QBluetoothServiceInfo::Alternative>()) {
            const QBluetoothServiceInfo::Alternative *alternative =
                    static_cast<const QBluetoothServiceInfo::Alternative *>(attribute.data());
            for (const QVariant &v : *alternative)
                writeAttribute(stream, v);
            stream->writeEndElement();
        } else {
            qCWarning(QT_BT_BLUEZ) << "Unknown variant type" << attribute.userType();
        }
    }
}

QBluetoothServiceInfoPrivate::QBluetoothServiceInfoPrivate()
:   serviceRecord(0), registered(false)
{
    initializeBluez5();
    service = new OrgBluezProfileManager1Interface(QStringLiteral("org.bluez"),
                                                   QStringLiteral("/org/bluez"),
                                                   QDBusConnection::systemBus(), this);
}

QBluetoothServiceInfoPrivate::~QBluetoothServiceInfoPrivate()
{
}

bool QBluetoothServiceInfoPrivate::isRegistered() const
{
    return registered;
}

bool QBluetoothServiceInfoPrivate::unregisterService()
{
    if (!registered)
        return false;

    if (profilePath.isEmpty())
        return false;

    QDBusPendingReply<> reply = service->UnregisterProfile(QDBusObjectPath(profilePath));
    reply.waitForFinished();
    if (reply.isError()) {
        qCWarning(QT_BT_BLUEZ) << "Cannot unregister profile:" << profilePath
                               << reply.error().message();
        return false;
    }
    profilePath.clear();

    registered = false;
    return true;
}

// TODO Implement local adapter behavior
bool QBluetoothServiceInfoPrivate::registerService(const QBluetoothAddress & /*localAdapter*/)
{
    if (registered)
        return false;

    QString xmlServiceRecord;

    QXmlStreamWriter stream(&xmlServiceRecord);
    stream.setAutoFormatting(true);

    stream.writeStartDocument(QStringLiteral("1.0"));

    stream.writeStartElement(QStringLiteral("record"));

    const QString unsignedFormat(QStringLiteral("0x%1"));

    QMap<quint16, QVariant>::ConstIterator i = attributes.constBegin();
    while (i != attributes.constEnd()) {
        stream.writeStartElement(QStringLiteral("attribute"));
        stream.writeAttribute(QStringLiteral("id"), unsignedFormat.arg(i.key(), 4, 16, QLatin1Char('0')));
        writeAttribute(&stream, i.value());
        stream.writeEndElement();

        ++i;
    }

    stream.writeEndElement();

    stream.writeEndDocument();

    // create path
    profilePath = u"/qt/profile"_s;
    profilePath.append(QString::fromLatin1("/%1%2/%3")
                               .arg(sanitizeNameForDBus(QCoreApplication::applicationName()))
                               .arg(QCoreApplication::applicationPid())
                               .arg(pathCounter.fetchAndAddOrdered(1)));

    QVariantMap mapping;
    mapping.insert(QStringLiteral("ServiceRecord"), xmlServiceRecord);
    mapping.insert(QStringLiteral("Role"), QStringLiteral("server"));

    // Strategy to pick service uuid
    // 1.) use serviceUuid()
    // 2.) use first custom uuid if available
    // 3.) use first service class uuid
    QBluetoothUuid profileUuid =
            attributes.value(QBluetoothServiceInfo::ServiceId).value<QBluetoothUuid>();
    QBluetoothUuid firstCustomUuid;
    if (profileUuid.isNull()) {
        const QVariant var = attributes.value(QBluetoothServiceInfo::ServiceClassIds);
        if (var.isValid()) {
            const QBluetoothServiceInfo::Sequence seq =
                    var.value<QBluetoothServiceInfo::Sequence>();
            for (const auto &e : seq) {
                auto tempUuid = e.value<QBluetoothUuid>();
                if (tempUuid.isNull())
                    continue;

                int size = tempUuid.minimumSize();
                if (size == 2 || size == 4) { // Base UUID derived
                    if (profileUuid.isNull())
                        profileUuid = tempUuid;
                } else if (firstCustomUuid.isNull()) {
                    firstCustomUuid = tempUuid;
                }
            }
        }
    }

    if (!firstCustomUuid.isNull())
        profileUuid = firstCustomUuid;

    QString uuidString = profileUuid.toString(QUuid::WithoutBraces);

    qCDebug(QT_BT_BLUEZ) << "Registering profile under" << profilePath << uuidString;

    QDBusPendingReply<> reply =
            service->RegisterProfile(QDBusObjectPath(profilePath), uuidString, mapping);
    reply.waitForFinished();
    if (reply.isError()) {
        qCWarning(QT_BT_BLUEZ) << "Cannot register profile" << reply.error().message();
        return false;
    }

    registered = true;
    return true;
}

QT_END_NAMESPACE
