// Copyright (C) 2016 BlackBerry Limited, Copyright (C) 2016 BasysKom GmbH
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNEARFIELDMANAGER_NEARD_H
#define QNEARFIELDMANAGER_NEARD_H

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
#include "qnearfieldmanager.h"
#include "qnearfieldtarget.h"
#include "neard/neard_helper_p.h"

#include <QDBusObjectPath>
#include <QDBusVariant>
#include <QMap>

QT_BEGIN_NAMESPACE

class QNearFieldManagerPrivateImpl : public QNearFieldManagerPrivate
{
    Q_OBJECT

public:
    QNearFieldManagerPrivateImpl();
    ~QNearFieldManagerPrivateImpl() override;

    bool isEnabled() const override;

    bool isSupported(QNearFieldTarget::AccessMethod) const override;

    bool startTargetDetection(QNearFieldTarget::AccessMethod) override;

    void stopTargetDetection(const QString& = QString()) override;

private Q_SLOTS:
    void handleTagFound(const QDBusObjectPath&);
    void handleTagRemoved(const QDBusObjectPath&);

private:
    QString m_adapterPath;
    QMap<QString, QNearFieldTarget*> m_activeTags;
    NeardHelper *m_neardHelper;
};

QT_END_NAMESPACE


#endif // QNEARFIELDMANAGER_NEARD_H
