// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXINPUTMANAGER_VISIONOS_P_H
#define QQUICK3DXINPUTMANAGER_VISIONOS_P_H

#include <QtQuick3DXr/private/qtquick3dxrglobal_p.h>

#include <QtGui/qquaternion.h>

#ifdef __OBJC__
#import <ARKit/ARKit.h>
#else
typedef struct ar_hand_anchor_s *ar_hand_anchor_t;
typedef struct ar_hand_tracking_provider_s *ar_hand_tracking_provider_t;
typedef struct ar_data_providers_s *ar_data_providers_t;
#endif

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

QT_BEGIN_NAMESPACE

class QQuick3DXrHandInput;
class QQuick3DXrInputManager;
class QQuick3DXrHandModel;
class QQuick3DXrController;
class QQuick3DViewport;

class QQuick3DXrInputManagerPrivate
{
    Q_DECLARE_PUBLIC(QQuick3DXrInputManager)
public:
    using Hand = QtQuick3DXr::Hand;
    using HandPoseSpace = QtQuick3DXr::HandPoseSpace;

    explicit QQuick3DXrInputManagerPrivate(QQuick3DXrInputManager &manager);
    ~QQuick3DXrInputManagerPrivate();

    // Two step process
    // 1.
    void prepareHandtracking(ar_data_providers_t dataProviders);
    // 2.
    void initHandtracking();

    void teardown();

    bool isValid() const { return m_initialized; }

    static QQuick3DXrInputManagerPrivate *get(QQuick3DXrInputManager *inputManager);

    void setPosePositionAndRotation(Hand hand, HandPoseSpace poseSpace, const QVector3D &position, const QQuaternion &rotation);

    void registerController(QQuick3DXrController *controller);
    void unregisterController(QQuick3DXrController *controller);

    bool isPoseInUse(Hand hand, HandPoseSpace poseSpace);

    QQuick3DXrHandInput *leftHandInput() const;
    QQuick3DXrHandInput *rightHandInput() const;

    void setupHandModel(QQuick3DXrHandModel *model);

    qsizetype getPokeJointIndex() const;

    void updateHandtracking();

    static void processSpatialEvents(const QQuick3DViewport &vrViewport, const QJsonObject &events);

private:
    QQuick3DXrInputManager *q_ptr = nullptr;
    QQuick3DXrHandInput *m_handInputState[2] {};

    QSet<QQuick3DXrController *> m_controllers;
    bool m_poseInUse[2][2] = {};
    bool m_poseUsageDirty = true;

    ar_hand_tracking_provider_t m_handTrackingProvider;
    ar_hand_anchor_t m_handAnchors[2] {};

    bool m_isHandTrackingSupported = false;
    bool m_initialized = false;

    struct JointCache {
        QList<QVector3D> positions;
        QList<QQuaternion> rotations;
    } jcache[2];
};

QT_END_NAMESPACE

#endif // QQUICK3DXINPUTMANAGER_VISIONOS_P_H
