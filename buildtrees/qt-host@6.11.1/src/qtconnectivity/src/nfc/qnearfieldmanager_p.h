// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNEARFIELDMANAGER_P_H
#define QNEARFIELDMANAGER_P_H

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

#include "qnearfieldmanager.h"
#include "qnearfieldtarget.h"
#include "qndefrecord.h"

#include "qtnfcglobal.h"

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QNdefFilter;

class Q_AUTOTEST_EXPORT QNearFieldManagerPrivate : public QObject
{
    Q_OBJECT

public:
    explicit QNearFieldManagerPrivate(QObject *parent = nullptr)
    :   QObject(parent)
    {
    }

    virtual ~QNearFieldManagerPrivate()
    {
    }

    virtual bool isEnabled() const
    {
        return false;
    }

    virtual bool isSupported(QNearFieldTarget::AccessMethod) const
    {
        return false;
    }

    virtual bool startTargetDetection(QNearFieldTarget::AccessMethod)
    {
        return false;
    }

    virtual void stopTargetDetection(const QString &)
    {
    }

    virtual void setUserInformation(const QString &)
    {
    }

signals:
    void adapterStateChanged(QNearFieldManager::AdapterState state);
    void targetDetectionStopped();
    void targetDetected(QNearFieldTarget *target);
    void targetLost(QNearFieldTarget *target);
};

QT_END_NAMESPACE

#endif // QNEARFIELDMANAGER_P_H
