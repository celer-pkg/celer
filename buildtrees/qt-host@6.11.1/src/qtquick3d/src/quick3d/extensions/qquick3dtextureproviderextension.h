// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DTEXTUREPROVIDEREXTENSION_H
#define QQUICK3DTEXTUREPROVIDEREXTENSION_H

#include <QtQuick3D/QQuick3DRenderExtension>
#include <QtQml/QQmlEngine>

QT_BEGIN_NAMESPACE

class QQuick3DTextureProviderExtensionPrivate;

class Q_QUICK3D_EXPORT QQuick3DTextureProviderExtension : public QQuick3DRenderExtension
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QQuick3DTextureProviderExtension)

    Q_PROPERTY(SamplerHint samplerHint READ samplerHint WRITE setSamplerHint NOTIFY samplerHintChanged FINAL)
    QML_NAMED_ELEMENT(TextureProviderExtension)
    QML_UNCREATABLE("TextureProviderExtension is an abstract type")
    QML_ADDED_IN_VERSION(6, 11)
public:
    // Sampler Types
    enum class SamplerHint {
        Sampler2D,
        Sampler2DArray,
        Sampler3D,
        SamplerCube,
        SamplerCubeArray,
        SamplerBuffer,
    };
    Q_ENUM(SamplerHint)

    explicit QQuick3DTextureProviderExtension(QQuick3DObject *parent = nullptr);
    ~QQuick3DTextureProviderExtension() override;

    SamplerHint samplerHint() const;
    void setSamplerHint(SamplerHint newSamplerHint);

Q_SIGNALS:
    void samplerHintChanged();

protected:
    bool event(QEvent *event) override;
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
};

QT_END_NAMESPACE

#endif // QQUICK3DTEXTUREPROVIDEREXTENSION_H

