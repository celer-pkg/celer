// Copyright (C) 2021 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:declarations-only

#ifndef QREMOTEOBJECTCONTAINERS_P_H
#define QREMOTEOBJECTCONTAINERS_P_H

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

#include <QtCore/qmetaassociation.h>
#include <QtCore/qmetasequence.h>
#include <QtCore/qvariant.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QtROSequentialContainer : public QVariantList
{
public:
    QtROSequentialContainer() = default;
    QtROSequentialContainer(const QMetaSequence::Iterable &lst)
    {
        m_valueType = lst.metaContainer().valueMetaType();
        reserve(lst.size());
        for (const auto v : lst)
            append(v);
    }
    void setValueType(const QByteArray &valueTypeName)
    {
        m_valueTypeName = valueTypeName;
        m_valueType = QMetaType::fromName(valueTypeName.constData());
        clear();
    }

    QMetaType m_valueType;
    QByteArray m_typeName, m_valueTypeName;
};

QDataStream &operator>>(QDataStream &ds, QtROSequentialContainer &p);

QDataStream &operator<<(QDataStream &ds, const QtROSequentialContainer &p);

class QtROAssociativeContainer : public QVariantMap
{
public:
    QtROAssociativeContainer() = default;
    QtROAssociativeContainer(const QMetaAssociation::Iterable &map)
    {
        m_keyType = map.metaContainer().keyMetaType();
        m_valueType = map.metaContainer().mappedMetaType();
        m_keys.reserve(map.size());
        QMetaAssociation::Iterable::const_iterator iter = map.begin();
        const QMetaAssociation::Iterable::const_iterator end = map.end();
        for ( ; iter != end; ++iter) {
            m_keys.append(iter.key());
            insert(iter.key().toString(), iter.value());
        }
    }
    void setTypes(const QByteArray &keyTypeName, const QByteArray &valueTypeName)
    {
        m_keyTypeName = keyTypeName;
        m_keyType = QMetaType::fromName(keyTypeName.constData());
        m_valueTypeName = valueTypeName;
        m_valueType = QMetaType::fromName(valueTypeName.constData());
        clear();
        m_keys.clear();
    }

    QMetaType m_keyType, m_valueType;
    QByteArray m_typeName, m_keyTypeName, m_valueTypeName;
    QVariantList m_keys; // Map uses QString for key, this doesn't lose information for proxy
};

QDataStream &operator>>(QDataStream &ds, QtROAssociativeContainer &p);

QDataStream &operator<<(QDataStream &ds, const QtROAssociativeContainer &p);

QT_END_NAMESPACE

#endif // QREMOTEOBJECTCONTAINERS_P_H
