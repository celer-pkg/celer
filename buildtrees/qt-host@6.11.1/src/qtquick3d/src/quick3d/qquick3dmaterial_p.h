// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGMATERIAL_H
#define QSSGMATERIAL_H

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

#include <QtQuick3D/qquick3dobject.h>
#include <QtQuick3D/private/qquick3dtexture_p.h>

#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

class QQuick3DSceneManager;
class Q_QUICK3D_EXPORT QQuick3DMaterial : public QQuick3DObject
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DTexture *lightProbe READ lightProbe WRITE setLightProbe NOTIFY lightProbeChanged)
    Q_PROPERTY(CullMode cullMode READ cullMode WRITE setCullMode NOTIFY cullModeChanged)
    Q_PROPERTY(DepthDrawMode depthDrawMode READ depthDrawMode WRITE setDepthDrawMode NOTIFY depthDrawModeChanged)

    QML_NAMED_ELEMENT(Material)
    QML_UNCREATABLE("Material is Abstract")

public:
    enum CullMode {
        BackFaceCulling = 1,
        FrontFaceCulling = 2,
        NoCulling = 3
    };
    Q_ENUM(CullMode)

    enum TextureChannelMapping {
        R = 0,
        G,
        B,
        A,
    };
    Q_ENUM(TextureChannelMapping)

    enum DepthDrawMode {
        OpaqueOnlyDepthDraw = 0,
        AlwaysDepthDraw,
        NeverDepthDraw,
        OpaquePrePassDepthDraw,
    };
    Q_ENUM(DepthDrawMode)

    enum VertexColorMask {
        NoMask = 0,
        RoughnessMask = 1,
        NormalStrengthMask = 2,
        SpecularAmountMask = 4,
        ClearcoatAmountMask = 8,
        ClearcoatRoughnessAmountMask = 16,
        ClearcoatNormalStrengthMask = 32,
        HeightAmountMask = 64,
        MetalnessMask = 128,
        OcclusionAmountMask = 256,
        ThicknessFactorMask = 512,
        TransmissionFactorMask = 1024
    };
    Q_ENUM(VertexColorMask)
    Q_DECLARE_FLAGS(VertexColorMaskFlags, VertexColorMask)
    Q_FLAG(VertexColorMaskFlags)

    ~QQuick3DMaterial() override;

    QQuick3DTexture *lightProbe() const;

    CullMode cullMode() const;

    DepthDrawMode depthDrawMode() const;

public Q_SLOTS:
    void setLightProbe(QQuick3DTexture *lightProbe);
    void setCullMode(QQuick3DMaterial::CullMode cullMode);
    void setDepthDrawMode(QQuick3DMaterial::DepthDrawMode depthDrawMode);

Q_SIGNALS:
    void lightProbeChanged(QQuick3DTexture *lightProbe);
    void cullModeChanged(QQuick3DMaterial::CullMode cullMode);
    void depthDrawModeChanged(QQuick3DMaterial::DepthDrawMode depthDrawMode);

protected:
    explicit QQuick3DMaterial(QQuick3DObjectPrivate &dd, QQuick3DObject *parent = nullptr);
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void itemChange(ItemChange, const ItemChangeData &) override;

private:
    void updateSceneManager(QQuick3DSceneManager *sceneManager);
    QQuick3DTexture *m_iblProbe = nullptr;

    CullMode m_cullMode = CullMode::BackFaceCulling;
    DepthDrawMode m_depthDrawMode = DepthDrawMode::OpaqueOnlyDepthDraw;
};

QT_END_NAMESPACE

#endif // QSSGMATERIAL_H
