// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QREGISTRYSOURCE_P_H
#define QREGISTRYSOURCE_P_H

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

#include "qtremoteobjectglobal.h"
#include "private/qglobal_p.h"

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QRegistrySource : public QObject
{
    Q_OBJECT
    Q_CLASSINFO(QCLASSINFO_REMOTEOBJECT_TYPE, "Registry")

    Q_PROPERTY(QRemoteObjectSourceLocations sourceLocations READ sourceLocations)

public:
    explicit QRegistrySource(QObject *parent = nullptr);
    ~QRegistrySource() override;

    QRemoteObjectSourceLocations sourceLocations() const;

Q_SIGNALS:
    void remoteObjectAdded(const QRemoteObjectSourceLocation &entry);
    void remoteObjectRemoved(const QRemoteObjectSourceLocation &entry);

public Q_SLOTS:
    void addSource(const QRemoteObjectSourceLocation &entry);
    void removeSource(const QRemoteObjectSourceLocation &entry);
    void removeServer(const QUrl &url);

private:
    QRemoteObjectSourceLocations m_sourceLocations;
};

QT_END_NAMESPACE

#endif
