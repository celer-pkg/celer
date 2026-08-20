// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "nfctarget.h"

NfcTarget::NfcTarget(QNearFieldTarget *target, QObject *parent) : QObject(parent), m_target(target)
{
    target->setParent(this);

    connect(target, &QNearFieldTarget::ndefMessageRead, this, &NfcTarget::ndefMessageRead);
    connect(target, &QNearFieldTarget::requestCompleted, this, &NfcTarget::requestCompleted);
    connect(target, &QNearFieldTarget::error, this, &NfcTarget::error);
}

bool NfcTarget::readNdefMessages()
{
    if (m_target.isNull())
        return false;

    auto req = m_target->readNdefMessages();
    return req.isValid();
}

bool NfcTarget::writeNdefMessage(const QNdefMessage &message)
{
    if (m_target.isNull())
        return false;

    auto req = m_target->writeNdefMessages({ message });
    return req.isValid();
}
