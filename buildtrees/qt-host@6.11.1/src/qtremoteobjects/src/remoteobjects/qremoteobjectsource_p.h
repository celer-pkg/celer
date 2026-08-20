// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:declarations-only

#ifndef QREMOTEOBJECTSOURCE_P_H
#define QREMOTEOBJECTSOURCE_P_H

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

#include <QtCore/qlist.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>
#include "qremoteobjectsource.h"
#include "qremoteobjectpacket_p.h"

QT_BEGIN_NAMESPACE

class QRemoteObjectSourceIo;
class QtROIoDeviceBase;

class QRemoteObjectSourceBase : public QObject
{
public:
    ~QRemoteObjectSourceBase() override;

    void setConnections();
    void resetObject(QObject *newObject);
    int qt_metacall(QMetaObject::Call call, int methodId, void **a) final;
    QObject *m_object, *m_adapter;
    const SourceApiMap *m_api;
    QVariantList m_marshalledArgs;
    bool hasAdapter() const { return m_adapter; }
    virtual QString name() const = 0;
    virtual bool isRoot() const = 0;

    QVariantList* marshalArgs(int index, void **a);
    void handleMetaCall(int index, QMetaObject::Call call, void **a);
    bool invoke(QMetaObject::Call c, int index, const QVariantList& args, QVariant* returnValue = nullptr);
    QByteArray m_objectChecksum;
    QMap<int, QPointer<QRemoteObjectSourceBase>> m_children;
    struct Private {
        Private(QRemoteObjectSourceIo *io, QRemoteObjectRootSource *root);
        QRemoteObjectSourceIo *m_sourceIo;
        QList<QtROIoDeviceBase*> m_listeners;
        // Pointer to codec, not owned by Private.  We can assume it is valid.
        QRemoteObjectPackets::CodecBase *codec;

        // Types needed during recursively sending a root to a new listener
        QSet<QString> sentTypes;
        bool isDynamic;
        QRemoteObjectRootSource *root;
    };
    Private *d;
    static const int qobjectPropertyOffset;
    static const int qobjectMethodOffset;
protected:
    explicit QRemoteObjectSourceBase(QObject *object, Private *d, const SourceApiMap *, QObject *adapter);
};

class QRemoteObjectSource : public QRemoteObjectSourceBase
{
public:
    explicit QRemoteObjectSource(QObject *object, Private *d, const SourceApiMap *, QObject *adapter, const QString &parentName);
    ~QRemoteObjectSource() override;

    bool isRoot() const override { return false; }
    QString name() const override { return m_name; }

    QString m_name;
};

class QRemoteObjectRootSource : public QRemoteObjectSourceBase
{
public:
    explicit QRemoteObjectRootSource(QObject *object, const SourceApiMap *,
                                     QObject *adapter, QRemoteObjectSourceIo *sourceIo);
    ~QRemoteObjectRootSource() override;

    bool isRoot() const override { return true; }
    QString name() const override { return m_name; }
    void addListener(QtROIoDeviceBase *io, bool dynamic = false);
    int removeListener(QtROIoDeviceBase *io, bool shouldSendRemove = false);

    QString m_name;
};

class DynamicApiMap final : public SourceApiMap
{
public:
    DynamicApiMap(QObject *object, const QMetaObject *metaObject, const QString &name, const QString &typeName);
    ~DynamicApiMap() override {}
    QString name() const override { return m_name; }
    QString typeName() const override { return m_typeName; }
    QByteArray className() const override { return QByteArray(m_metaObject->className()); }
    int enumCount() const override { return m_enumCount; }
    int propertyCount() const override { return m_properties.size(); }
    int signalCount() const override { return m_signals.size(); }
    int methodCount() const override { return m_methods.size(); }
    int sourceEnumIndex(int index) const override
    {
        if (index < 0 || index >= enumCount())
            return -1;
        return m_enumOffset + index;
    }
    int sourcePropertyIndex(int index) const override
    {
        if (index < 0 || index >= propertyCount())
            return -1;
        return m_properties.at(index);
    }
    int sourceSignalIndex(int index) const override
    {
        if (index < 0 || index >= signalCount())
            return -1;
        return m_signals.at(index);
    }
    int sourceMethodIndex(int index) const override
    {
        if (index < 0 || index >= methodCount())
            return -1;
        return m_methods.at(index);
    }
    int signalParameterCount(int index) const override { return parameterCount(m_signals.at(index)); }
    int signalParameterType(int sigIndex, int paramIndex) const override { return parameterType(m_signals.at(sigIndex), paramIndex); }
    const QByteArray signalSignature(int index) const override { return signature(m_signals.at(index)); }
    QByteArrayList signalParameterNames(int index) const override;

    int methodParameterCount(int index) const override { return parameterCount(m_methods.at(index)); }
    int methodParameterType(int methodIndex, int paramIndex) const override { return parameterType(m_methods.at(methodIndex), paramIndex); }
    const QByteArray methodSignature(int index) const override { return signature(m_methods.at(index)); }
    QMetaMethod::MethodType methodType(int index) const override;
    const QByteArray typeName(int index) const override;
    QByteArrayList methodParameterNames(int index) const override;

    int propertyIndexFromSignal(int index) const override
    {
        if (index >= 0 && index < m_propertyAssociatedWithSignal.size())
            return m_properties.at(m_propertyAssociatedWithSignal.at(index));
        return -1;
    }
    int propertyRawIndexFromSignal(int index) const override
    {
        if (index >= 0 && index < m_propertyAssociatedWithSignal.size())
            return m_propertyAssociatedWithSignal.at(index);
        return -1;
    }
    QByteArray objectSignature() const override { return m_objectSignature; }

    bool isDynamic() const override { return true; }

    int parameterCount(int objectIndex) const;
    int parameterType(int objectIndex, int paramIndex) const;
    const QByteArray signature(int objectIndex) const;
    inline void checkCache(int objectIndex) const
    {
        if (objectIndex != m_cachedMetamethodIndex) {
            m_cachedMetamethodIndex = objectIndex;
            m_cachedMetamethod = m_metaObject->method(objectIndex);
        }
    }

    QString m_name;
    QString m_typeName;
    int m_enumCount;
    int m_enumOffset;
    QList<int> m_properties;
    QList<int> m_signals;
    QList<int> m_methods;
    QList<int> m_propertyAssociatedWithSignal;
    const QMetaObject *m_metaObject;
    mutable QMetaMethod m_cachedMetamethod;
    mutable int m_cachedMetamethodIndex;
    QByteArray m_objectSignature;
};

QT_END_NAMESPACE

#endif
