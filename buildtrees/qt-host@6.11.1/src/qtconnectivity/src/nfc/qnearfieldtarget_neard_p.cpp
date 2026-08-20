// Copyright (C) 2016 BlackBerry Limited, Copyright (C) 2016 BasysKom GmbH
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qnearfieldtarget_neard_p.h"

#include <qndefnfctextrecord.h>
#include <qndefnfcsmartposterrecord.h>
#include <qndefnfcurirecord.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_NFC_NEARD)

using namespace QtNfcPrivate; // for D-Bus wrappers

QNearFieldTargetPrivateImpl::QNearFieldTargetPrivateImpl(QObject *parent, QDBusObjectPath interfacePath)
    : QNearFieldTargetPrivate(parent),
      m_tagPath(interfacePath),
      m_readRequested(false)
{
    m_readErrorTimer.setSingleShot(true);
    m_recordPathsCollectedTimer.setSingleShot(true);
    m_delayedWriteTimer.setSingleShot(true);

    qCDebug(QT_NFC_NEARD) << "tag found at path" << interfacePath.path();
    m_dbusProperties = new OrgFreedesktopDBusPropertiesInterface(QStringLiteral("org.neard"),
                                        interfacePath.path(), QDBusConnection::systemBus(), this);
    if (!m_dbusProperties->isValid()) {
        qCWarning(QT_NFC_NEARD) << "Could not connect to dbus property interface at path"
                                << interfacePath.path();
        return;
    }

    QDBusPendingReply<QVariantMap> reply = m_dbusProperties->GetAll(QStringLiteral("org.neard.Tag"));
    reply.waitForFinished();
    if (reply.isError()) {
        qCWarning(QT_NFC_NEARD) << "Could not get properties of org.neard.Tag dbus interface";
        return;
    }

    const QVariantMap props = reply.value();
    const QString type = props.value(QStringLiteral("Type")).toString();
    m_type = QNearFieldTarget::ProprietaryTag;

    if (type == QStringLiteral("Type 1"))
        m_type = QNearFieldTarget::NfcTagType1;
    else if (type == QStringLiteral("Type 2"))
        m_type = QNearFieldTarget::NfcTagType2;
    else if (type == QStringLiteral("Type 3"))
        m_type = QNearFieldTarget::NfcTagType3;
    else if (type == QStringLiteral("Type 4"))
        m_type = QNearFieldTarget::NfcTagType4;

    qCDebug(QT_NFC_NEARD) << "tag type" << type;

    m_uid = props.value(QStringLiteral("Uid")).toByteArray();
    qCDebug(QT_NFC_NEARD) << "tag UID" << m_uid.toHex();

    connect(&m_recordPathsCollectedTimer, &QTimer::timeout,
            this, &QNearFieldTargetPrivateImpl::createNdefMessage);
    connect(&m_readErrorTimer, &QTimer::timeout,
            this, &QNearFieldTargetPrivateImpl::handleReadError);
    connect(&m_delayedWriteTimer, &QTimer::timeout,
            this, &QNearFieldTargetPrivateImpl::handleWriteRequest);
    connect(NeardHelper::instance(), &NeardHelper::recordFound,
            this, &QNearFieldTargetPrivateImpl::handleRecordFound);
}

QNearFieldTargetPrivateImpl::~QNearFieldTargetPrivateImpl()
{
}

bool QNearFieldTargetPrivateImpl::isValid()
{
    return m_dbusProperties->isValid() && NeardHelper::instance()->dbusObjectManager()->isValid();
}

QByteArray QNearFieldTargetPrivateImpl::uid() const
{
    return m_uid;
}

QNearFieldTarget::Type QNearFieldTargetPrivateImpl::type() const
{
    return m_type;
}

QNearFieldTarget::AccessMethods QNearFieldTargetPrivateImpl::accessMethods() const
{
    return QNearFieldTarget::NdefAccess;
}

bool QNearFieldTargetPrivateImpl::hasNdefMessage()
{
    return !m_recordPaths.isEmpty();
}

QNearFieldTarget::RequestId QNearFieldTargetPrivateImpl::readNdefMessages()
{
    if (isValid()) {
        // if the user calls readNdefMessages before the previous request has been completed
        // return the current request id.
        if (m_currentReadRequestId.isValid())
            return m_currentReadRequestId;

        QNearFieldTarget::RequestId requestId = QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate());
        // save the id so it can be passed along with requestCompleted
        m_currentReadRequestId = requestId;
        // since the triggering of interfaceAdded will ultimately lead to createNdefMessage being called
        // we need to make sure that ndefMessagesRead will only be triggered when readNdefMessages has
        // been called before. In case readNdefMessages is called again after that we can directly call
        // call createNdefMessage.
        m_readRequested = true;
        if (hasNdefMessage())
            createNdefMessage();
        else
            m_readErrorTimer.start(1000);

        return requestId;
    } else {
        return QNearFieldTarget::RequestId();
    }
}

