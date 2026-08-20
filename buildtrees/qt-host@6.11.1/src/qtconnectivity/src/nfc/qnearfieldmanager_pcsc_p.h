// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNEARFIELDMANAGER_PCSC_P_H
#define QNEARFIELDMANAGER_PCSC_P_H

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

QT_BEGIN_NAMESPACE

class QPcscManager;
class QPcscCard;
class QThread;

class QNearFieldManagerPrivateImpl : public QNearFieldManagerPrivate
{
    Q_OBJECT
public:
    QNearFieldManagerPrivateImpl();
    ~QNearFieldManagerPrivateImpl() override;

    bool isEnabled() const override;
    bool isSupported(QNearFieldTarget::AccessMethod accessMethod) const override;

    bool startTargetDetection(QNearFieldTarget::AccessMethod accessMethod) override;
    void stopTargetDetection(const QString &errorMessage) override;

public Q_SLOTS:
    void onCardInserted(QPcscCard *card, const QByteArray &uid,
                        QNearFieldTarget::AccessMethods accessMethods, int maxInputLength);
    void onTargetLost(QNearFieldTargetPrivate *target);

Q_SIGNALS:
    void startTargetDetectionRequest(QNearFieldTarget::AccessMethod accessMethod);
    void stopTargetDetectionRequest();

private:
    QThread *m_workerThread;
    QPcscManager *m_worker;
};

QT_END_NAMESPACE

#endif // QNEARFIELDMANAGER_PCSC_P_H
