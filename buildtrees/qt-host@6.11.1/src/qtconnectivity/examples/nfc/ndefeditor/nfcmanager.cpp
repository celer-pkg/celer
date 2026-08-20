// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "nfcmanager.h"

#include <QNearFieldManager>

#include "nfctarget.h"

NfcManager::NfcManager(QObject *parent) : QObject(parent)
{
    m_manager = new QNearFieldManager(this);

    connect(m_manager, &QNearFieldManager::targetDetected, this, [this](QNearFieldTarget *target) {
        auto jsTarget = new NfcTarget(target);
        QJSEngine::setObjectOwnership(jsTarget, QJSEngine::JavaScriptOwnership);
        Q_EMIT targetDetected(jsTarget);
    });
}

void NfcManager::startTargetDetection()
{
    m_manager->startTargetDetection(QNearFieldTarget::NdefAccess);
}

void NfcManager::stopTargetDetection()
{
    m_manager->stopTargetDetection();
}
