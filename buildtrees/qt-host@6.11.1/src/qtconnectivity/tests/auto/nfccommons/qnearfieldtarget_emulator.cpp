// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qnearfieldtarget_emulator_p.h"
#include <QtNfc/private/qnearfieldtarget_p.h>

#include <QtCore/QByteArray>
#include <QtCore/QByteArrayView>
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDirIterator>
#include <QtCore/QMutex>
#include <QtCore/QSettings>

QT_BEGIN_NAMESPACE

static QMutex tagMutex;
static QMap<TagBase *, bool> tagMap;

Q_GLOBAL_STATIC(TagActivator, globalTagActivator)

TagType1::TagType1(TagBase *tag, QObject *parent)
:   QNearFieldTagType1(parent), tag(tag)
{
}

TagType1::~TagType1()
{
}

QByteArray TagType1::uid() const
{
    QMutexLocker locker(&tagMutex);

    return tag->uid();
}

QNearFieldTarget::AccessMethods TagType1::accessMethods() const
{
    return QNearFieldTarget::NdefAccess | QNearFieldTarget::TagTypeSpecificAccess;
}

QNearFieldTarget::RequestId TagType1::sendCommand(const QByteArray &command)
{
    QMutexLocker locker(&tagMutex);

    QNearFieldTarget::RequestId id(new QNearFieldTarget::RequestIdPrivate);

    // tag not in proximity
    if (!tagMap.value(tag)) {
        reportError(QNearFieldTarget::TargetOutOfRangeError, id);
        return id;
    }

    quint16 crc = qChecksum(QByteArrayView(command.constData(), command.size()), Qt::ChecksumItuV41);

    QByteArray response = tag->processCommand(command + char(crc & 0xff) + char(crc >> 8));

    if (response.isEmpty()) {
        reportError(QNearFieldTarget::NoResponseError, id);
        return id;
    }

    // check crc
    if (qChecksum(QByteArrayView(response.constData(), response.size()), Qt::ChecksumItuV41) != 0) {
        reportError(QNearFieldTarget::ChecksumMismatchError, id);
        return id;
    }

    response.chop(2);

    QMetaObject::invokeMethod(this, [this, id, response] {
        this->setResponseForRequest(id, response);
    }, Qt::QueuedConnection);

    return id;
}

bool TagType1::waitForRequestCompleted(const QNearFieldTarget::RequestId &id, int msecs)
{
    QCoreApplication::sendPostedEvents(this, QEvent::MetaCall);

    return QNearFieldTagType1::waitForRequestCompleted(id, msecs);
}


TagType2::TagType2(TagBase *tag, QObject *parent)
:   QNearFieldTagType2(parent), tag(tag)
{
}

TagType2::~TagType2()
{
}

QByteArray TagType2::uid() const
{
    QMutexLocker locker(&tagMutex);

    return tag->uid();
}

QNearFieldTarget::AccessMethods TagType2::accessMethods() const
{
    return QNearFieldTarget::NdefAccess | QNearFieldTarget::TagTypeSpecificAccess;
}

QNearFieldTarget::RequestId TagType2::sendCommand(const QByteArray &command)
{
    QMutexLocker locker(&tagMutex);

    QNearFieldTarget::RequestId id(new QNearFieldTarget::RequestIdPrivate);

    // tag not in proximity
    if (!tagMap.value(tag)) {
        reportError(QNearFieldTarget::TargetOutOfRangeError, id);
        return id;
    }

    quint16 crc = qChecksum(QByteArrayView(command.constData(), command.size()), Qt::ChecksumItuV41);

    QByteArray response = tag->processCommand(command + char(crc & 0xff) + char(crc >> 8));

    if (response.isEmpty())
        return id;

    if (response.size() > 1) {
        // check crc
        if (qChecksum(QByteArrayView(response.constData(), response.size()), Qt::ChecksumItuV41) != 0) {
            reportError(QNearFieldTarget::ChecksumMismatchError, id);
            return id;
        }

        response.chop(2);
    }

    QMetaObject::invokeMethod(this, [this, id, response] {
        this->setResponseForRequest(id, response);
    }, Qt::QueuedConnection);

    return id;
}

bool TagType2::waitForRequestCompleted(const QNearFieldTarget::RequestId &id, int msecs)
{
    QCoreApplication::sendPostedEvents(this, QEvent::MetaCall);

    return QNearFieldTagType2::waitForRequestCompleted(id, msecs);
}

TagActivator::TagActivator() : QObject()
{
    qRegisterMetaType<QNearFieldTarget::Error>();
}

TagActivator::~TagActivator()
{
    QMutexLocker locker(&tagMutex);
    qDeleteAll(tagMap.keys());
    tagMap.clear();
}

void TagActivator::initialize()
{
    QMutexLocker locker(&tagMutex);

    if (!tagMap.isEmpty())
        return;

#ifndef BUILTIN_TESTDATA
    QDirIterator nfcTargets(QDir::currentPath(), QStringList(QStringLiteral("*.nfc")), QDir::Files);
#else
    QDirIterator nfcTargets(":/nfcdata", QStringList(QStringLiteral("*.nfc")), QDir::Files);
#endif
    while (nfcTargets.hasNext()) {
        const QString targetFilename = nfcTargets.next();

        QSettings target(targetFilename, QSettings::IniFormat);

        target.beginGroup(QStringLiteral("Target"));

        const QString tagType = target.value(QStringLiteral("Type")).toString();

        target.endGroup();

        if (tagType == QLatin1String("TagType1")) {
            NfcTagType1 *tag = new NfcTagType1;
            tag->load(&target);

            tagMap.insert(tag, false);
        } else if (tagType == QLatin1String("TagType2")) {
            NfcTagType2 *tag = new NfcTagType2;
            tag->load(&target);

            tagMap.insert(tag, false);
        } else {
            qWarning("Unknown tag type %s\n", qPrintable(tagType));
        }
    }

    current = tagMap.end();
}

void TagActivator::reset()
{
    QMutexLocker locker(&tagMutex);

    stopInternal();

    qDeleteAll(tagMap.keys());
    tagMap.clear();
}

void TagActivator::start()
{
    QMutexLocker locker(&tagMutex);
    timerId = startTimer(1000);
}

void TagActivator::stop()
{
    QMutexLocker locker(&tagMutex);
    stopInternal();
}

void TagActivator::stopInternal()
{
    if (timerId != -1) {
        killTimer(timerId);
        timerId = -1;
    }
}

TagActivator *TagActivator::instance()
{
    return globalTagActivator();
}

void TagActivator::timerEvent(QTimerEvent *e)
{
    Q_UNUSED(e);

    tagMutex.lock();

    if (current != tagMap.end()) {
        if (current.key()->lastAccessTime() + 1500 > QDateTime::currentMSecsSinceEpoch()) {
            tagMutex.unlock();
            return;
        }

        *current = false;

        TagBase *tag = current.key();

        tagMutex.unlock();
        Q_EMIT tagDeactivated(tag);
        tagMutex.lock();
    }

    if (current != tagMap.end())
        ++current;

    if (current == tagMap.end())
        current = tagMap.begin();

    if (current != tagMap.end()) {
        *current = true;

        TagBase *tag = current.key();

        tagMutex.unlock();
        Q_EMIT tagActivated(tag);
        tagMutex.lock();
    }

    tagMutex.unlock();
}

QT_END_NAMESPACE
