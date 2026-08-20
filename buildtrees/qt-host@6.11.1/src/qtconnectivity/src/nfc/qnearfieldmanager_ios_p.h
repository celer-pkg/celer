// Copyright (C) 2020 Governikus GmbH & Co. KG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNEARFIELDMANAGER_IOS_P_H
#define QNEARFIELDMANAGER_IOS_P_H

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
#include <QtCore/private/qcore_mac_p.h>

#include "qnearfieldmanager_p.h"

#include <QPointer>
#include <QTimer>

#import <os/availability.h>

Q_FORWARD_DECLARE_OBJC_CLASS(QT_MANGLE_NAMESPACE(QIosTagReaderDelegate));

Q_FORWARD_DECLARE_OBJC_CLASS(QT_MANGLE_NAMESPACE(QIosNfcNdefSessionDelegate));
QT_NAMESPACE_ALIAS_OBJC_CLASS(QIosNfcNdefSessionDelegate);

QT_BEGIN_NAMESPACE

class QNearFieldTargetPrivateImpl;

class QNearFieldManagerPrivateImpl : public QNearFieldManagerPrivate
{
    Q_OBJECT

public:
    QNearFieldManagerPrivateImpl();
    ~QNearFieldManagerPrivateImpl();

    bool isEnabled() const override
    {
        return true;
    }

    bool isSupported(QNearFieldTarget::AccessMethod accessMethod) const override;

    bool startTargetDetection(QNearFieldTarget::AccessMethod accessMethod) override;
    void stopTargetDetection(const QString &errorMessage) override;

    void setUserInformation(const QString &message) override;

Q_SIGNALS:
    void tagDiscovered(void *tag);
    void didInvalidateWithError(bool doRestart);

private:
    QT_MANGLE_NAMESPACE(QIosTagReaderDelegate) *delegate API_AVAILABLE(ios(13.0)) = nullptr;
    QIosNfcNdefSessionDelegate *ndefDelegate = nullptr;
    bool detectionRunning = false;
    bool isSessionScheduled = false;
    QTimer sessionTimer;
    QList<QPointer<QNearFieldTargetPrivateImpl>> detectedTargets;
    QNearFieldTarget::AccessMethod activeAccessMethod = QNearFieldTarget::UnknownAccess;

    bool scheduleSession(QNearFieldTarget::AccessMethod accessMethod);
    void startSession();
    bool startNdefSession();
    void stopSession(const QString &error);
    void stopNdefSession(const QString &error);
    void clearTargets();

private Q_SLOTS:
    void onTagDiscovered(void *target);
    void onTargetLost(QNearFieldTargetPrivateImpl *target);
    void onDidInvalidateWithError(bool doRestart);
    void onSessionTimer();
};


QT_END_NAMESPACE

#endif // QNEARFIELDMANAGER_IOS_P_H
