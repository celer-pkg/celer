// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDERER_UTIL_H
#define QSSG_RENDERER_UTIL_H

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

#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>

#include <QtCore/qhashfunctions.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

namespace QSSGRendererUtil
{
inline constexpr quint32 nextMultipleOf4(quint32 value) {
    return (value + 3) & ~3;
}
}

class QSSGRenderPath
{
public:
    QSSGRenderPath() = default;
    explicit inline QSSGRenderPath(const QString &p, const QString &lightmapKey = {}) noexcept
        : m_path(p), m_lightmapKey(lightmapKey),
          m_key{qHashMulti(QHashSeed::globalSeed(), p, lightmapKey)}
    {
    }

    inline bool isNull() const { return m_path.isNull(); }
    inline bool isEmpty() const { return m_path.isEmpty(); }
    QString path() const { return m_path; }
private:
    friend bool operator==(const QSSGRenderPath &, const QSSGRenderPath &);
    friend size_t qHash(const QSSGRenderPath &key, size_t seed = 0) noexcept
    {
        return key.m_key ? key.m_key : qHashMulti(seed, key.m_path, key.m_lightmapKey);
    }
    QString m_path;
    QString m_lightmapKey;
    size_t m_key = 0;
};

inline bool operator==(const QSSGRenderPath &p1, const QSSGRenderPath &p2)
{
    return (p1.m_key == p2.m_key) && (p1.m_path == p2.m_path) && (p1.m_lightmapKey == p2.m_lightmapKey);
}

QT_END_NAMESPACE

#endif
