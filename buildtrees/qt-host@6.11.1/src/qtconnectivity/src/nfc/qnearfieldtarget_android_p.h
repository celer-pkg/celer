// Copyright (C) 2016 Centria research and development
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNEARFIELDTARGET_ANDROID_P_H
#define QNEARFIELDTARGET_ANDROID_P_H

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

#include "android/androidjninfc_p.h"
#include "qnearfieldtarget_p.h"
#include "qndefmessage.h"
#include "qlist.h"
#include "qstringlist.h"
#include <QTimer>

#include <QtCore/QJniObject>
#include <QtCore/QJniEnvironment>

QT_BEGIN_NAMESPACE

class QNearFieldTargetPrivateImpl : public QNearFieldTargetPrivate
{
    Q_OBJECT
public:
    QNearFieldTargetPrivateImpl(QJniObject intent,
                                const QByteArray uid,
                                QObject *parent = nullptr);
    ~QNearFieldTargetPrivateImpl() override;

    QByteArray uid() const override;
    QNearFieldTarget::Type type() const override;
    QNearFieldTarget::AccessMethods accessMethods() const override;

    bool disconnect() override;

    bool hasNdefMessage() override;
    QNearFieldTarget::RequestId readNdefMessages() override;
    QNearFieldTarget::RequestId writeNdefMessages(const QList<QNdefMessage> &messages) override;

    int maxCommandLength() const override;
    QNearFieldTarget::RequestId sendCommand(const QByteArray &command) override;

    void setIntent(QJniObject intent);

signals:
    void targetDestroyed(const QByteArray &tagId);
    void targetLost(QNearFieldTargetPrivateImpl *target);
    void ndefMessageRead(const QNdefMessage &message, const QNearFieldTarget::RequestId &id);

protected slots:
    void checkIsTargetLost();

protected:
    void releaseIntent();
    void updateTechList();
    void updateType();
    QNearFieldTarget::Type getTagType() const;
    void setupTargetCheckTimer();
    void handleTargetLost();
    QJniObject getTagTechnology(const QString &tech) const;
    bool setTagTechnology(const QStringList &technologies);
    bool connect();
    bool setCommandTimeout(int timeout);

protected:
    QJniObject targetIntent;
    QByteArray targetUid;
    QTimer *targetCheckTimer;

    QString selectedTech;
    QStringList techList;
    QNearFieldTarget::Type tagType;
    QJniObject tagTech;
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_ANDROID_P_H
