// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qnearfieldmanager_emulator_p.h"
#include "qnearfieldtarget_emulator_p.h"

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl()
{
    TagActivator *activator = TagActivator::instance();
    activator->initialize();

    connect(activator, &TagActivator::tagActivated, this, &QNearFieldManagerPrivateImpl::tagActivated);
    connect(activator, &TagActivator::tagDeactivated, this, &QNearFieldManagerPrivateImpl::tagDeactivated);
}

QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
}

bool QNearFieldManagerPrivateImpl::isEnabled() const
{
    return true;
}

bool QNearFieldManagerPrivateImpl::isSupported(QNearFieldTarget::AccessMethod accessMethod) const
{
    return accessMethod == QNearFieldTarget::NdefAccess
            || accessMethod == QNearFieldTarget::AnyAccess;
}

bool QNearFieldManagerPrivateImpl::startTargetDetection(QNearFieldTarget::AccessMethod accessMethod)
{
    const bool supported = isSupported(accessMethod);
    if (supported)
        TagActivator::instance()->start();

    return supported;
}

void QNearFieldManagerPrivateImpl::stopTargetDetection(const QString &message)
{
    setUserInformation(message);
    TagActivator::instance()->stop();
    emit targetDetectionStopped();
}

void QNearFieldManagerPrivateImpl::setUserInformation(const QString &message)
{
    emit userInformationChanged(message);
}

void QNearFieldManagerPrivateImpl::reset()
{
    TagActivator::instance()->reset();
}

void QNearFieldManagerPrivateImpl::tagActivated(TagBase *tag)
{
    QNearFieldTargetPrivate *target = targets.value(tag).data();
    if (!target) {
        if (dynamic_cast<NfcTagType1 *>(tag))
            target = new TagType1(tag, this);
        else if (dynamic_cast<NfcTagType2 *>(tag))
            target = new TagType2(tag, this);
        else
            qFatal("Unknown emulator tag type");

        targets.insert(tag, target);
    }

    Q_EMIT targetDetected(new QNearFieldTarget(target, this));
}

void QNearFieldManagerPrivateImpl::tagDeactivated(TagBase *tag)
{
    QNearFieldTargetPrivate *target = targets.value(tag).data();
    if (!target) {
        targets.remove(tag);
        return;
    }

    Q_EMIT targetLost(target->q_ptr);
    QMetaObject::invokeMethod(target->q_ptr, &QNearFieldTarget::disconnected);
}

QT_END_NAMESPACE
