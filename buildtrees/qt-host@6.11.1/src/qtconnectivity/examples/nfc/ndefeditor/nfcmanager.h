// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef NFCMANAGER_H
#define NFCMANAGER_H

#include <QObject>

#include <QQmlEngine>

#include "nfctarget.h"

QT_FORWARD_DECLARE_CLASS(QNearFieldManager);

class NfcManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit NfcManager(QObject *parent = nullptr);

    Q_INVOKABLE void startTargetDetection();
    Q_INVOKABLE void stopTargetDetection();

Q_SIGNALS:
    void targetDetected(NfcTarget *target);

private:
    QNearFieldManager *m_manager;
};

#endif // NFCMANAGER_H
