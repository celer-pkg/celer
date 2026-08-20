// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qremoteobjectdynamicreplica.h"
#include "qremoteobjectreplica_p.h"

#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

/*!
    \class QRemoteObjectDynamicReplica
    \inmodule QtRemoteObjects
    \brief A dynamically instantiated \l {Replica}.

    There are generated replicas (replicas having the header files produced by
    the \l {repc} {Replica Compiler}), and dynamic replicas, that are generated
    on-the-fly. This is the class for the dynamic type of replica.

    When the connection to the \l {Source} object is made, the initialization
    step passes the current property values (see \l {Replica Initialization}).
    In a DynamicReplica, the property/signal/slot details are also sent,
    allowing the replica object to be created on-the-fly. This can be convenient
    in QML or scripting, but has two primary disadvantages. First, the object is
    in effect "empty" until it is successfully initialized by the \l {Source}.
    Second, in C++, calls must be made using QMetaObject::invokeMethod(), as the
    moc generated lookup will not be available.

    This class does not have a public constructor. It can only be instantiated
    by using the dynamic QRemoteObjectNode::acquire method.
*/

QRemoteObjectDynamicReplica::QRemoteObjectDynamicReplica()
    : QRemoteObjectReplica()
{
}

QRemoteObjectDynamicReplica::QRemoteObjectDynamicReplica(QRemoteObjectNode *node, const QString &name)
    : QRemoteObjectReplica(ConstructWithNode)
{
    initializeNode(node, name);
}

/*!
    Destroys the dynamic replica.

    \sa {Replica Ownership}
*/
QRemoteObjectDynamicReplica::~QRemoteObjectDynamicReplica()
{
}

/*!
    \internal
    Returns a pointer to the dynamically generated meta-object of this object, or
    QRemoteObjectDynamicReplica's metaObject if the object is not initialized.  This
    function overrides the QObject::metaObject() virtual function to provide the same
    functionality for dynamic replicas.

    \sa QObject::metaObject(), {Replica Initialization}
*/
const QMetaObject* QRemoteObjectDynamicReplica::metaObject() const
{
    auto impl = qSharedPointerCast<QRemoteObjectReplicaImplementation>(d_impl);
    // Returning nullptr will likely result in a crash if this type is used before the
    // definition is received.  Note: QRemoteObjectDynamicReplica doesn't include the
    // QObject macro, so it's metaobject would resolve to QRemoteObjectReplica::metaObject()
    // if we weren't overriding it.
    if (!impl->m_metaObject) {
        qWarning() << "Dynamic metaobject is not assigned, returning generic Replica metaObject.";
        qWarning() << "This may cause issues if used for more than checking the Replica state.";
        return QRemoteObjectReplica::metaObject();
    }

    return impl->m_metaObject;
}

/*!
    \internal
    This function overrides the QObject::qt_metacast() virtual function to provide the same functionality for dynamic replicas.

    \sa QObject::qt_metacast()
*/
void *QRemoteObjectDynamicReplica::qt_metacast(const char *name)
{
    if (!name)
        return nullptr;

    if (!strcmp(name, "QRemoteObjectDynamicReplica"))
        return static_cast<void*>(const_cast<QRemoteObjectDynamicReplica*>(this));

    // not entirely sure that one is needed... TODO: check
    auto impl = qSharedPointerCast<QRemoteObjectReplicaImplementation>(d_impl);
    if (QString::fromLatin1(name) == impl->m_objectName)
        return static_cast<void*>(const_cast<QRemoteObjectDynamicReplica*>(this));

    return QObject::qt_metacast(name);
}

/*!
    \internal
    This function overrides the QObject::qt_metacall() virtual function to provide the same functionality for dynamic replicas.

    \sa QObject::qt_metacall()
*/
int QRemoteObjectDynamicReplica::qt_metacall(QMetaObject::Call call, int id, void **argv)
{
    static const bool debugArgs = qEnvironmentVariableIsSet("QT_REMOTEOBJECT_DEBUG_ARGUMENTS");

    auto impl = qSharedPointerCast<QConnectedReplicaImplementation>(d_impl);

    int saved_id = id;
    id = QRemoteObjectReplica::qt_metacall(call, id, argv);
    if (id < 0 || impl->m_metaObject == nullptr)
        return id;

    if (call == QMetaObject::ReadProperty || call == QMetaObject::WriteProperty) {
        QMetaProperty mp = metaObject()->property(saved_id);

        if (call == QMetaObject::WriteProperty) {
            QVariantList args;
            if (mp.userType() == QMetaType::QVariant)
                args << *reinterpret_cast<QVariant*>(argv[0]);
            else
                args << QVariant(mp.metaType(), argv[0]);
            QRemoteObjectReplica::send(QMetaObject::WriteProperty, saved_id, args);
        } else {
            if (mp.userType() == QMetaType::QVariant)
                *reinterpret_cast<QVariant*>(argv[0]) = impl->m_propertyStorage[id];
            else {
                const QVariant value = propAsVariant(id);
                mp.metaType().destruct(argv[0]);
                mp.metaType().construct(argv[0], value.data());
            }
        }

        id = -1;
    } else if (call == QMetaObject::InvokeMetaMethod) {
        if (id < impl->m_numSignals) {
            qCDebug(QT_REMOTEOBJECT) << "DynamicReplica Activate" << impl->m_metaObject->method(saved_id).methodSignature();
            // signal relay from Source world to Replica
            QMetaObject::activate(this, impl->m_metaObject, id, argv);

        } else {
            // method relay from Replica to Source
            const QMetaMethod mm = impl->m_metaObject->method(saved_id);
            const int nParam = mm.parameterCount();
            QVariantList args;
            args.reserve(nParam);
            for (int i = 0; i < nParam; ++i) {
                const auto metaType = mm.parameterMetaType(i);
                if (metaType.flags().testFlag(QMetaType::IsEnumeration)) {
                    auto transferType = QRemoteObjectPackets::transferTypeForEnum(metaType);
                    args.push_back(QVariant(transferType, argv[i + 1]));
                } else
                    args.push_back(QVariant(metaType, argv[i + 1]));
            }

            if (debugArgs) {
                qCDebug(QT_REMOTEOBJECT) << "method" << mm.methodSignature() << "invoked - args:" << args;
            } else {
                qCDebug(QT_REMOTEOBJECT) << "method" << mm.methodSignature() << "invoked";
            }

            if (mm.returnType() == QMetaType::Void)
                QRemoteObjectReplica::send(QMetaObject::InvokeMetaMethod, saved_id, args);
            else {
                QRemoteObjectPendingCall call = QRemoteObjectReplica::sendWithReply(QMetaObject::InvokeMetaMethod, saved_id, args);
                if (argv[0])
                    *(static_cast<QRemoteObjectPendingCall*>(argv[0])) = call;
            }
        }

        id = -1;
    }

    return id;
}

QT_END_NAMESPACE
