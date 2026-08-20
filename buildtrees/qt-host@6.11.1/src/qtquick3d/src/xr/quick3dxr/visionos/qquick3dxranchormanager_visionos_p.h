// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRANCHORMANAGER_VISIONOS_P_H
#define QQUICK3DXRANCHORMANAGER_VISIONOS_P_H

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

#include <QtQuick3DXr/private/qtquick3dxrglobal_p.h>

#include <QtCore/quuid.h>

#ifdef __OBJC__
#import <ARKit/ARKit.h>
#else
typedef struct ar_data_providers_s *ar_data_providers_t;
typedef struct ar_plane_detection_provider_s *ar_plane_detection_provider_t;
typedef struct ar_plane_anchors_s *ar_plane_anchors_t;
#endif

QT_BEGIN_NAMESPACE

class QVector2D;
class QVector3D;

class QQuick3DXrSpatialAnchor;

class QQuick3DXrAnchorManager : public QObject
{
    Q_OBJECT
public:
    static QQuick3DXrAnchorManager* instance();

    // Note: There's only one "space" in the VisionOS world, so we always return 1 (0 equals invalid).
    QtQuick3DXr::XrSpaceId getCurrentSpaceId() const { return QtQuick3DXr::XrSpaceId(1); }

    void requestSceneCapture();
    bool queryAllAnchors();

    void teardown();

    QList<QQuick3DXrSpatialAnchor *> anchors() const;
    qsizetype anchorCount() const;

    bool getBoundingBox3D(QtQuick3DXr::XrSpaceId space, QVector3D &offset, QVector3D &extent);
    bool getBoundingBox2D(QtQuick3DXr::XrSpaceId space, QVector2D &offset, QVector2D &extent);

    bool setupSpatialAnchor(QtQuick3DXr::XrSpaceId space, QQuick3DXrSpatialAnchor &anchor);

    void addAnchor(QQuick3DXrSpatialAnchor *anchor);
    void removeAnchor(QUuid uuid);
    void updateAnchor(QQuick3DXrSpatialAnchor *anchor);

    void populateAnchorsList();

Q_SIGNALS:
    void anchorAdded(QQuick3DXrSpatialAnchor *anchor);
    void anchorRemoved(QUuid uuid);
    void anchorUpdated(QQuick3DXrSpatialAnchor *anchor);

    void sceneCaptureCompleted();

private:
    friend class QQuick3DXrManagerPrivate;

    explicit QQuick3DXrAnchorManager(QObject *parent = nullptr);
    ~QQuick3DXrAnchorManager() override;

    void prepareAnchorManager(ar_data_providers_t dataProviders);
    void initAnchorManager();

    static void planeUpdateHandler(void *, ar_plane_anchors_t, ar_plane_anchors_t, ar_plane_anchors_t);

    ar_plane_detection_provider_t m_planeDetectionProvider;

    mutable QReadWriteLock m_anchorsLock;
    QList<QQuick3DXrSpatialAnchor *> m_anchors;
    QHash<QUuid, QQuick3DXrSpatialAnchor *> m_anchorsByUuid;

    enum class AnchorType {
        Plane,
    };

    AnchorType m_requestedAnchorType = AnchorType::Plane;

    bool m_isInitialized = false;
    bool m_isPlaneDetectionSupported = false;
};

QT_END_NAMESPACE

#endif // QQUICK3DXRANCHORMANAGER_VISIONOS_P_H
