// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qv4resolvedtypereference_p.h"

#include <QtQml/private/qqmlengine_p.h>
#include <QtQml/qqmlpropertymap.h>
#include <QtCore/qcryptographichash.h>

QT_BEGIN_NAMESPACE

namespace QV4 {

template <typename T>
bool qtTypeInherits(const QMetaObject *mo) {
    while (mo) {
        if (mo == &T::staticMetaObject)
            return true;
        mo = mo->superClass();
    }
    return false;
}

void ResolvedTypeReference::doDynamicTypeCheck()
{
    const QMetaObject *mo = nullptr;
    if (m_typePropertyCache)
        mo = m_typePropertyCache->firstCppMetaObject();
    else if (m_type.isValid())
        mo = m_type.metaObject();
    m_isFullyDynamicType = qtTypeInherits<QQmlPropertyMap>(mo);
}

/*!
    \internal
    Returns the property cache, creating one if it doesn't already exist. The
    cache is not referenced.
*/
QQmlPropertyCache::ConstPtr ResolvedTypeReference::createPropertyCache()
{
    if (!m_typePropertyCache && m_type.isValid()) {
        const QMetaObject *metaObject = m_type.metaObject();
        if (!metaObject) // value type of non-Q_GADGET base with extension
            metaObject = m_type.extensionMetaObject();
        if (metaObject)
            m_typePropertyCache = QQmlMetaType::propertyCache(metaObject, m_version);
    }

    return m_typePropertyCache;
}

bool ResolvedTypeReference::addToHash(
        QCryptographicHash *hash, QHash<quintptr, QByteArray> *checksums)
{
    // No need to add types from the same document to the hash.
    if (m_isSelfReference)
        return true;

    if (m_compilationUnit) {
        hash->addData({m_compilationUnit->unitData()->md5Checksum,
                        sizeof(m_compilationUnit->unitData()->md5Checksum)});
        return true;
    }

    if (QQmlPropertyCache::ConstPtr propertyCache = createPropertyCache()) {
        bool ok = false;
        hash->addData(propertyCache->checksum(checksums, &ok));
        return ok;
    }

    return false;
}

} // namespace QV4

QT_END_NAMESPACE
