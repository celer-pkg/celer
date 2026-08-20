// Copyright (C) 2021 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include <QtCore/qiodevice.h>

#include "qremoteobjectcontainers_p.h"
#include "qremoteobjectpacket_p.h"

QT_BEGIN_NAMESPACE

QDataStream &operator>>(QDataStream &ds, QtROSequentialContainer &p)
{
    QByteArray typeName;
    quint32 count;
    ds >> typeName;
    p.setValueType(typeName);
    ds >> count;
    p.reserve(count);
    QVariant value{p.m_valueType, nullptr};
    for (quint32 i = 0; i < count; i++) {
        if (!p.m_valueType.load(ds, value.data())) {
            qWarning("QSQ_: unable to load type '%s', returning an empty list.", p.m_valueTypeName.constData());
            p.clear();
            break;
        }
        p.append(value);
    }
    return ds;
}

QDataStream &operator<<(QDataStream &ds, const QtROSequentialContainer &p)
{
    ds << p.m_valueTypeName;
    auto pos = ds.device()->pos();
    quint32 count = p.size();
    ds << count;
    for (quint32 i = 0; i < count; i++) {
        if (!p.m_valueType.save(ds, p.at(i).data())) {
            ds.device()->seek(pos);
            ds.resetStatus();
            ds << quint32(0);
            qWarning("QSQ_: unable to save type '%s'.", p.m_valueTypeName.constData());
            break;
        }
    }
    return ds;
}

const char *descopedName(QMetaType type) {
    auto name = QByteArray::fromRawData(type.name(), qstrlen(type.name()));
    int index = name.lastIndexOf(':'); // Returns -1 if not found
    return type.name() + index + 1;
}

QDataStream &operator>>(QDataStream &ds, QtROAssociativeContainer &p)
{
    QByteArray keyTypeName, valueTypeName;
    quint32 count;
    ds >> keyTypeName;
    ds >> valueTypeName;
    p.setTypes(keyTypeName, valueTypeName);
    ds >> count;
    p.m_keys.reserve(count);
    auto transferType = p.m_keyType;
    if (p.m_keyType.flags().testFlag(QMetaType::IsEnumeration))
        transferType = QRemoteObjectPackets::transferTypeForEnum(p.m_keyType);
    QVariant key{transferType, nullptr};
    QVariant value{p.m_valueType, nullptr};
    for (quint32 i = 0; i < count; i++) {
        if (!transferType.load(ds, key.data())) {
            qWarning("QAS_: unable to load key '%s', returning an empty map.", p.m_keyTypeName.constData());
            p.clear();
            break;
        }
        if (!p.m_valueType.load(ds, value.data())) {
            qWarning("QAS_: unable to load value '%s', returning an empty map.", p.m_valueTypeName.constData());
            p.clear();
            break;
        }
        if (transferType != p.m_keyType) {
            bool isFlag = false;
            QVariant enumKey(key);
            enumKey.convert(p.m_keyType);
            p.m_keys.append(enumKey);
            if (auto meta = p.m_keyType.metaObject()) {
                int index = meta->indexOfEnumerator(descopedName(p.m_keyType));
                isFlag = meta->enumerator(index).isFlag();
            }
            // If multiple flag values are set, toString() returns an empty string
            // Thus, for flags, we convert the integer value to a string
            if (isFlag)
                p.insert(key.toString(), value);
            else
                p.insert(enumKey.toString(), value);
        } else {
            p.insert(key.toString(), value);
            p.m_keys.append(key);
        }
    }
    return ds;
}

QDataStream &operator<<(QDataStream &ds, const QtROAssociativeContainer &p)
{
    ds << p.m_keyTypeName;
    ds << p.m_valueTypeName;
    auto pos = ds.device()->pos();
    quint32 count = p.size();
    ds << count;
    QMetaAssociation::Iterable map(&p);
    QMetaAssociation::Iterable::const_iterator iter = map.begin();
    auto transferType = p.m_keyType;
    if (p.m_keyType.flags().testFlag(QMetaType::IsEnumeration))
        transferType = QRemoteObjectPackets::transferTypeForEnum(p.m_keyType);
    bool keySaved;
    for (quint32 i = 0; i < count; i++) {
        if (transferType != p.m_keyType) {
            QVariant intKey(iter.key());
            intKey.convert(transferType);
            keySaved = transferType.save(ds, intKey.data());
        } else {
            keySaved = transferType.save(ds, iter.key().data());
        }
        if (!keySaved) {
            ds.device()->seek(pos);
            ds.resetStatus();
            ds << quint32(0);
            qWarning("QAS_: unable to save type '%s'.", p.m_valueTypeName.constData());
            break;
        }
        if (!p.m_valueType.save(ds, iter.value().data())) {
            ds.device()->seek(pos);
            ds.resetStatus();
            ds << quint32(0);
            qWarning("QAS_: unable to save type '%s'.", p.m_valueTypeName.constData());
            break;
        }
        iter++;
    }
    return ds;
}

QT_END_NAMESPACE
