// Copyright (C) 2016 Centria research and development
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNEARFIELDMANAGER_ANDROID_P_H
#define QNEARFIELDMANAGER_ANDROID_P_H

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

#include "qnearfieldmanager_p.h"
#include "qnearfieldtarget_android_p.h"
#include "android/androidjninfc_p.h"
#include "android/androidmainnewintentlistener_p.h"

#include <QHash>
#include <QtCore/QJniObject>

QT_BEGIN_NAMESPACE

class QByteArray;
class QNearFieldManagerPrivateImpl : public QNearFieldManagerPrivate,
                                     public QAndroidNfcListenerInterface
{
    Q_OBJECT

public:
    QNearFieldManagerPrivateImpl();
    ~QNearFieldManagerPrivateImpl() override;

    bool isEnabled() const override;
    bool isSupported(QNearFieldTarget::AccessMethod accessMethod) const override;
    bool startTargetDetection(QNearFieldTarget::AccessMethod accessMethod) override;
    void stopTargetDetection(const QString &errorMessage) override;
    void newIntent(QJniObject intent) override;
    QByteArray getUid(const QJniObject &intent);

private:
    bool detecting;
    QNearFieldTarget::AccessMethod requestedMethod;
    QHash<QByteArray, QNearFieldTargetPrivateImpl*> detectedTargets;

    QJniObject broadcastReceiver;

private slots:
    void onTargetDiscovered(QJniObject intent);
    void onTargetDestroyed(const QByteArray &uid);
    void onTargetDetected(QNearFieldTargetPrivateImpl *target);
    void onTargetLost(QNearFieldTargetPrivateImpl *target);
};

QT_END_NAMESPACE

#endif // QNEARFIELDMANAGER_ANDROID_P_H
