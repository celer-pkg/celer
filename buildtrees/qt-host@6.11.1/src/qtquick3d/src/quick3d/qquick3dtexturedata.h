// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DTEXTUREDATA_H
#define QQUICK3DTEXTUREDATA_H

#include <QtQuick3D/qquick3dobject.h>

QT_BEGIN_NAMESPACE

class QQuick3DTextureDataPrivate;

class Q_QUICK3D_EXPORT QQuick3DTextureData : public QQuick3DObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QQuick3DTextureData)

    QML_NAMED_ELEMENT(TextureData)
    QML_UNCREATABLE("TextureData is Abstract")
    QML_ADDED_IN_VERSION(6, 0)
public:
    enum Format {
        None,
        RGBA8,
        RGBA16F,
        RGBA32F,
        RGBE8,
        R8,
        R16,
        R16F,
        R32F,
        BC1,
        BC2,
        BC3,
        BC4,
        BC5,
        BC6H,
        BC7,
        DXT1_RGBA,
        DXT1_RGB,
        DXT3_RGBA,
        DXT5_RGBA,
        ETC2_RGB8,
        ETC2_RGB8A1,
        ETC2_RGBA8,
        ASTC_4x4,
        ASTC_5x4,
        ASTC_5x5,
        ASTC_6x5,
        ASTC_6x6,
        ASTC_8x5,
        ASTC_8x6,
        ASTC_8x8,
        ASTC_10x5,
        ASTC_10x6,
        ASTC_10x8,
        ASTC_10x10,
        ASTC_12x10,
        ASTC_12x12,
    };
    Q_ENUM(Format)

    QQuick3DTextureData(QQuick3DObject *parent = nullptr);
    ~QQuick3DTextureData();

    const QByteArray textureData() const;
    void setTextureData(const QByteArray &data);

    QSize size() const;
    void setSize(const QSize &size);

    int depth() const;
    void setDepth(int depth);

    Format format() const;
    void setFormat(Format format);

    bool hasTransparency() const;
    void setHasTransparency(bool hasTransparency);

Q_SIGNALS:
    void textureDataNodeDirty();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void markAllDirty() override;
};

QT_END_NAMESPACE

#endif // QQUICK3DTEXTUREDATA_H
