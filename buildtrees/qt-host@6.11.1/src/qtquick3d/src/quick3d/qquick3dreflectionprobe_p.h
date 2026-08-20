// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGREFLECTIONPROBE_H
#define QSSGREFLECTIONPROBE_H

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

#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQuick3D/private/qquick3dgeometry_p.h>
#include <QtQuick3D/private/qquick3dmodel_p.h>
#include <QtQuick3D/private/qquick3ddefaultmaterial_p.h>
#include <QtQuick3D/private/qquick3dviewport_p.h>
#include <QtQuick3D/private/qquick3dcubemaptexture_p.h>
#include <QColor>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DReflectionProbe : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(ReflectionQuality quality READ quality WRITE setQuality NOTIFY qualityChanged)
    Q_PROPERTY(QColor clearColor READ clearColor WRITE setClearColor NOTIFY clearColorChanged)
    Q_PROPERTY(ReflectionRefreshMode refreshMode READ refreshMode WRITE setRefreshMode NOTIFY refreshModeChanged)
    Q_PROPERTY(ReflectionTimeSlicing timeSlicing READ timeSlicing WRITE setTimeSlicing NOTIFY timeSlicingChanged)
    Q_PROPERTY(bool parallaxCorrection READ parallaxCorrection WRITE setParallaxCorrection NOTIFY parallaxCorrectionChanged)
    Q_PROPERTY(QVector3D boxSize READ boxSize WRITE setBoxSize NOTIFY boxSizeChanged)
    Q_PROPERTY(QVector3D boxOffset READ boxOffset WRITE setBoxOffset NOTIFY boxOffsetChanged REVISION(6, 4))
    Q_PROPERTY(bool debugView READ debugView WRITE setDebugView NOTIFY debugViewChanged REVISION(6, 4))
    Q_PROPERTY(QQuick3DCubeMapTexture *texture READ texture WRITE setTexture NOTIFY textureChanged REVISION(6, 5))
    QML_NAMED_ELEMENT(ReflectionProbe)
    QML_ADDED_IN_VERSION(6, 3)

public:
    enum class ReflectionQuality {
        VeryLow,
        Low,
        Medium,
        High,
        VeryHigh
    };
    Q_ENUM(ReflectionQuality)

    enum class ReflectionRefreshMode {
        FirstFrame,
        EveryFrame
    };
    Q_ENUM(ReflectionRefreshMode)

    enum class ReflectionTimeSlicing {
        None,
        AllFacesAtOnce,
        IndividualFaces,
    };
    Q_ENUM(ReflectionTimeSlicing)

    explicit QQuick3DReflectionProbe(QQuick3DNode *parent = nullptr);
    ~QQuick3DReflectionProbe() override { }

    ReflectionQuality quality() const;
    QColor clearColor() const;
    ReflectionRefreshMode refreshMode() const;
    ReflectionTimeSlicing timeSlicing() const;
    bool parallaxCorrection() const;
    QVector3D boxSize() const;
    Q_REVISION(6, 4) bool debugView() const;
    Q_REVISION(6, 4) QVector3D boxOffset() const;

    Q_REVISION(6, 4) Q_INVOKABLE void scheduleUpdate();
    Q_REVISION(6, 5) QQuick3DCubeMapTexture *texture() const;

public Q_SLOTS:
    void setQuality(ReflectionQuality reflectionQuality);
    void setClearColor(const QColor &clearColor);
    void setRefreshMode(ReflectionRefreshMode newRefreshMode);
    void setTimeSlicing(ReflectionTimeSlicing newTimeSlicing);
    void setParallaxCorrection(bool parallaxCorrection);
    void setBoxSize(const QVector3D &newBoxSize);
    Q_REVISION(6, 4) void setDebugView(bool debugView);
    Q_REVISION(6, 4) void setBoxOffset(const QVector3D &boxOffset);
    Q_REVISION(6, 5) void setTexture(QQuick3DCubeMapTexture *newTexture);

Q_SIGNALS:
    void qualityChanged();
    void clearColorChanged();
    void refreshModeChanged();
    void timeSlicingChanged();
    void parallaxCorrectionChanged();

    void boxSizeChanged();
    Q_REVISION(6, 4) void debugViewChanged();
    Q_REVISION(6, 4) void boxOffsetChanged();
    Q_REVISION(6, 5) void textureChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void markAllDirty() override;
    void itemChange(ItemChange, const ItemChangeData &) override;

    enum class DirtyFlag {
        QualityDirty = (1 << 0),
        ClearColorDirty = (1 << 1),
        RefreshModeDirty = (1 << 2),
        ParallaxCorrectionDirty = (1 << 3),
        BoxDirty = (1 << 4),
        TimeSlicingDirty = (1 << 5),
        TextureDirty = (1 << 6)
    };
    Q_DECLARE_FLAGS(DirtyFlags, DirtyFlag)

    static constexpr DirtyFlags AllDirty = DirtyFlags(DirtyFlag::QualityDirty)
                                           | DirtyFlags(DirtyFlag::ClearColorDirty)
                                           | DirtyFlags(DirtyFlag::RefreshModeDirty)
                                           | DirtyFlags(DirtyFlag::ParallaxCorrectionDirty)
                                           | DirtyFlags(DirtyFlag::BoxDirty)
                                           | DirtyFlags(DirtyFlag::TimeSlicingDirty)
                                           | DirtyFlags(DirtyFlag::TextureDirty);

    DirtyFlags m_dirtyFlags = QQuick3DReflectionProbe::AllDirty;

private:
    quint32 mapToReflectionResolution(ReflectionQuality quality);
    void findSceneView();
    void createDebugView();
    void updateDebugView();
    void updateSceneManager(QQuick3DSceneManager *window);
    ReflectionQuality m_quality = ReflectionQuality::Low;
    QColor m_clearColor = Qt::transparent;
    ReflectionRefreshMode m_refreshMode = ReflectionRefreshMode::EveryFrame;
    bool m_parallaxCorrection = false;
    QVector3D m_boxSize = QVector3D(0, 0, 0);
    ReflectionTimeSlicing m_timeSlicing = ReflectionTimeSlicing::None;
    bool m_debugView = false;
    // These objects are used to visualize the reflection probe box.
    QQuick3DGeometry *m_debugViewGeometry = nullptr;
    QQuick3DModel *m_debugViewModel = nullptr;
    QQuick3DDefaultMaterial *m_debugViewMaterial = nullptr;
    QVector3D m_boxOffset;
    QQuick3DViewport* m_sceneView = nullptr;
    QQuick3DCubeMapTexture *m_texture = nullptr;
};

QT_END_NAMESPACE

#endif // QSSGREFLECTIONPROBE_H