QNearFieldTarget::RequestId QNearFieldTargetPrivateImpl::sendCommand(const QByteArray &command)
{
    Q_UNUSED(command);
    return QNearFieldTarget::RequestId();
}

QNearFieldTarget::RequestId QNearFieldTargetPrivateImpl::writeNdefMessages(const QList<QNdefMessage> &messages)
{
    // disabling write due to neard crash (see QTBUG-43802)
    qWarning("QNearFieldTarget::WriteNdefMessages() disabled. See QTBUG-43802\n");
    return QNearFieldTarget::RequestId();


    // return old request id when previous write request hasn't completed
    if (m_currentWriteRequestId.isValid())
        return m_currentWriteRequestId;

    qCDebug(QT_NFC_NEARD) << "writing messages";
    if (messages.isEmpty() || messages.first().isEmpty()) {
        qCWarning(QT_NFC_NEARD) << "No record specified";
        return QNearFieldTarget::RequestId();
    }
    if (messages.count() > 1 || messages.first().count() > 1) {
        // neard only supports one ndef record per tag
        qCWarning(QT_NFC_NEARD) << "Writing of only one NDEF record and message is supported";
        return QNearFieldTarget::RequestId();
    }
    QNdefRecord record = messages.first().first();

    if (record.typeNameFormat() == QNdefRecord::NfcRtd) {
        m_currentWriteRequestData.clear();
        if (record.isRecordType<QNdefNfcUriRecord>()) {
            m_currentWriteRequestData.insert(QStringLiteral("Type"), QStringLiteral("URI"));
            QNdefNfcUriRecord uriRecord = static_cast<QNdefNfcUriRecord>(record);
            m_currentWriteRequestData.insert(QStringLiteral("URI"), uriRecord.uri().toString());
        } else if (record.isRecordType<QNdefNfcSmartPosterRecord>()) {
            m_currentWriteRequestData.insert(QStringLiteral("Type"), QStringLiteral("SmartPoster"));
            QNdefNfcSmartPosterRecord spRecord = static_cast<QNdefNfcSmartPosterRecord>(record);
            m_currentWriteRequestData.insert(QStringLiteral("URI"), spRecord.uri().toString());
            // Currently neard only supports the uri property for writing
        } else if (record.isRecordType<QNdefNfcTextRecord>()) {
            m_currentWriteRequestData.insert(QStringLiteral("Type"), QStringLiteral("Text"));
            QNdefNfcTextRecord textRecord = static_cast<QNdefNfcTextRecord>(record);
            m_currentWriteRequestData.insert(QStringLiteral("Representation"), textRecord.text());
            m_currentWriteRequestData.insert(QStringLiteral("Encoding"),
                                    textRecord.encoding() == QNdefNfcTextRecord::Utf8 ?
                                            QStringLiteral("UTF-8") : QStringLiteral("UTF-16") );
            m_currentWriteRequestData.insert(QStringLiteral("Language"), textRecord.locale());
        } else {
            qCWarning(QT_NFC_NEARD) << "Record type not supported for writing";
            return QNearFieldTarget::RequestId();
        }

        m_currentWriteRequestId = QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate());
        // trigger delayed write
        m_delayedWriteTimer.start(100);

        return m_currentWriteRequestId;
    }

    return QNearFieldTarget::RequestId();
}

