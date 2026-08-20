// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PHYSICSWORLD_H
#define PHYSICSWORLD_H

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

#include <QtQuick3DPhysics/qtquick3dphysicsglobal.h>

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QTimerEvent>
#include <QtCore/QElapsedTimer>
#include <QtGui/QVector3D>
#include <QtQml/qqml.h>
#include <QBasicTimer>

#include <QtQuick3D/private/qquick3dviewport_p.h>

namespace physx {
class PxMaterial;
class PxPhysics;
class PxShape;
class PxRigidDynamic;
class PxRigidActor;
class PxRigidStatic;
class PxCooking;
class PxControllerManager;
class PxConvexMesh;
class PxTriangleMesh;
class PxHeightField;
}

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcQuick3dPhysics);

class QAbstractPhysicsNode;
class QAbstractCollisionShape;
class QAbstractRigidBody;
class QAbstractPhysXNode;
class QQuick3DModel;
class QQuick3DGeometry;
class QQuick3DPrincipledMaterial;
class QPhysXWorld;
class FrameAnimator;

class Q_QUICK3DPHYSICS_EXPORT QPhysicsWorld : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QVector3D gravity READ gravity WRITE setGravity NOTIFY gravityChanged)
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(bool forceDebugDraw READ forceDebugDraw WRITE setForceDebugDraw NOTIFY
                       forceDebugDrawChanged)
    Q_PROPERTY(bool enableCCD READ enableCCD WRITE setEnableCCD NOTIFY enableCCDChanged)
    Q_PROPERTY(float typicalLength READ typicalLength WRITE setTypicalLength NOTIFY
                       typicalLengthChanged)
    Q_PROPERTY(
            float typicalSpeed READ typicalSpeed WRITE setTypicalSpeed NOTIFY typicalSpeedChanged)
    Q_PROPERTY(float defaultDensity READ defaultDensity WRITE setDefaultDensity NOTIFY
                       defaultDensityChanged)
    Q_PROPERTY(QQuick3DNode *viewport READ viewport WRITE setViewport NOTIFY viewportChanged
                       REVISION(6, 5))
    Q_PROPERTY(float minimumTimestep READ minimumTimestep WRITE setMinimumTimestep NOTIFY
                       minimumTimestepChanged REVISION(6, 5))
    Q_PROPERTY(float maximumTimestep READ maximumTimestep WRITE setMaximumTimestep NOTIFY
                       maximumTimestepChanged REVISION(6, 5))
    Q_PROPERTY(QQuick3DNode *scene READ scene WRITE setScene NOTIFY sceneChanged REVISION(6, 5))
    Q_PROPERTY(int numThreads READ numThreads WRITE setNumThreads NOTIFY numThreadsChanged
                       REVISION(6, 7))
    Q_PROPERTY(bool reportKinematicKinematicCollisions READ reportKinematicKinematicCollisions WRITE
                       setReportKinematicKinematicCollisions NOTIFY
                               reportKinematicKinematicCollisionsChanged FINAL REVISION(6, 7))
    Q_PROPERTY(bool reportStaticKinematicCollisions READ reportStaticKinematicCollisions WRITE
                       setReportStaticKinematicCollisions NOTIFY
                               reportStaticKinematicCollisionsChanged FINAL REVISION(6, 7))

    QML_NAMED_ELEMENT(PhysicsWorld)

public:
    explicit QPhysicsWorld(QObject *parent = nullptr);
    ~QPhysicsWorld();

    void classBegin() override;
    void componentComplete() override;

    QVector3D gravity() const;

    bool running() const;
    bool forceDebugDraw() const;
    bool enableCCD() const;
    float typicalLength() const;
    float typicalSpeed() const;
    float defaultDensity() const;
    Q_REVISION(6, 5) float minimumTimestep() const;
    Q_REVISION(6, 5) float maximumTimestep() const;

    bool isNodeRemoved(QAbstractPhysicsNode *object);

    static QPhysicsWorld *getWorld(QQuick3DNode *node);

    static void registerNode(QAbstractPhysicsNode *physicsNode);
    static void deregisterNode(QAbstractPhysicsNode *physicsNode);

    void registerContact(QAbstractPhysicsNode *sender, QAbstractPhysicsNode *receiver,
                         const QVector<QVector3D> &positions, const QVector<QVector3D> &impulses,
                         const QVector<QVector3D> &normals);

    Q_REVISION(6, 5) QQuick3DNode *viewport() const;
    void setHasIndividualDebugDraw();
    physx::PxControllerManager *controllerManager();
    Q_REVISION(6, 5) QQuick3DNode *scene() const;
    Q_REVISION(6, 7) int numThreads() const;
    Q_REVISION(6, 7) bool reportKinematicKinematicCollisions() const;
    Q_REVISION(6, 7)
    void setReportKinematicKinematicCollisions(bool newReportKinematicKinematicCollisions);
    Q_REVISION(6, 7) bool reportStaticKinematicCollisions() const;
    Q_REVISION(6, 7)
    void setReportStaticKinematicCollisions(bool newReportStaticKinematicCollisions);

public slots:
    void setGravity(QVector3D gravity);
    void setRunning(bool running);
    void setForceDebugDraw(bool forceDebugDraw);
    void setEnableCCD(bool enableCCD);
    void setTypicalLength(float typicalLength);
    void setTypicalSpeed(float typicalSpeed);
    void setDefaultDensity(float defaultDensity);
    Q_REVISION(6, 5) void setViewport(QQuick3DNode *viewport);
    Q_REVISION(6, 5) void setMinimumTimestep(float minTimestep);
    Q_REVISION(6, 5) void setMaximumTimestep(float maxTimestep);
    Q_REVISION(6, 5) void setScene(QQuick3DNode *newScene);
    Q_REVISION(6, 7) void setNumThreads(int newNumThreads);

