// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qquick3dxrhandinput_p.h"
#include "qquick3dxrinputmanager_visionos_p.h"

#include "../qquick3dxrinputmanager_p.h"
#include "../qquick3dxractionmapper_p.h"
#include "../qquick3dxrcontroller_p.h"

#include <QtQuick3DUtils/private/qssgassert_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>

#include <QtQuick3D/private/qquick3dprincipledmaterial_p.h>
#include <QtQuick3D/private/qquick3dviewport_p.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcQuick3DXr);

QQuick3DXrInputManagerPrivate::QQuick3DXrInputManagerPrivate(QQuick3DXrInputManager &manager)
    : q_ptr(&manager)
{
    m_handInputState[Hand::LeftHand] = new QQuick3DXrHandInput(&manager);
    m_handInputState[Hand::RightHand] = new QQuick3DXrHandInput(&manager);
}

QQuick3DXrInputManagerPrivate::~QQuick3DXrInputManagerPrivate()
{

}

static inline void setInputValue(QtQuick3DXr::Hand hand, int id, const char *shortName, float value)
{
    QSSG_ASSERT(hand < 2, hand = QtQuick3DXr::Hand::LeftHand);
    QQuick3DXrActionMapper::handleInput(QQuick3DXrInputAction::Action(id), static_cast<QQuick3DXrInputAction::Controller>(hand), shortName, value);
}


void QQuick3DXrInputManagerPrivate::prepareHandtracking(ar_data_providers_t dataProviders)
{
    QSSG_ASSERT_X(!m_initialized, "Handtracking is already initialized!", return);

    m_isHandTrackingSupported = ar_hand_tracking_provider_is_supported();
    if (m_isHandTrackingSupported) {
        ar_hand_tracking_configuration_t handTrackingConfiguration = ar_hand_tracking_configuration_create();
        m_handTrackingProvider = ar_hand_tracking_provider_create(handTrackingConfiguration);
        ar_data_providers_add_data_provider(dataProviders, m_handTrackingProvider);
    } else {
        qCWarning(lcQuick3DXr, "Handtracking is not supported on this device!");
    }

    qCDebug(lcQuick3DXr) << Q_FUNC_INFO << ", Handtracking supported: " << m_isHandTrackingSupported;
}

void QQuick3DXrInputManagerPrivate::initHandtracking()
{
    if (m_isHandTrackingSupported) {
        m_handAnchors[Hand::LeftHand] = ar_hand_anchor_create();
        m_handAnchors[Hand::RightHand] = ar_hand_anchor_create();
        m_initialized = true;
    }

    qCDebug(lcQuick3DXr) << Q_FUNC_INFO << ", Initialized: " << m_initialized;
}

void QQuick3DXrInputManagerPrivate::teardown()
{
}

void QQuick3DXrInputManagerPrivate::setPosePositionAndRotation(Hand hand, HandPoseSpace poseSpace, const QVector3D &position, const QQuaternion &rotation)
{
    for (QQuick3DXrController *controller : std::as_const(m_controllers)) {
        if (QtQuick3DXr::handForController(controller->controller()) == hand && QtQuick3DXr::pose_cast(controller->poseSpace()) == poseSpace) {
            controller->setPosition(position);
            controller->setRotation(rotation);
        }
    }
}

// Used both to add a new controller, and notify that an existing one has changed
void QQuick3DXrInputManagerPrivate::registerController(QQuick3DXrController *controller)
{
    m_poseUsageDirty = true;
    if (controller->controller() == QQuick3DXrController::ControllerNone) {
        m_controllers.remove(controller);
        return;
    }
    // No point in checking whether it's already in the set: that's just as expensive as inserting
    m_controllers.insert(controller);
}

void QQuick3DXrInputManagerPrivate::unregisterController(QQuick3DXrController *controller)
{
    m_poseUsageDirty = m_controllers.remove(controller);
}

bool QQuick3DXrInputManagerPrivate::isPoseInUse(Hand hand, HandPoseSpace poseSpace)
{
    QSSG_ASSERT(uint(hand) < 2 && uint(poseSpace) < 2, return false);
    if (m_poseUsageDirty) {
        std::fill_n(&m_poseInUse[0][0], 4, false);
        for (const auto *controller : std::as_const(m_controllers)) {
            m_poseInUse[uint(controller->controller())][uint(controller->poseSpace())] = true;
        }
        m_poseUsageDirty = false;
    }
    return m_poseInUse[uint(hand)][uint(poseSpace)];
}

