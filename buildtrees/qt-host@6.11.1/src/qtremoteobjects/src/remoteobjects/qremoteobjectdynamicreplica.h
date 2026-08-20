// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDYNAMICREMOTEOBJECT_H
#define QDYNAMICREMOTEOBJECT_H

#include <QtRemoteObjects/qremoteobjectreplica.h>

QT_BEGIN_NAMESPACE

class Q_REMOTEOBJECTS_EXPORT QRemoteObjectDynamicReplica : public QRemoteObjectReplica
{
public:
    ~QRemoteObjectDynamicReplica() override;

    const QMetaObject *metaObject() const override;
    void *qt_metacast(const char *name) override;
    int qt_metacall(QMetaObject::Call call, int id, void **argv) override;

private:
    explicit QRemoteObjectDynamicReplica();
    explicit QRemoteObjectDynamicReplica(QRemoteObjectNode *node, const QString &name);
    friend class QRemoteObjectNodePrivate;
    friend class QRemoteObjectNode;
};

QT_END_NAMESPACE

#endif
