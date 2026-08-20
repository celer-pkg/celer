// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNEARFIELDMANAGER_H
#define QNEARFIELDMANAGER_H

#include <QtCore/QObject>
#include <QtNfc/qtnfcglobal.h>
#include <QtNfc/QNearFieldTarget>
#include <QtNfc/QNdefRecord>
#include <QtNfc/QNdefFilter>

QT_BEGIN_NAMESPACE

class QNearFieldManagerPrivate;
class Q_NFC_EXPORT QNearFieldManager : public QObject
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(QNearFieldManager)

public:
    enum class AdapterState {
        Offline = 1,
        TurningOn = 2,
        Online = 3,
        TurningOff = 4
    };
    Q_ENUM(AdapterState)

    explicit QNearFieldManager(QObject *parent = nullptr);
    explicit QNearFieldManager(QNearFieldManagerPrivate *backend, QObject *parent = nullptr);
    ~QNearFieldManager();

    bool isEnabled() const;
    bool isSupported(QNearFieldTarget::AccessMethod accessMethod
                     = QNearFieldTarget::AnyAccess) const;

    bool startTargetDetection(QNearFieldTarget::AccessMethod accessMethod);
    void stopTargetDetection(const QString &errorMessage = QString());

    void setUserInformation(const QString &message);

Q_SIGNALS:
    void adapterStateChanged(QNearFieldManager::AdapterState state);
    void targetDetectionStopped();
    void targetDetected(QNearFieldTarget *target);
    void targetLost(QNearFieldTarget *target);

private:
    QNearFieldManagerPrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QNEARFIELDMANAGER_H