QNdefRecord QNearFieldTargetPrivateImpl::readRecord(const QDBusObjectPath &path)
{
    qCDebug(QT_NFC_NEARD) << "reading record for path" << path.path();
    OrgFreedesktopDBusPropertiesInterface recordInterface(QStringLiteral("org.neard"), path.path(),
                                                          QDBusConnection::systemBus());
    if (!recordInterface.isValid())
        return QNdefRecord();

    QDBusPendingReply<QVariantMap> reply = recordInterface.GetAll(QStringLiteral("org.neard.Record"));
    reply.waitForFinished();
    if (reply.isError())
        return QNdefRecord();

    const QString &value = reply.value().value(QStringLiteral("Representation")).toString();
    const QString &locale = reply.value().value(QStringLiteral("Language")).toString();
    const QString &encoding = reply.value().value(QStringLiteral("Encoding")).toString();
    const QString &uri = reply.value().value(QStringLiteral("URI")).toString();

    //const QString &mime = reply.value().value(QStringLiteral("MIME")).toString();
    //const QString &arr = reply.value().value(QStringLiteral("ARR")).toString();

    const QString type = reply.value().value(QStringLiteral("Type")).toString();
    if (type == QStringLiteral("Text")) {
        QNdefNfcTextRecord textRecord;
        textRecord.setText(value);
        textRecord.setLocale(locale);
        textRecord.setEncoding((encoding == QStringLiteral("UTF-8")) ? QNdefNfcTextRecord::Utf8
                                                                     : QNdefNfcTextRecord::Utf16);
        return textRecord;
    } else if (type == QStringLiteral("SmartPoster")) {
        QNdefNfcSmartPosterRecord spRecord;
        if (!value.isEmpty()) {
            spRecord.addTitle(value, locale, (encoding == QStringLiteral("UTF-8"))
                                                    ? QNdefNfcTextRecord::Utf8
                                                    : QNdefNfcTextRecord::Utf16);
        }

        if (!uri.isEmpty())
            spRecord.setUri(QUrl(uri));

        const QString &action = reply.value().value(QStringLiteral("Action")).toString();
        if (!action.isEmpty()) {
            if (action == QStringLiteral("Do"))
                spRecord.setAction(QNdefNfcSmartPosterRecord::DoAction);
            else if (action == QStringLiteral("Save"))
                spRecord.setAction(QNdefNfcSmartPosterRecord::SaveAction);
            else if (action == QStringLiteral("Edit"))
                spRecord.setAction(QNdefNfcSmartPosterRecord::EditAction);
        }

        if (reply.value().contains(QStringLiteral("Size"))) {
            uint size = reply.value().value(QStringLiteral("Size")).toUInt();
            spRecord.setSize(size);
        }

        const QString &mimeType = reply.value().value(QStringLiteral("MIMEType")).toString();
        if (!mimeType.isEmpty()) {
            spRecord.setTypeInfo(mimeType);
        }


        return spRecord;
    } else if (type == QStringLiteral("URI")) {
        QNdefNfcUriRecord uriRecord;
        uriRecord.setUri(QUrl(uri));
        return uriRecord;
    } else if (type == QStringLiteral("MIME")) {

    } else if (type == QStringLiteral("AAR")) {

    }

    return QNdefRecord();
}

void QNearFieldTargetPrivateImpl::handleRecordFound(const QDBusObjectPath &path)
{
    m_recordPaths.append(path);
    // FIXME: this timer only exists because neard doesn't currently supply enough
    // information to let us know when all record interfaces have been added or
    // how many records are actually contained on a tag. We assume that when no
    // signal has been received for 100ms all record interfaces have been added.
    m_recordPathsCollectedTimer.start(100);
    // as soon as record paths have been added we can handle errors without the timer.
    m_readErrorTimer.stop();
}

void QNearFieldTargetPrivateImpl::createNdefMessage()
{
    if (m_readRequested) {
        qCDebug(QT_NFC_NEARD) << "creating Ndef message, reading" << m_recordPaths.length() << "record paths";
        QNdefMessage newNdefMessage;
        for (const QDBusObjectPath &recordPath : std::as_const(m_recordPaths))
            newNdefMessage.append(readRecord(recordPath));

        if (!newNdefMessage.isEmpty()) {
            QMetaObject::invokeMethod(this, "ndefMessageRead", Qt::QueuedConnection,
                                      Q_ARG(QNdefMessage, newNdefMessage));
            // the request id in requestCompleted has to match the one created in readNdefMessages
            QMetaObject::invokeMethod(this, [this]() {
                Q_EMIT requestCompleted(m_currentReadRequestId);
            }, Qt::QueuedConnection);
        } else {
            reportError(QNearFieldTarget::UnknownError, m_currentReadRequestId);
        }

        m_readRequested = false;
        // invalidate the current request id
        m_currentReadRequestId = QNearFieldTarget::RequestId(0);
    }
}

void QNearFieldTargetPrivateImpl::handleReadError()
{
    reportError(QNearFieldTarget::UnknownError, m_currentReadRequestId);
    m_currentReadRequestId = QNearFieldTarget::RequestId(0);
}

void QNearFieldTargetPrivateImpl::handleWriteRequest()
{
    OrgNeardTagInterface tagInterface(QStringLiteral("org.neard"), m_tagPath.path(),
                                      QDBusConnection::systemBus());
    if (!tagInterface.isValid()) {
        qCWarning(QT_NFC_NEARD) << "tag interface invalid";
    } else {
        QDBusPendingReply<> reply;
        reply = tagInterface.Write(m_currentWriteRequestData);
        reply.waitForFinished();
        if (reply.isError()) {
            qCWarning(QT_NFC_NEARD) << "Error writing to NFC tag" << reply.error();
            reportError(QNearFieldTarget::UnknownError, m_currentWriteRequestId);
        }

        QMetaObject::invokeMethod(this, [this]() {
            Q_EMIT requestCompleted(m_currentWriteRequestId);
        }, Qt::QueuedConnection);
    }

    // invalidate current write request
    m_currentWriteRequestId = QNearFieldTarget::RequestId(0);
}

QT_END_NAMESPACE
