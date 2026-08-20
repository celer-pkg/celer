// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef NFCTARGET_H
#define NFCTARGET_H

#include <QObject>

#include <QNdefMessage>
#include <QNearFieldTarget>
#include <QPointer>
#include <QQmlEngine>

class NfcTarget : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by NfcManager")

public:
    explicit NfcTarget(QNearFieldTarget *target, QObject *parent = nullptr);

    Q_INVOKABLE bool readNdefMessages();
    Q_INVOKABLE bool writeNdefMessage(const QNdefMessage &message);

Q_SIGNALS:
    void ndefMessageRead(const QNdefMessage &message);
    void requestCompleted();
    void error(QNearFieldTarget::Error error);

private:
    QPointer<QNearFieldTarget> m_target;
};

#endif // NFCTARGET_H
