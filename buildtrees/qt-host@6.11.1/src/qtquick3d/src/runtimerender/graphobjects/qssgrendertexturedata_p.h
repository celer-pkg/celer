// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRENDERTEXTUREDATA_H
#define QSSGRENDERTEXTUREDATA_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimagetexture_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>
#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>
#include <QtCore/qsize.h>
#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderTextureData : public QSSGRenderGraphObject
{
public:
    explicit QSSGRenderTextureData();
    virtual ~QSSGRenderTextureData();

    const QByteArray &textureData() const;
    void setTextureData(const QByteArray &data);

    QSize size() const { return m_size; }
    void setSize(const QSize &size);

    int depth() const { return m_depth; }
    void setDepth(int depth);

    QSSGRenderTextureFormat format() const { return m_format; }
    void setFormat(QSSGRenderTextureFormat format);

    bool hasTransparency() const { return m_hasTransparency; }
    void setHasTransparency(bool hasTransparency);

    // We use a version number to track changes in the texture data.
    [[nodiscard]] quint32 version() const { return m_textureDataVersion; }

    QString debugObjectName;

protected:
    Q_DISABLE_COPY(QSSGRenderTextureData)

    // it's specially used for Type::Skin
    explicit QSSGRenderTextureData(QSSGRenderGraphObject::Type inType);

    QByteArray m_textureData;
    QSize m_size;
    int m_depth = 0;
    quint32 m_textureDataVersion = 0;
    QSSGRenderTextureFormat m_format = QSSGRenderTextureFormat::Unknown;
    bool m_hasTransparency = false;
};

// NOTE: We only hash the size, depth, format and hasTransparency here, not the actual data.
// This is because we want to be able to quickly check if a texture has changed, without needing
// to inspect the data content. If only the version changes we'll try to recycle the existing
// texture resource, if the size, depth, format or hasTransparency changes we'll need to create a
// new texture resource (See: QSSGBufferManager::loadTextureData).
inline size_t qHash(const QSSGRenderTextureData &data, size_t seed) noexcept
{
    const auto format = data.format();
    return qHashMulti(seed, data.size(), data.depth(), format.format, data.hasTransparency());
}

QT_END_NAMESPACE

#endif // QSSGRENDERTEXTUREDATA_H
