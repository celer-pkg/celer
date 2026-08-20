// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qphysxworld_p.h"

#include "characterkinematic/PxControllerManager.h"
#include "cooking/PxCooking.h"
#include "extensions/PxDefaultCpuDispatcher.h"
#include "pvd/PxPvdTransport.h"
#include "PxFoundation.h"
#include "PxPhysics.h"
#include "PxPhysicsVersion.h"
#include "PxRigidActor.h"
#include "PxScene.h"
#include "PxSimulationEventCallback.h"

#include "qabstractphysicsnode_p.h"
#include "qphysicsutils_p.h"
#include "qphysicsworld_p.h"
#include "qstaticphysxobjects_p.h"
#include "qtriggerbody_p.h"

QT_BEGIN_NAMESPACE

class SimulationEventCallback : public physx::PxSimulationEventCallback
{
public:
    SimulationEventCallback(QPhysicsWorld *worldIn) : world(worldIn) {};
    virtual ~SimulationEventCallback() = default;

    void onTrigger(physx::PxTriggerPair *pairs, physx::PxU32 count) override
    {
        QMutexLocker locker(&world->m_removedPhysicsNodesMutex);

        for (physx::PxU32 i = 0; i < count; i++) {
            // ignore pairs when shapes have been deleted
            if (pairs[i].flags
                & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER
                   | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
                continue;

            QTriggerBody *triggerNode =
                    static_cast<QTriggerBody *>(pairs[i].triggerActor->userData);

            QAbstractPhysicsNode *otherNode =
                    static_cast<QAbstractPhysicsNode *>(pairs[i].otherActor->userData);

            if (!triggerNode || !otherNode) {
                qWarning() << "QtQuick3DPhysics internal error: null pointer in trigger collision.";
                continue;
            }

            if (world->isNodeRemoved(triggerNode) || world->isNodeRemoved(otherNode))
                continue;

            if (pairs->status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) {
                if (otherNode->sendTriggerReports()) {
                    triggerNode->registerCollision(otherNode);
                }
                if (otherNode->receiveTriggerReports()) {
                    emit otherNode->enteredTriggerBody(triggerNode);
                }
            } else if (pairs->status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST) {
                if (otherNode->sendTriggerReports()) {
                    triggerNode->deregisterCollision(otherNode);
                }
                if (otherNode->receiveTriggerReports()) {
                    emit otherNode->exitedTriggerBody(triggerNode);
                }
            }
        }
    }

    void onConstraintBreak(physx::PxConstraintInfo * /*constraints*/,
                           physx::PxU32 /*count*/) override {};
    void onWake(physx::PxActor ** /*actors*/, physx::PxU32 /*count*/) override {};
    void onSleep(physx::PxActor ** /*actors*/, physx::PxU32 /*count*/) override {};
    void onContact(const physx::PxContactPairHeader &pairHeader, const physx::PxContactPair *pairs,
                   physx::PxU32 nbPairs) override
    {
        QMutexLocker locker(&world->m_removedPhysicsNodesMutex);
        constexpr physx::PxU32 bufferSize = 64;
        physx::PxContactPairPoint contacts[bufferSize];

        for (physx::PxU32 i = 0; i < nbPairs; i++) {
            const physx::PxContactPair &contactPair = pairs[i];

            if (contactPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) {
                QAbstractPhysicsNode *trigger =
                        static_cast<QAbstractPhysicsNode *>(pairHeader.actors[0]->userData);
                QAbstractPhysicsNode *other =
                        static_cast<QAbstractPhysicsNode *>(pairHeader.actors[1]->userData);

                if (!trigger || !other || world->isNodeRemoved(trigger)
                    || world->isNodeRemoved(other) || !trigger->m_backendObject
                    || !other->m_backendObject)
                    continue;

                const bool triggerReceive =
                        trigger->receiveContactReports() && other->sendContactReports();
                const bool otherReceive =
                        other->receiveContactReports() && trigger->sendContactReports();

                if (!triggerReceive && !otherReceive)
                    continue;

                physx::PxU32 nbContacts = pairs[i].extractContacts(contacts, bufferSize);

                QList<QVector3D> positions;
                QList<QVector3D> impulses;
                QList<QVector3D> normals;

                positions.reserve(nbContacts);
                impulses.reserve(nbContacts);
                normals.reserve(nbContacts);

                for (physx::PxU32 j = 0; j < nbContacts; j++) {
                    physx::PxVec3 position = contacts[j].position;
                    physx::PxVec3 impulse = contacts[j].impulse;
                    physx::PxVec3 normal = contacts[j].normal;

                    positions.push_back(QPhysicsUtils::toQtType(position));
                    impulses.push_back(QPhysicsUtils::toQtType(impulse));
                    normals.push_back(QPhysicsUtils::toQtType(normal));
                }

                QList<QVector3D> normalsInverted;
                normalsInverted.reserve(normals.size());
                for (const QVector3D &v : normals) {
                    normalsInverted.push_back(QVector3D(-v.x(), -v.y(), -v.z()));
                }

                if (triggerReceive)
                    world->registerContact(other, trigger, positions, impulses, normals);
                if (otherReceive)
                    world->registerContact(trigger, other, positions, impulses, normalsInverted);
            }
        }
    };
    void onAdvance(const physx::PxRigidBody *const * /*bodyBuffer*/,
                   const physx::PxTransform * /*poseBuffer*/,
                   const physx::PxU32 /*count*/) override {};

private:
    QPhysicsWorld *world = nullptr;
};

static constexpr bool isBitSet(quint32 value, quint32 position)
{
    Q_ASSERT(position <= 32);
    return value & (1 << (position));
}

static physx::PxFilterFlags
contactReportFilterShader(physx::PxFilterObjectAttributes /*attributes0*/,
                          physx::PxFilterData filterData0,
                          physx::PxFilterObjectAttributes /*attributes1*/,
                          physx::PxFilterData filterData1, physx::PxPairFlags &pairFlags,
                          const void * /*constantBlock*/, physx::PxU32 /*constantBlockSize*/)
{
    // First word is id, second is collision mask
    const quint32 id0 = filterData0.word0;
    const quint32 id1 = filterData1.word0;
    const quint32 mask0 = filterData0.word1;
    const quint32 mask1 = filterData1.word1;

    // If any 'id' bit is set in the other mask it means collisions should be ignored
    if (id0 < 32 && id1 < 32 && (isBitSet(mask0, id1) || isBitSet(mask1, id0))) {
        // We return a 'suppress' since that will still re-evaluate when filter data is changed.
        return physx::PxFilterFlag::eSUPPRESS;
    }

    // Makes objects collide
    const auto defaultCollisonFlags =
            physx::PxPairFlag::eSOLVE_CONTACT | physx::PxPairFlag::eDETECT_DISCRETE_CONTACT;

    // For trigger body detection
    const auto notifyTouchFlags =
            physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_TOUCH_LOST;

    // For contact detection
    const auto notifyContactFlags = physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;

    pairFlags = defaultCollisonFlags | notifyTouchFlags | notifyContactFlags;
    return physx::PxFilterFlag::eDEFAULT;
}

static physx::PxFilterFlags
contactReportFilterShaderCCD(physx::PxFilterObjectAttributes /*attributes0*/,
                             physx::PxFilterData /*filterData0*/,
                             physx::PxFilterObjectAttributes /*attributes1*/,
                             physx::PxFilterData /*filterData1*/, physx::PxPairFlags &pairFlags,
                             const void * /*constantBlock*/, physx::PxU32 /*constantBlockSize*/)
{
    // Makes objects collide
    const auto defaultCollisonFlags = physx::PxPairFlag::eSOLVE_CONTACT
            | physx::PxPairFlag::eDETECT_DISCRETE_CONTACT | physx::PxPairFlag::eDETECT_CCD_CONTACT;

    // For trigger body detection
    const auto notifyTouchFlags =
            physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_TOUCH_LOST;

    // For contact detection
    const auto notifyContactFlags = physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;

    pairFlags = defaultCollisonFlags | notifyTouchFlags | notifyContactFlags;
    return physx::PxFilterFlag::eDEFAULT;
}

#define PHYSX_RELEASE(x)                                                                           \
    if (x != nullptr) {                                                                            \
        x->release();                                                                              \
        x = nullptr;                                                                               \
    }

void QPhysXWorld::createWorld()
{
    auto &s_physx = StaticPhysXObjects::getReference();
    s_physx.foundationRefCount++;

    if (s_physx.foundationCreated)
        return;

    s_physx.foundation = PxCreateFoundation(
            PX_PHYSICS_VERSION, s_physx.defaultAllocatorCallback, s_physx.defaultErrorCallback);
    if (!s_physx.foundation)
        qFatal("PxCreateFoundation failed!");

    s_physx.foundationCreated = true;

#if PHYSX_ENABLE_PVD
    s_physx.pvd = PxCreatePvd(*m_physx->foundation);
    s_physx.transport = physx::PxDefaultPvdSocketTransportCreate("qt", 5425, 10);
    s_physx.pvd->connect(*m_physx->transport, physx::PxPvdInstrumentationFlag::eALL);
#endif

    // FIXME: does the tolerance matter?
    s_physx.cooking = PxCreateCooking(PX_PHYSICS_VERSION, *s_physx.foundation,
                                      physx::PxCookingParams(physx::PxTolerancesScale()));

}

void QPhysXWorld::deleteWorld()
{
    auto &s_physx = StaticPhysXObjects::getReference();
    s_physx.foundationRefCount--;
    if (s_physx.foundationRefCount == 0) {
        PHYSX_RELEASE(controllerManager);
        PHYSX_RELEASE(scene);
        PHYSX_RELEASE(s_physx.dispatcher);
        PHYSX_RELEASE(s_physx.cooking);
        PHYSX_RELEASE(s_physx.transport);
        PHYSX_RELEASE(s_physx.pvd);
        PHYSX_RELEASE(s_physx.physics);
        PHYSX_RELEASE(s_physx.foundation);

        delete callback;
        callback = nullptr;
        s_physx.foundationCreated = false;
        s_physx.physicsCreated = false;
    } else {
        delete callback;
        callback = nullptr;
        PHYSX_RELEASE(controllerManager);
        PHYSX_RELEASE(scene);
    }
}

void QPhysXWorld::createScene(float typicalLength, float typicalSpeed, const QVector3D &gravity,
                              bool enableCCD, QPhysicsWorld *physicsWorld, unsigned int numThreads)
{
    if (scene) {
        qWarning() << "Scene already created";
        return;
    }

    physx::PxTolerancesScale scale;
    scale.length = typicalLength;
    scale.speed = typicalSpeed;

    auto &s_physx = StaticPhysXObjects::getReference();

    if (!s_physx.physicsCreated) {
        constexpr bool recordMemoryAllocations = true;
        s_physx.physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_physx.foundation, scale,
                                          recordMemoryAllocations, s_physx.pvd);
        if (!s_physx.physics)
            qFatal("PxCreatePhysics failed!");

        s_physx.dispatcher = physx::PxDefaultCpuDispatcherCreate(numThreads);
        s_physx.physicsCreated = true;
    }

    callback = new SimulationEventCallback(physicsWorld);

    physx::PxSceneDesc sceneDesc(scale);
    sceneDesc.gravity = QPhysicsUtils::toPhysXType(gravity);
    sceneDesc.cpuDispatcher = s_physx.dispatcher;

    if (enableCCD) {
        sceneDesc.filterShader = contactReportFilterShaderCCD;
        sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD;
    } else {
        sceneDesc.filterShader = contactReportFilterShader;
    }
    sceneDesc.solverType = physx::PxSolverType::eTGS;
    sceneDesc.simulationEventCallback = callback;

    if (physicsWorld->reportKinematicKinematicCollisions())
        sceneDesc.kineKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
    if (physicsWorld->reportStaticKinematicCollisions())
        sceneDesc.staticKineFilteringMode = physx::PxPairFilteringMode::eKEEP;

    scene = s_physx.physics->createScene(sceneDesc);
}

QT_END_NAMESPACE
