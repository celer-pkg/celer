// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtremoteobjectglobal.h"
#include "qremoteobjectpacket_p.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(QT_REMOTEOBJECT, "qt.remoteobjects", QtWarningMsg)
Q_LOGGING_CATEGORY(QT_REMOTEOBJECT_MODELS, "qt.remoteobjects.models", QtWarningMsg)
Q_LOGGING_CATEGORY(QT_REMOTEOBJECT_IO, "qt.remoteobjects.io", QtWarningMsg)

QT_IMPL_METATYPE_EXTERN(QRemoteObjectSourceLocation)
QT_IMPL_METATYPE_EXTERN(QRemoteObjectSourceLocations)
QT_IMPL_METATYPE_EXTERN(QIntHash)

/*!
    \namespace QtRemoteObjects
    \inmodule QtRemoteObjects

    \brief The QtRemoteObjects namespace contains identifiers used in the
    Remote Objects module, as well as some functions used from code generated
    by the \l{Qt Remote Objects Compiler}{Replica Compiler (repc)}.
*/

/*!
    \enum QtRemoteObjects::InitialAction

    This enum type specifies the initial action when acquiring a \l Replica derived
    from QAbstractItemModel.

    \value FetchRootSize Only the size of the model is requested before the
                         \l {QRemoteObjectReplica::}{initialized} signal is emitted,
                         no data will be prefetched before that.
    \value PrefetchData  Some data can be prefetched before the
                         \l {QRemoteObjectReplica::}{initialized} signal is emitted.

    \sa QRemoteObjectNode::acquireModel(), QRemoteObjectReplica::initialized()
*/

namespace QtRemoteObjects {

void copyStoredProperties(const QMetaObject *mo, const void *src, void *dst)
{
    if (!src) {
        qCWarning(QT_REMOTEOBJECT) << Q_FUNC_INFO << ": trying to copy from a null source";
        return;
    }
    if (!dst) {
        qCWarning(QT_REMOTEOBJECT) << Q_FUNC_INFO << ": trying to copy to a null destination";
        return;
    }

    for (int i = 0, end = mo->propertyCount(); i != end; ++i) {
        const QMetaProperty mp = mo->property(i);
        mp.writeOnGadget(dst, mp.readOnGadget(src));
    }
}

void copyStoredProperties(const QMetaObject *mo, const void *src, QDataStream &dst)
{
    if (!src) {
        qCWarning(QT_REMOTEOBJECT) << Q_FUNC_INFO << ": trying to copy from a null source";
        return;
    }

    for (int i = 0, end = mo->propertyCount(); i != end; ++i) {
        const QMetaProperty mp = mo->property(i);
        dst << QRemoteObjectPackets::encodeVariant(mp.readOnGadget(src));
    }
}

void copyStoredProperties(const QMetaObject *mo, QDataStream &src, void *dst)
{
    if (!dst) {
        qCWarning(QT_REMOTEOBJECT) << Q_FUNC_INFO << ": trying to copy to a null destination";
        return;
    }

    for (int i = 0, end = mo->propertyCount(); i != end; ++i) {
        const QMetaProperty mp = mo->property(i);
        QVariant v;
        src >> v;
        mp.writeOnGadget(dst, QRemoteObjectPackets::decodeVariant(std::move(v), mp.metaType()));
    }
}

} // namespace QtRemoteObjects

QT_END_NAMESPACE