QQuick3DXrHandInput *QQuick3DXrInputManagerPrivate::leftHandInput() const
{
    return m_handInputState[Hand::LeftHand];
}

QQuick3DXrHandInput *QQuick3DXrInputManagerPrivate::rightHandInput() const
{
    return m_handInputState[Hand::RightHand];
}

QQuick3DXrInputManagerPrivate *QQuick3DXrInputManagerPrivate::get(QQuick3DXrInputManager *inputManager)
{
    return inputManager->d_func();
}

void QQuick3DXrInputManagerPrivate::setupHandModel(QQuick3DXrHandModel *model)
{
    Q_UNUSED(model);
}

static bool setupJoint(ar_hand_skeleton_joint_name_t jointName, const ar_hand_skeleton_t handSkeleton, const simd_float4x4 handTransform, QVector3D &jointPosition, QQuaternion &jointRotation)
{
    bool isTracked = false;
    ar_skeleton_joint_t joint = ar_hand_skeleton_get_joint_named(handSkeleton, jointName);
    if (joint != nullptr) {
        if (ar_skeleton_joint_is_tracked(joint)) {
            simd_float4x4 jointTransform = ar_skeleton_joint_get_anchor_from_joint_transform(joint);
            jointTransform = simd_mul(handTransform, jointTransform);

            QMatrix4x4 transform{jointTransform.columns[0].x, jointTransform.columns[1].x, jointTransform.columns[2].x, jointTransform.columns[3].x,
                                   jointTransform.columns[0].y, jointTransform.columns[1].y, jointTransform.columns[2].y, jointTransform.columns[3].y,
                                   jointTransform.columns[0].z, jointTransform.columns[1].z, jointTransform.columns[2].z, jointTransform.columns[3].z,
                                   0.0f, 0.0f, 0.0f, 1.0f};


            QVector3D jp;
            QVector3D scale;
            QQuaternion jr;
            QSSGUtils::mat44::decompose(transform, jp, scale, jr);

            // NOTE: We need to scale the joint position by 100 to get it into the right scale (m -> cm).
            jointPosition = jp * 100.0f;
            jointRotation = jr;

            isTracked = true;
        }
    }

    return isTracked;
}

using HandJointList = QVarLengthArray<ar_hand_skeleton_joint_name_t, 28>;

const HandJointList &getJointNameTable()
{
    // NOTE: "Joints" are placed in the order from the forarm to the fingers (This might differ from Apple's documentation),
    // moving from the wrist to the finger tips going from the left to the right.
    // That means the forarm is at position 0, and the wrist is at position 1 (this is the origin).

    static const HandJointList jointNames {
        ar_hand_skeleton_joint_name_forearm_arm, // This is the forearm.

        // These are technically the same joint, but depends on the orientation of the hand.
        ar_hand_skeleton_joint_name_wrist, // This is the actual wrist.
        ar_hand_skeleton_joint_name_forearm_wrist,

        // The thumb.
        ar_hand_skeleton_joint_name_thumb_knuckle,
        ar_hand_skeleton_joint_name_thumb_intermediate_base,
        ar_hand_skeleton_joint_name_thumb_intermediate_tip,
        ar_hand_skeleton_joint_name_thumb_tip,

        // The index finger.
        ar_hand_skeleton_joint_name_index_finger_metacarpal,
        ar_hand_skeleton_joint_name_index_finger_knuckle,
        ar_hand_skeleton_joint_name_index_finger_intermediate_base,
        ar_hand_skeleton_joint_name_index_finger_intermediate_tip,
        ar_hand_skeleton_joint_name_index_finger_tip,

        // The middle finger.
        ar_hand_skeleton_joint_name_middle_finger_metacarpal,
        ar_hand_skeleton_joint_name_middle_finger_knuckle,
        ar_hand_skeleton_joint_name_middle_finger_intermediate_base,
        ar_hand_skeleton_joint_name_middle_finger_intermediate_tip,
        ar_hand_skeleton_joint_name_middle_finger_tip,

        // The ring finger.
        ar_hand_skeleton_joint_name_ring_finger_metacarpal,
        ar_hand_skeleton_joint_name_ring_finger_knuckle,
        ar_hand_skeleton_joint_name_ring_finger_intermediate_base,
        ar_hand_skeleton_joint_name_ring_finger_intermediate_tip,
        ar_hand_skeleton_joint_name_ring_finger_tip,

        // The little finger.
        ar_hand_skeleton_joint_name_little_finger_metacarpal,
        ar_hand_skeleton_joint_name_little_finger_knuckle,
        ar_hand_skeleton_joint_name_little_finger_intermediate_base,
        ar_hand_skeleton_joint_name_little_finger_intermediate_tip,
        ar_hand_skeleton_joint_name_little_finger_tip,
    };

    return jointNames;
}

