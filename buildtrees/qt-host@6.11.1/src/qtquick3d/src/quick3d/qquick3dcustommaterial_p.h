// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGCUSTOMMATERIAL_H
#define QSSGCUSTOMMATERIAL_H

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

#include <QtQuick3D/private/qquick3dmaterial_p.h>
#include <QtCore/qurl.h>

#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>
#include <QtQuick3D/private/qquick3dshaderutils_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DCustomMaterial : public QQuick3DMaterial
{
    Q_OBJECT
    Q_PROPERTY(ShadingMode shadingMode READ shadingMode WRITE setShadingMode NOTIFY shadingModeChanged)
    Q_PROPERTY(QUrl fragmentShader READ fragmentShader WRITE setFragmentShader NOTIFY fragmentShaderChanged)
    Q_PROPERTY(QUrl vertexShader READ vertexShader WRITE setVertexShader NOTIFY vertexShaderChanged)
    Q_PROPERTY(QString __fragmentShaderCode READ fragmentShaderCode WRITE setFragmentShaderCode NOTIFY fragmentShaderCodeChanged REVISION(6, 8))
    Q_PROPERTY(QString __vertexShaderCode READ vertexShaderCode WRITE setVertexShaderCode NOTIFY vertexShaderCodeChanged REVISION(6, 8))
    Q_PROPERTY(BlendMode sourceBlend READ srcBlend WRITE setSrcBlend NOTIFY srcBlendChanged)
    Q_PROPERTY(BlendMode destinationBlend READ dstBlend WRITE setDstBlend NOTIFY dstBlendChanged)
    Q_PROPERTY(BlendMode sourceAlphaBlend READ srcAlphaBlend WRITE setSrcAlphaBlend NOTIFY srcAlphaBlendChanged REVISION(6, 7))
    Q_PROPERTY(BlendMode destinationAlphaBlend READ dstAlphaBlend WRITE setDstAlphaBlend NOTIFY dstAlphaBlendChanged REVISION(6, 7))
    Q_PROPERTY(bool alwaysDirty READ alwaysDirty WRITE setAlwaysDirty NOTIFY alwaysDirtyChanged)
    Q_PROPERTY(float lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)

    QML_NAMED_ELEMENT(CustomMaterial)

public:
    enum class ShadingMode
    {
        Unshaded,
        Shaded
    };
    Q_ENUM(ShadingMode)

    enum class BlendMode
    {
        NoBlend,
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        SrcAlphaSaturate
    };
    Q_ENUM(BlendMode)

    explicit QQuick3DCustomMaterial(QQuick3DObject *parent = nullptr);
    ~QQuick3DCustomMaterial() override;

    ShadingMode shadingMode() const;
    QUrl vertexShader() const;
    QUrl fragmentShader() const;
    Q_REVISION(6, 8) QString vertexShaderCode() const;
    Q_REVISION(6, 8) QString fragmentShaderCode() const;
    BlendMode srcBlend() const;
    BlendMode dstBlend() const;
    Q_REVISION(6, 7) BlendMode srcAlphaBlend() const;
    Q_REVISION(6, 7) BlendMode dstAlphaBlend() const;
    bool alwaysDirty() const;
    float lineWidth() const;

public Q_SLOTS:
    void setShadingMode(QQuick3DCustomMaterial::ShadingMode mode);
    void setVertexShader(const QUrl &url);
    void setFragmentShader(const QUrl &url);
    Q_REVISION(6, 8) void setVertexShaderCode(const QString &code);
    Q_REVISION(6, 8) void setFragmentShaderCode(const QString &code);
    void setSrcBlend(QQuick3DCustomMaterial::BlendMode mode);
    void setDstBlend(QQuick3DCustomMaterial::BlendMode mode);
    Q_REVISION(6, 7) void setSrcAlphaBlend(QQuick3DCustomMaterial::BlendMode mode);
    Q_REVISION(6, 7) void setDstAlphaBlend(QQuick3DCustomMaterial::BlendMode mode);
    void setAlwaysDirty(bool alwaysDirty);
    void setLineWidth(float width);

Q_SIGNALS:
    void shadingModeChanged();
    void vertexShaderChanged();
    void fragmentShaderChanged();
    Q_REVISION(6, 8) void vertexShaderCodeChanged();
    Q_REVISION(6, 8) void fragmentShaderCodeChanged();
    void srcBlendChanged();
    void dstBlendChanged();
    Q_REVISION(6, 7) void srcAlphaBlendChanged();
    Q_REVISION(6, 7) void dstAlphaBlendChanged();
    void alwaysDirtyChanged();
    void lineWidthChanged();

protected:
    enum Dirty : quint32 {
        TextureDirty = 0x1,
        PropertyDirty = 0x2,
        ShaderSettingsDirty = 0x4,
        DynamicPropertiesDirty = 0x8, // Special case for custom materials created manually
        AllDirty = std::numeric_limits<quint32>::max() ^ DynamicPropertiesDirty // (DynamicPropertiesDirty is intentionally excluded from AllDirty)
    };

    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void itemChange(ItemChange, const ItemChangeData &) override;
    void markAllDirty() override;
    static void markDirty(QQuick3DCustomMaterial &that, QQuick3DCustomMaterial::Dirty type);

private Q_SLOTS:
    void onPropertyDirty();
    void onTextureDirty();

private:
    friend class QQuick3DShaderUtilsTextureInput;
    friend class QQuick3DViewport;

    void setDynamicTextureMap(QQuick3DShaderUtilsTextureInput *textureMap);

    QSet<QQuick3DShaderUtilsTextureInput *> m_dynamicTextureMaps;
    quint32 m_dirtyAttributes = Dirty::AllDirty;
    BlendMode m_srcBlend = BlendMode::NoBlend;
    BlendMode m_dstBlend = BlendMode::NoBlend;
    BlendMode m_srcAlphaBlend = BlendMode::NoBlend;
    BlendMode m_dstAlphaBlend = BlendMode::NoBlend;
    ShadingMode m_shadingMode = ShadingMode::Shaded;
    QUrl m_vertexShader;
    QUrl m_fragmentShader;
    QString m_vertexShaderCode;
    QString m_fragmentShaderCode;
    bool m_alwaysDirty = false;
    float m_lineWidth = 1.0f;
};

QT_END_NAMESPACE

#endif // QSSGCUSTOMMATERIAL_H
