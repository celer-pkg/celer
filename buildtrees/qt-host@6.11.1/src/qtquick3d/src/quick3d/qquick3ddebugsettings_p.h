// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DDEBUGSETTINGS_H
#define QQUICK3DDEBUGSETTINGS_H

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

#include <QtQuick3D/private/qquick3dobject_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DDebugSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DMaterialOverrides materialOverride READ materialOverride WRITE setMaterialOverride NOTIFY materialOverrideChanged)
    Q_PROPERTY(bool wireframeEnabled READ wireframeEnabled WRITE setWireframeEnabled NOTIFY wireframeEnabledChanged)
    Q_PROPERTY(bool drawDirectionalLightShadowBoxes READ drawDirectionalLightShadowBoxes WRITE
                       setDrawDirectionalLightShadowBoxes NOTIFY drawDirectionalLightShadowBoxesChanged FINAL REVISION(6, 8))
    Q_PROPERTY(bool drawPointLightShadowBoxes READ drawPointLightShadowBoxes WRITE setDrawPointLightShadowBoxes NOTIFY
                       drawPointLightShadowBoxesChanged FINAL REVISION(6, 9))
    Q_PROPERTY(bool drawShadowCastingBounds READ drawShadowCastingBounds WRITE setDrawShadowCastingBounds NOTIFY drawShadowCastingBoundsChanged FINAL REVISION(6, 8))
    Q_PROPERTY(bool drawShadowReceivingBounds READ drawShadowReceivingBounds WRITE setDrawShadowReceivingBounds NOTIFY drawShadowReceivingBoundsChanged FINAL REVISION(6, 8))
    Q_PROPERTY(bool drawCascades READ drawCascades WRITE setDrawCascades NOTIFY drawCascadesChanged FINAL REVISION(6, 8))
    Q_PROPERTY(bool drawSceneCascadeIntersection READ drawSceneCascadeIntersection WRITE setDrawSceneCascadeIntersection NOTIFY drawSceneCascadeIntersectionChanged FINAL REVISION(6, 8))
    Q_PROPERTY(bool disableShadowCameraUpdate READ disableShadowCameraUpdate WRITE setDisableShadowCameraUpdate NOTIFY disableShadowCameraUpdateChanged FINAL REVISION(6, 8))
    Q_PROPERTY(bool drawCulledObjects READ drawCulledObjects WRITE setDrawCulledObjects NOTIFY drawCulledObjectsChanged FINAL REVISION(6, 11))

    QML_NAMED_ELEMENT(DebugSettings)
public:
    enum QQuick3DMaterialOverrides {
        None,
        BaseColor,
        Roughness,
        Metalness,
        Diffuse,
        Specular,
        ShadowOcclusion,
        Emission,
        AmbientOcclusion,
        Normals,
        Tangents,
        Binormals,
        F0
    };
    Q_ENUM(QQuick3DMaterialOverrides)

    explicit QQuick3DDebugSettings(QObject *parent = nullptr);

    QQuick3DMaterialOverrides materialOverride() const;
    void setMaterialOverride(QQuick3DMaterialOverrides newMaterialOverride);

    bool wireframeEnabled() const;
    void setWireframeEnabled(bool newWireframeEnabled);

    Q_REVISION(6, 8) bool drawDirectionalLightShadowBoxes() const;
    Q_REVISION(6, 8) void setDrawDirectionalLightShadowBoxes(bool newDrawDirectionalLightShadowBoxes);

    Q_REVISION(6, 9) bool drawPointLightShadowBoxes() const;
    Q_REVISION(6, 9) void setDrawPointLightShadowBoxes(bool newDrawPointLightShadowBoxes);

    Q_REVISION(6, 8) bool drawShadowCastingBounds() const;
    Q_REVISION(6, 8) void setDrawShadowCastingBounds(bool newDrawShadowCastingBounds);

    Q_REVISION(6, 8) bool drawShadowReceivingBounds() const;
    Q_REVISION(6, 8) void setDrawShadowReceivingBounds(bool newDrawShadowReceivingBounds);

    Q_REVISION(6, 8) bool drawCascades() const;
    Q_REVISION(6, 8) void setDrawCascades(bool newDrawCascades);

    Q_REVISION(6, 8) bool drawSceneCascadeIntersection() const;
    Q_REVISION(6, 8) void setDrawSceneCascadeIntersection(bool newDrawSceneCascadeIntersection);

    Q_REVISION(6, 8) bool disableShadowCameraUpdate() const;
    Q_REVISION(6, 8) void setDisableShadowCameraUpdate(bool newDisableShadowCameraUpdate);

    Q_REVISION(6, 11) bool drawCulledObjects() const;
    Q_REVISION(6, 11) void setDrawCulledObjects(bool newDrawCulledObjects);

Q_SIGNALS:
    void materialOverrideChanged();
    void wireframeEnabledChanged();
    Q_REVISION(6, 8) void drawDirectionalLightShadowBoxesChanged();
    Q_REVISION(6, 9) void drawPointLightShadowBoxesChanged();
    Q_REVISION(6, 8) void drawShadowCastingBoundsChanged();
    Q_REVISION(6, 8) void drawShadowReceivingBoundsChanged();
    Q_REVISION(6, 8) void drawCascadesChanged();
    Q_REVISION(6, 8) void drawSceneCascadeIntersectionChanged();
    Q_REVISION(6, 8) void disableShadowCameraUpdateChanged();
    Q_REVISION(6, 11) void drawCulledObjectsChanged();
    void changed();

private:
    void update();
    QQuick3DMaterialOverrides m_materialOverride = None;

    bool m_wireframeEnabled = false;
    bool m_drawDirectionalLightShadowBoxes = false;
    bool m_drawPointLightShadowBoxes = false;
    bool m_drawShadowCastingBounds = false;
    bool m_drawShadowReceivingBounds = false;
    bool m_drawCascades = false;
    bool m_drawSceneCascadeIntersection = false;
    bool m_disableShadowCameraUpdate = false;
    bool m_drawCulledObjects = false;
};

QT_END_NAMESPACE

#endif // QQUICK3DDEBUGSETTINGS_H