static qsizetype getJointIndex(ar_hand_skeleton_joint_name_t jointName)
{
    qsizetype index = -1;
    const auto &jointNames = getJointNameTable();
    for (size_t i = 0, e = jointNames.size(); i != e && index == -1; ++i) {
        if (jointNames[i] == jointName)
             index = i;
     }

    return index;
}

template <QtQuick3DXr::HandPoseSpace HandPose>
static void getHandPose(ar_hand_skeleton_t handSkeleton, const simd_float4x4 handTransform, QVector3D &posePosition, QQuaternion &poseRotation)
{
    QMatrix4x4 qHandTransform{handTransform.columns[0].x, handTransform.columns[1].x, handTransform.columns[2].x, handTransform.columns[3].x,
                                handTransform.columns[0].y, handTransform.columns[1].y, handTransform.columns[2].y, handTransform.columns[3].y,
                                handTransform.columns[0].z, handTransform.columns[1].z, handTransform.columns[2].z, handTransform.columns[3].z,
                                0.0f, 0.0f, 0.0f, 1.0f};
    simd_float4x4 rotHandTransform = { simd_float4{ qHandTransform(0,0), qHandTransform(1,0), qHandTransform(2,0), qHandTransform(3,0) },
                                       simd_float4{ qHandTransform(0,1), qHandTransform(1,1), qHandTransform(2,1), qHandTransform(3,1) },
                                       simd_float4{ qHandTransform(0,2), qHandTransform(1,2), qHandTransform(2,2), qHandTransform(3,2) },
                                       simd_float4{ qHandTransform(0,3), qHandTransform(1,3), qHandTransform(2,3), qHandTransform(3,3) } };

    if constexpr (HandPose == QtQuick3DXr::HandPoseSpace::AimPose) {
        // Using the knuckle joint as the aim pose, as it produces a more "stable" feeling when aiming.
        setupJoint(ar_hand_skeleton_joint_name_index_finger_knuckle, handSkeleton, rotHandTransform, posePosition, poseRotation);

        static QQuaternion rotation = QQuaternion::fromEulerAngles(QVector3D(0, 90, 90));
        poseRotation = poseRotation * rotation;
    } else {
        static_assert(HandPose == QtQuick3DXr::HandPoseSpace::GripPose);
        setupJoint(ar_hand_skeleton_joint_name_middle_finger_knuckle, handSkeleton, rotHandTransform, posePosition, poseRotation);

        static QQuaternion rotation = QQuaternion::fromEulerAngles(QVector3D(0, 180, 90));
        poseRotation = poseRotation * rotation;
    }
}

struct ActionTypeAndName
{
    const char name[16];
    const float pinchDistanceThreshold;
    QQuick3DXrInputAction::Action type;
};

static const ActionTypeAndName VOPinchGestures[] {
    { "", 0.0, QQuick3DXrInputAction::CustomAction },
    { "index_pinch", 0.005, QQuick3DXrInputAction::IndexFingerPinch },
    { "middle_pinch", 0.009, QQuick3DXrInputAction::MiddleFingerPinch },
    { "ring_pinch", 0.009, QQuick3DXrInputAction::RingFingerPinch },
    { "little_pinch", 0.01, QQuick3DXrInputAction::LittleFingerPinch },
};