signals:
    void gravityChanged(QVector3D gravity);
    void runningChanged(bool running);
    void enableCCDChanged(bool enableCCD);
    void forceDebugDrawChanged(bool forceDebugDraw);
    void typicalLengthChanged(float typicalLength);
    void typicalSpeedChanged(float typicalSpeed);
    void defaultDensityChanged(float defaultDensity);
    Q_REVISION(6, 5) void viewportChanged(QQuick3DNode *viewport);
    Q_REVISION(6, 5) void minimumTimestepChanged(float minimumTimestep);
    Q_REVISION(6, 5) void maximumTimestepChanged(float maxTimestep);
    Q_REVISION(6, 5) void frameDone(float timestep);
    Q_REVISION(6, 5) void sceneChanged();
    Q_REVISION(6, 7) void numThreadsChanged();
    Q_REVISION(6, 7) void reportKinematicKinematicCollisionsChanged();
    Q_REVISION(6, 7) void reportStaticKinematicCollisionsChanged();

private:
    void simulateFrame();
    void frameFinished(float deltaTime);
    void frameFinishedDesignStudio();
    void initPhysics();
    void cleanupRemovedNodes();
    void updateDebugDraw();
    void updateDebugDrawDesignStudio();
    void setupDebugMaterials(QQuick3DNode *sceneNode);
    void disableDebugDraw();
    void matchOrphanNodes();
    void findPhysicsNodes();
    void emitContactCallbacks();

    struct BodyContact
    {
        QAbstractPhysicsNode *sender = nullptr;
        QAbstractPhysicsNode *receiver = nullptr;
        QVector<QVector3D> positions;
        QVector<QVector3D> impulses;
        QVector<QVector3D> normals;
    };

    struct DebugModelHolder
    {
        QQuick3DModel *model = nullptr;
        QQuick3DGeometry *geometry = nullptr;
        QVector3D data;
        void *ptr = nullptr;

        void releaseMeshPointer();

        const QVector3D &halfExtents() const;
        void setHalfExtents(const QVector3D &halfExtents);

        float radius() const;
        void setRadius(float radius);

        float heightScale() const;
        void setHeightScale(float heightScale);

        float halfHeight() const;
        void setHalfHeight(float halfHeight);

        float rowScale() const;
        void setRowScale(float rowScale);

        float columnScale() const;
        void setColumnScale(float columnScale);

        physx::PxConvexMesh *getConvexMesh();
        void setConvexMesh(physx::PxConvexMesh *mesh);

        physx::PxTriangleMesh *getTriangleMesh();
        void setTriangleMesh(physx::PxTriangleMesh *mesh);

        physx::PxHeightField *getHeightField();
        void setHeightField(physx::PxHeightField *hf);
    };

    QList<QAbstractPhysXNode *> m_physXBodies;
    QList<QAbstractPhysicsNode *> m_newPhysicsNodes;
    QHash<QPair<QAbstractCollisionShape *, QAbstractPhysicsNode *>, DebugModelHolder>
            m_DesignStudioDebugModels;
    QHash<QPair<QAbstractCollisionShape *, QAbstractPhysXNode *>, DebugModelHolder>
            m_collisionShapeDebugModels;
    QSet<QAbstractPhysicsNode *> m_removedPhysicsNodes;
    QMutex m_removedPhysicsNodesMutex;
    QList<BodyContact> m_registeredContacts;

    QVector3D m_gravity = QVector3D(0.f, -981.f, 0.f);
    float m_typicalLength = 100.f; // 100 cm
    float m_typicalSpeed = 1000.f; // 1000 cm/s
    float m_defaultDensity = 0.001f; // 1 g/cm^3
    float m_minTimestep = 1.0f; // 1000 fps
    float m_maxTimestep = 33.333f; // 30 fps

    bool m_running = true;
    bool m_forceDebugDraw = false;
    // For performance, used to keep track if we have indiviually enabled debug drawing for any
    // collision shape
    bool m_hasIndividualDebugDraw = false;
    bool m_physicsInitialized = false;
    bool m_enableCCD = false;

    QPhysXWorld *m_physx = nullptr;
    QQuick3DNode *m_viewport = nullptr;
    QVector<QQuick3DPrincipledMaterial *> m_debugMaterials;

    friend class QQuick3DPhysicsMesh; // TODO: better internal API
    friend class QTriangleMeshShape; //####
    friend class QHeightFieldShape;
    friend class QQuick3DPhysicsHeightField;
    friend class SimulationEventCallback;
    friend class ControllerCallback;
    static physx::PxPhysics *getPhysics();
    static physx::PxCooking *getCooking();
    FrameAnimator *m_frameAnimator = nullptr;
    QQuick3DNode *m_scene = nullptr;
    bool m_inDesignStudio = false;
    int m_numThreads = -1;
    bool m_reportKinematicKinematicCollisions = false;
    bool m_reportStaticKinematicCollisions = false;
    QElapsedTimer m_timer;
    float m_currTimeStep = 0.f;
    QList<float> m_frameTimings;
    bool m_frameFetched = false;
};

QT_END_NAMESPACE

#endif // PHYSICSWORLD_H
