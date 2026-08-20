// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGSHADERMAPKEY_P_H
#define QSSGSHADERMAPKEY_P_H

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

#include <QtQuick3DRuntimeRender/private/qtquick3druntimerenderglobal_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderkeys_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderparticleshaderkeys_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>

QT_BEGIN_NAMESPACE

template <typename T>
struct TShaderMapKey
{
    QByteArray m_name;
    const QSSGShaderFeatures m_featuresOrig;
    const T *m_materialKeyOrig;
    T m_materialKeyCopy;
    size_t m_hashCode;

    void detach()
    {
        if (m_materialKeyOrig) {
            m_materialKeyCopy = *m_materialKeyOrig;
            m_materialKeyOrig = nullptr;
        }
    }

    TShaderMapKey(const QByteArray &inName,
                     const QSSGShaderFeatures &inFeatures,
                     const T &inMaterialKey)
        : m_name(inName), m_featuresOrig(inFeatures), m_materialKeyOrig(&inMaterialKey)
    {
        m_hashCode = qHash(m_name) ^ qHash(m_featuresOrig) ^ qHash(m_materialKeyOrig->hash());
    }
};
template <typename T>
inline bool operator==(const TShaderMapKey<T> &a, const TShaderMapKey<T> &b) Q_DECL_NOTHROW
{
    if (a.m_name != b.m_name)
        return false;

    const T *keyA = a.m_materialKeyOrig ? a.m_materialKeyOrig : &a.m_materialKeyCopy;
    const T *keyB = b.m_materialKeyOrig ? b.m_materialKeyOrig : &b.m_materialKeyCopy;
    if (!(*keyA == *keyB))
        return false;

    return (a.m_featuresOrig == b.m_featuresOrig);
}

typedef TShaderMapKey<QSSGShaderDefaultMaterialKey> QSSGShaderMapKey;
typedef TShaderMapKey<QSSGShaderParticleMaterialKey> QSSGParticleShaderMapKey;

inline size_t qHash(const QSSGShaderMapKey &key, size_t seed)
{
    return key.m_hashCode ^ seed;
}

inline size_t qHash(const QSSGParticleShaderMapKey &key, size_t seed)
{
    return key.m_hashCode ^ seed;
}

QT_END_NAMESPACE

#endif