static void detectGestures(ar_hand_skeleton_t handSkeleton, QtQuick3DXr::Hand hand)
{
    enum PinchJoints {
        ThumbTip,
        IndexFingerTip,
        MiddleFingerTip,
        RingFingerTip,
        LittleFingerTip,
    };

    const ar_hand_skeleton_joint_name_t pinchJoints[] { ar_hand_skeleton_joint_name_thumb_tip,
                                                        ar_hand_skeleton_joint_name_index_finger_tip,
                                                        ar_hand_skeleton_joint_name_middle_finger_tip,
                                                        ar_hand_skeleton_joint_name_ring_finger_tip,
                                                        ar_hand_skeleton_joint_name_little_finger_tip                                                        };

    constexpr size_t pinchJointCount = std::size(pinchJoints);

    simd_float4x4 jointTransforms[pinchJointCount];
    // Assume the thumb tip is tracked.
    bool isTracked[pinchJointCount] { true, false, false, false };

    for (int i = 0, end = pinchJointCount; isTracked[ThumbTip] && i != end; ++i) {
        ar_hand_skeleton_joint_name_t jointName = pinchJoints[i];
        if (ar_skeleton_joint_t joint = ar_hand_skeleton_get_joint_named(handSkeleton, jointName)) {
            if (ar_skeleton_joint_is_tracked(joint)) {
                jointTransforms[i] = ar_skeleton_joint_get_anchor_from_joint_transform(joint);
                isTracked[i] = true;
            }
        }
    }

    // If the thumb isn't tracked, we can't do anything, bail out.
    if (!isTracked[ThumbTip])
        return;

    // Calculate the distance between the tips and the thumb tip (first one wins).
    const simd_float4 thumbTip = jointTransforms[ThumbTip].columns[3];

    // Start from the index finger (1).
    for (int i = 1, end = pinchJointCount; i != end; ++i) {
        if (!isTracked[i])
            continue;

        const simd_float4 diff = jointTransforms[i].columns[3] - thumbTip;
        const float distance = simd_length(diff);
        setInputValue(hand, VOPinchGestures[i].type, VOPinchGestures[i].name, float(distance < VOPinchGestures[i].pinchDistanceThreshold));
    }
}

qsizetype QQuick3DXrInputManagerPrivate::getPokeJointIndex() const
{
    // NOTE: Static for now...
    static const qsizetype idx = getJointIndex(ar_hand_skeleton_joint_name_index_finger_tip);
    return idx;
}

void QQuick3DXrInputManagerPrivate::updateHandtracking()
{
    if (!m_isHandTrackingSupported)
        return;

    QSSG_ASSERT(m_handTrackingProvider != nullptr, return);
    QSSG_ASSERT(m_handAnchors[Hand::LeftHand] != nullptr && m_handAnchors[Hand::RightHand] != nullptr, return);

    ar_hand_tracking_provider_get_latest_anchors(m_handTrackingProvider, m_handAnchors[Hand::LeftHand], m_handAnchors[Hand::RightHand]);

    // FIXME: We can and probably should cache the hand skeleton.
    ar_hand_skeleton_t handSkeletons[2] {};
    uint64_t handJointCount = 0;
    for (const auto hand : { Hand::LeftHand, Hand::RightHand }) {
        handSkeletons[hand] = ar_hand_anchor_get_hand_skeleton(m_handAnchors[hand]);
        handJointCount = qMax(handJointCount, ar_hand_skeleton_get_joint_count(handSkeletons[hand]));
    }

    const auto &jointNames = getJointNameTable();
    // Sanity check the joint count.
    QSSG_CHECK(handJointCount <= size_t(jointNames.size()));

    for (const auto hand : { Hand::LeftHand, Hand::RightHand }) {
        const auto handSkeleton = handSkeletons[hand];
        if (handSkeleton == nullptr) {
            m_handInputState[hand]->setIsHandTrackingActive(false);
            continue;
        }

        // Clear cached joint data.
        auto &jpositions = jcache[hand].positions;
        auto &jrotations = jcache[hand].rotations;
        jpositions.clear();
        jrotations.clear();

        // NOTE: Separate as we're just sanity checking that the wrist is tracked.
        ar_skeleton_joint_t wristJoinOrigin = ar_hand_skeleton_get_joint_named(handSkeleton, ar_hand_skeleton_joint_name_wrist);
        const bool isWristTracked = ar_skeleton_joint_is_tracked(wristJoinOrigin);

        // The hand transform is relative to the head anchor. The wrist is at the origin of the hand anchor.
        const simd_float4x4 handTransform = ar_anchor_get_origin_from_anchor_transform(m_handAnchors[hand]);

        // Get the joint data.
        for (auto jointName : jointNames) {
            QVector3D jointPosition;
            QQuaternion jointRotation;
            if (setupJoint(jointName, handSkeleton, handTransform, jointPosition, jointRotation)) {
                jpositions.append(jointPosition);
                jrotations.append(jointRotation);
            }
        }

        // Detect gestures.
        detectGestures(handSkeleton, hand);

        // Get and set the aim/grip pose.

        if (isPoseInUse(hand, HandPoseSpace::AimPose) ) {
            QVector3D handPosition;
            QQuaternion handRotation;
            getHandPose<HandPoseSpace::AimPose>(handSkeleton, handTransform, handPosition, handRotation);

            setPosePositionAndRotation(hand, HandPoseSpace::AimPose, handPosition, handRotation);
        } else if (isPoseInUse(hand, HandPoseSpace::GripPose)) {
            QVector3D handPosition;
            QQuaternion handRotation;
            getHandPose<HandPoseSpace::GripPose>(handSkeleton, handTransform, handPosition, handRotation);

            setPosePositionAndRotation(hand, HandPoseSpace::GripPose, handPosition, handRotation);
        }

        m_handInputState[hand]->setJointPositionsAndRotations(jpositions, jrotations);

        m_handInputState[hand]->setIsActive(isWristTracked);
    }
}

void QQuick3DXrInputManagerPrivate::processSpatialEvents(const QQuick3DViewport &vrViewport, const QJsonObject &events)
{
  // An example of the input data
  // {
  //     "id":4404736049417834088,
  //     "inputDevicePose": {
  //         "altitude":0,
  //         "azimuth":1.5707963267948966,
  //         "pose3D":{
  //             "position":{
  //                 "x":0.227996826171875,
  //                 "y":0.957000732421875,
  //                 "z":-0.55999755859375
  //             },
  //             "rotation":{
  //                 "vector":[0,0,0,1]
  //             }
  //         }
  //     },
  //     "kind":"indirectPinch",
  //     "location":[0,0],
  //     "location3D":{
  //         "x":0,
  //         "y":0,
  //         "z":0
  //     },
  //     "modifierKeys":0,
  //     "phase":"ended",
  //     "selectionRay":{
  //         "direction":{
  //             "x":0.3321685791015625,
  //             "y":0.25982666015625,
  //             "z":-0.9067230224609375
  //         },
  //         "origin":{
  //             "x":0.227996826171875,
  //             "y":0.957000732421875,
  //             "z":0
  //         }
  //     },
  //     "timestamp":74368.590710375
  // }

    static qint64 lastId = -1;

    QJsonArray eventArray = events.value(QStringLiteral("events")).toArray();
    for (const auto &event : eventArray) {
        QJsonObject eventObj = event.toObject();
        // qDebug() << eventObj;

        // ID (unique per event)
        const qint64 id = eventObj.value(QStringLiteral("id")).toDouble();
        // timestamp (in seconds)
        //const double timestamp = eventObj.value(QStringLiteral("timestamp")).toDouble();
        // kind
        const QString kind = eventObj.value(QStringLiteral("kind")).toString();
        if (kind != QStringLiteral("indirectPinch"))
            qWarning() << "kind is " << kind << "!";


        // phase
        const QString phase = eventObj.value(QStringLiteral("phase")).toString();

        // selectionRay (check if exists first)
        QJsonObject selectionRayObj = eventObj.value(QStringLiteral("selectionRay")).toObject();
        if (!selectionRayObj.isEmpty()) {
            // origin
            QJsonObject originObj = selectionRayObj.value(QStringLiteral("origin")).toObject();
            QVector3D origin(originObj.value(QStringLiteral("x")).toDouble(), originObj.value(QStringLiteral("y")).toDouble(), originObj.value(QStringLiteral("z")).toDouble());
            // convert meters to cm
            origin *= 100.0;

            // direction
            QJsonObject directionObj = selectionRayObj.value(QStringLiteral("direction")).toObject();
            QVector3D direction(directionObj.value(QStringLiteral("x")).toDouble(), directionObj.value(QStringLiteral("y")).toDouble(), directionObj.value(QStringLiteral("z")).toDouble());

            QEvent::Type eventType;

            if (phase == QStringLiteral("active")) {
                if (lastId != id) {
                    // Press
                    lastId = id;
                    eventType = QEvent::MouseButtonPress;
                } else {
                    // Move
                    eventType = QEvent::MouseMove;
                }
            } else {
                // Release
                lastId = -1;
                eventType = QEvent::MouseButtonRelease;
            }

            QMouseEvent *event = new QMouseEvent(eventType, QPointF(), QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            vrViewport.processPointerEventFromRay(origin, direction, event);
            delete event;
        }
    }
}

QT_END_NAMESPACE
