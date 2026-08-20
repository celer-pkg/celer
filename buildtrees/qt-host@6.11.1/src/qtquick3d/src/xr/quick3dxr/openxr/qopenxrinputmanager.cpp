// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "../qquick3dxrinputmanager_p.h"
#include "qopenxrinputmanager_p.h"
#include "openxr/qopenxrhelpers_p.h"
#include "qquick3dxrhandinput_p.h"
#include "qquick3dxrhandmodel_p.h"

#include "qquick3dxrcontroller_p.h" //### InputAction enum

#include <QDebug>

#include <private/qquick3djoint_p.h>

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcQuick3DXr);

QQuick3DXrInputManagerPrivate::QQuick3DXrInputManagerPrivate(QQuick3DXrInputManager &manager)
    : q_ptr(&manager)
{
    m_handInputState[Hand::LeftHand] = new QQuick3DXrHandInput(this);
    m_handInputState[Hand::RightHand] = new QQuick3DXrHandInput(this);
}

QQuick3DXrInputManagerPrivate::~QQuick3DXrInputManagerPrivate()
{
    teardown();
    delete m_handInputState[Hand::LeftHand];
    delete m_handInputState[Hand::RightHand];

    m_handInputState[Hand::LeftHand] = nullptr;
    m_handInputState[Hand::RightHand] = nullptr;
}

QQuick3DXrInputManagerPrivate::QXRHandComponentPath QQuick3DXrInputManagerPrivate::makeHandInputPaths(const QByteArrayView path)
{
    QXRHandComponentPath res;
    setPath(res.paths[Hand::LeftHand], "/user/hand/left/" + path);
    setPath(res.paths[Hand::RightHand], "/user/hand/right/" + path);
    return res;
}


XrPath QQuick3DXrInputManagerPrivate::makeInputPath(const QByteArrayView path)
{
    XrPath res;
    setPath(res, path.toByteArray());
    return res;
}

QQuick3DGeometry *QQuick3DXrInputManagerPrivate::createHandMeshGeometry(const HandMeshData &handMeshData)
{
    QQuick3DGeometry *geometry = new QQuick3DGeometry();
    geometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);

    // Figure out which attributes should be used
    const qsizetype expectedLength = handMeshData.vertexPositions.size();
    bool hasPositions = !handMeshData.vertexPositions.isEmpty();
    bool hasNormals = handMeshData.vertexNormals.size() >= expectedLength;
    bool hasUV0s = handMeshData.vertexUVs.size() >= expectedLength;
    bool hasJoints = handMeshData.vertexBlendIndices.size() >= expectedLength;
    bool hasWeights = handMeshData.vertexBlendWeights.size() >= expectedLength;
    bool hasIndexes = !handMeshData.indices.isEmpty();

    int offset = 0;
    if (hasPositions) {
        geometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, offset, QQuick3DGeometry::Attribute::ComponentType::F32Type);
        offset += 3 * sizeof(float);
    }

    if (hasNormals) {
        geometry->addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, offset, QQuick3DGeometry::Attribute::ComponentType::F32Type);
        offset += 3 * sizeof(float);
    }

    if (hasUV0s) {
        geometry->addAttribute(QQuick3DGeometry::Attribute::TexCoordSemantic, offset, QQuick3DGeometry::Attribute::ComponentType::F32Type);
        offset += 2 * sizeof(float);
    }

    if (hasJoints) {
        geometry->addAttribute(QQuick3DGeometry::Attribute::JointSemantic, offset, QQuick3DGeometry::Attribute::ComponentType::I32Type);
        offset += 4 * sizeof(qint32);
    }

    if (hasWeights) {
        geometry->addAttribute(QQuick3DGeometry::Attribute::WeightSemantic, offset, QQuick3DGeometry::Attribute::ComponentType::F32Type);
        offset += 4 * sizeof(float);
    }

    if (hasIndexes)
        geometry->addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0, QQuick3DGeometry::Attribute::ComponentType::U16Type);

    // set up the vertex buffer
    const int stride = offset;
    const qsizetype bufferSize = expectedLength * stride;
    geometry->setStride(stride);

    QByteArray vertexBuffer;
    vertexBuffer.reserve(bufferSize);

    QVector3D minBounds;
    QVector3D maxBounds;

    auto appendFloat = [&vertexBuffer](float f) {
        vertexBuffer.append(reinterpret_cast<const char *>(&f), sizeof(float));
    };
    auto appendInt = [&vertexBuffer](qint32 i) {
        vertexBuffer.append(reinterpret_cast<const char *>(&i), sizeof(qint32));
    };

    for (qsizetype i = 0; i < expectedLength; ++i) {
        // start writing float values to vertexBuffer
        if (hasPositions) {
            const QVector3D position = OpenXRHelpers::toQVector(handMeshData.vertexPositions[i]);
            appendFloat(position.x());
            appendFloat(position.y());
            appendFloat(position.z());
            minBounds.setX(qMin(minBounds.x(), position.x()));
            maxBounds.setX(qMax(maxBounds.x(), position.x()));
            minBounds.setY(qMin(minBounds.y(), position.y()));
            maxBounds.setY(qMax(maxBounds.y(), position.y()));
            minBounds.setZ(qMin(minBounds.z(), position.z()));
            maxBounds.setZ(qMax(maxBounds.z(), position.z()));
        }
        if (hasNormals) {
            const auto &normal = handMeshData.vertexNormals[i];
            appendFloat(normal.x);
            appendFloat(normal.y);
            appendFloat(normal.z);
        }

        if (hasUV0s) {
            const auto &uv0 = handMeshData.vertexUVs[i];
            appendFloat(uv0.x);
            appendFloat(uv0.y);
        }

        if (hasJoints) {
            const auto &joint = handMeshData.vertexBlendIndices[i];
            appendInt(joint.x);
            appendInt(joint.y);
            appendInt(joint.z);
            appendInt(joint.w);
        }

        if (hasWeights) {
            const auto &weight = handMeshData.vertexBlendWeights[i];
            appendFloat(weight.x);
            appendFloat(weight.y);
            appendFloat(weight.z);
            appendFloat(weight.w);
        }
    }

    geometry->setBounds(minBounds, maxBounds);
    geometry->setVertexData(vertexBuffer);

    // Index Buffer
    if (hasIndexes) {
        const qsizetype indexLength = handMeshData.indices.size();
        QByteArray indexBuffer;
        indexBuffer.reserve(indexLength * sizeof(int16_t));
        for (qsizetype i = 0; i < indexLength; ++i) {
            const auto &index = handMeshData.indices[i];
            indexBuffer.append(reinterpret_cast<const char *>(&index), sizeof(int16_t));
        }
        geometry->setIndexData(indexBuffer);
    }

    return geometry;
}

void QQuick3DXrInputManagerPrivate::loadBindings(QList<ControllerBindings>* controllerBindingsList)
{
    // Oculus touch and HTC Vive Controllers
    QList<ActionPaths> GripAimHapticSupported = {
        ActionPaths::leftGripPose,
        ActionPaths::leftAimPose,
        ActionPaths::leftHaptic,
        ActionPaths::rightGripPose,
        ActionPaths::rightAimPose,
        ActionPaths::rightHaptic
    };

    // Oculus Touch
    QList<InputMapping> oculusTouchInputMapping = {
        InputMapping{QQuick3DXrInputAction::Button1Pressed, InputNames::XClick, LeftHandSubPath},
        InputMapping{QQuick3DXrInputAction::Button1Pressed, InputNames::AClick, RightHandSubPath},
        InputMapping{QQuick3DXrInputAction::Button2Pressed, InputNames::YClick, LeftHandSubPath},
        InputMapping{QQuick3DXrInputAction::Button2Pressed, InputNames::BClick, RightHandSubPath},
        InputMapping{QQuick3DXrInputAction::Button1Touched, InputNames::XTouch, LeftHandSubPath},
        InputMapping{QQuick3DXrInputAction::Button1Touched, InputNames::ATouch, RightHandSubPath},
        InputMapping{QQuick3DXrInputAction::Button2Touched, InputNames::YTouch, LeftHandSubPath},
        InputMapping{QQuick3DXrInputAction::Button2Touched, InputNames::BTouch, RightHandSubPath},
        InputMapping{QQuick3DXrInputAction::ButtonMenuPressed, InputNames::MenuClick, LeftHandSubPath},
        InputMapping{QQuick3DXrInputAction::ButtonSystemPressed, InputNames::SystemClick, RightHandSubPath},
        InputMapping{QQuick3DXrInputAction::SqueezeValue, InputNames::SqueezeValue, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::TriggerValue, InputNames::TriggerValue, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::TriggerTouched, InputNames::TriggerTouch, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::ThumbstickX, InputNames::ThumbstickX, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::ThumbstickY, InputNames::ThumbstickY, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::ThumbstickPressed, InputNames::ThumbstickClick, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::ThumbstickTouched, InputNames::ThumbstickTouch, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::ThumbrestTouched, InputNames::ThumbrestTouch, BothHandsSubPath},
    };

    ControllerBindings oculusTouch{
        "Oculus touch",
        "/interaction_profiles/oculus/touch_controller",
        oculusTouchInputMapping,
        GripAimHapticSupported
    };
    controllerBindingsList->append(oculusTouch);

    // HTC Vive controller
    QList<InputMapping> viveControllerInputMapping {
        InputMapping{QQuick3DXrInputAction::ButtonMenuPressed, InputNames::MenuClick, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::ButtonSystemPressed, InputNames::SystemClick, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::SqueezePressed, InputNames::SqueezeClick, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::TriggerValue, InputNames::TriggerValue, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::TriggerPressed, InputNames::TriggerClick, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::TrackpadX, InputNames::TrackpadX, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::TrackpadY, InputNames::TrackpadY, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::TrackpadPressed, InputNames::TrackpadClick, BothHandsSubPath},
        InputMapping{QQuick3DXrInputAction::TrackpadTouched, InputNames::TrackpadTouch, BothHandsSubPath},
    };

    ControllerBindings viveController {
        "Vive controller",
        "/interaction_profiles/htc/vive_controller",
        viveControllerInputMapping,
        GripAimHapticSupported
    };
    controllerBindingsList->append(viveController);

    // Microsoft hand interaction extension as supported by Quest 3
    // TODO: there are other, very similar, extensions: XR_HTC_HAND_INTERACTION_EXTENSION_NAME and XR_EXT_HAND_INTERACTION_EXTENSION_NAME
    QList<InputMapping> microsoftHandInteractionExtensionInputMapping {
        InputMapping{QQuick3DXrInputAction::SqueezeValue, InputNames::SqueezeValue, BothHandsSubPath},
    };

    QList<ActionPaths> microsoftHandInteractionExtensionActionPaths = {
        ActionPaths::leftGripPose,
        ActionPaths::leftAimPose,
        ActionPaths::rightGripPose,
        ActionPaths::rightAimPose
    };

    ControllerBindings microsoftHandInteractionExtension {
        "microsoftHandInteractionExtension",
        "/interaction_profiles/microsoft/hand_interaction",
        microsoftHandInteractionExtensionInputMapping,
        microsoftHandInteractionExtensionActionPaths
    };
    controllerBindingsList->append(microsoftHandInteractionExtension);

    #if 0 // Unused
    // Microsoft MRM ### TODO
    QList<InputMapping> microsoftMRMInputMapping {};
    QList<ActionPaths> microsoftMRMActionPaths;

    ControllerBindings microsoftMRM{
        "Microsoft MRM",
        "/interaction_profiles/microsoft/motion_controller",
        microsoftMRMInputMapping,
        microsoftMRMActionPaths
    };
    controllerBindingsList->append(microsoftMRM);

    // Valve Index ### TODO
    QList<InputMapping> valveIndexInputMapping {};
    QList<ActionPaths> valveIndexActionPaths;

    ControllerBindings valveIndex{
        "Valve Index",
        "/interaction_profiles/valve/index_controller",
        valveIndexInputMapping,
        valveIndexActionPaths
    };
    controllerBindingsList->append(valveIndex);
    #endif // Unused
}

void QQuick3DXrInputManagerPrivate::setUpBindings(QList<ControllerBindings>* controllerBindingsList, QMap<InputNames, QXRHandComponentPath>* handComponentPaths)
{
    QMap<ActionPaths, XrActionSuggestedBinding> actionPaths;

    // Hand Left
    XrPath leftGripPose;                  // OCULUS_TOUCH | VALVE_INDEX | MICROSOFT_MRM | HTC_VIVE
    XrPath leftAimPose;                   // OCULUS_TOUCH | VALVE_INDEX | MICROSOFT_MRM | HTC_VIVE
    XrPath leftHaptic;                    // OCULUS_TOUCH | VALVE_INDEX | MICROSOFT_MRM | HTC_VIVE

    setPath(leftGripPose, "/user/hand/left/input/grip/pose");
    setPath(leftAimPose, "/user/hand/left/input/aim/pose");
    setPath(leftHaptic, "/user/hand/left/output/haptic");

    actionPaths.insert(ActionPaths::leftGripPose, {m_handActions.gripPoseAction, leftGripPose});
    actionPaths.insert(ActionPaths::leftAimPose, {m_handActions.aimPoseAction, leftAimPose});
    actionPaths.insert(ActionPaths::leftHaptic, {m_handActions.hapticAction, leftHaptic});

    // Hand Right
    XrPath rightGripPose;                 // OCULUS_TOUCH | VALVE_INDEX | MICROSOFT_MRM | HTC_VIVE
    XrPath rightAimPose;                  // OCULUS_TOUCH | VALVE_INDEX | MICROSOFT_MRM | HTC_VIVE
    XrPath rightHaptic;                   // OCULUS_TOUCH | VALVE_INDEX | MICROSOFT_MRM | HTC_VIVE

    setPath(rightGripPose, "/user/hand/right/input/grip/pose");
    setPath(rightAimPose, "/user/hand/right/input/aim/pose");
    setPath(rightHaptic, "/user/hand/right/output/haptic");

    actionPaths.insert(ActionPaths::rightGripPose, {m_handActions.gripPoseAction, rightGripPose});
    actionPaths.insert(ActionPaths::rightAimPose, {m_handActions.aimPoseAction, rightAimPose});
    actionPaths.insert(ActionPaths::rightHaptic, {m_handActions.hapticAction, rightHaptic});

    for (int i = 0; i < controllerBindingsList->size(); i++) {
        ControllerBindings controllerBindings = controllerBindingsList->at(i);

        if (controllerBindings.profileMappingDefs.size() == 0 || controllerBindings.supportedActionPaths.size() == 0)
            continue;

        XrPath profilePath;
        setPath(profilePath, controllerBindings.profilePath);

        std::vector<XrActionSuggestedBinding> bindings {};

        for (const auto& path : controllerBindings.supportedActionPaths) {
            bindings.push_back(actionPaths.value(path));
        }

        for (const auto &[actionId, path, selector] : controllerBindings.profileMappingDefs) {
            if (selector & LeftHandSubPath)
                bindings.push_back({m_inputActions[actionId], handComponentPaths->value(path).paths[Hand::LeftHand]});
            if (selector & RightHandSubPath)
                bindings.push_back({m_inputActions[actionId], handComponentPaths->value(path).paths[Hand::RightHand]});
        }

        XrInteractionProfileSuggestedBinding suggestedBindings{};
        suggestedBindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
        suggestedBindings.interactionProfile = profilePath;
        suggestedBindings.suggestedBindings = bindings.data();
        suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
        if (!checkXrResult(xrSuggestInteractionProfileBindings(m_instance, &suggestedBindings)))
            qWarning() << "Failed to get suggested interaction profile bindings for " << controllerBindings.profileName;
    }
}

void QQuick3DXrInputManagerPrivate::init(XrInstance instance, XrSession session)
{
    if (m_initialized) {
        qWarning() << "QQuick3DXrInputManager: Trying to initialize an already initialized session";
        teardown();
    }

    m_instance = instance;
    m_session = session;

    setupHandTracking();

    setupActions();

    QMap<InputNames, QXRHandComponentPath> handComponentPaths;

    handComponentPaths.insert(InputNames::AClick, makeHandInputPaths("input/a/click")); // OCULUS_TOUCH (right) | VALVE_INDEX (right + left)
    handComponentPaths.insert(InputNames::BClick, makeHandInputPaths("input/b/click")); // OCULUS_TOUCH (right) | VALVE_INDEX (right + left)
    handComponentPaths.insert(InputNames::ATouch, makeHandInputPaths("input/a/touch")); // OCULUS_TOUCH (right) | VALVE_INDEX (right + left)
    handComponentPaths.insert(InputNames::BTouch, makeHandInputPaths("input/b/touch")); // OCULUS_TOUCH (right) | VALVE_INDEX (right + left)

    handComponentPaths.insert(InputNames::XClick, makeHandInputPaths("input/x/click")); // OCULUS_TOUCH (left)
    handComponentPaths.insert(InputNames::YClick, makeHandInputPaths("input/y/click")); // OCULUS_TOUCH (left)
    handComponentPaths.insert(InputNames::XTouch, makeHandInputPaths("input/x/touch")); // OCULUS_TOUCH (left)
    handComponentPaths.insert(InputNames::YTouch, makeHandInputPaths("input/y/touch")); // OCULUS_TOUCH (left)

    handComponentPaths.insert(InputNames::MenuClick, makeHandInputPaths("input/menu/click")); // OCULUS_TOUCH (left) | MICROSOFT_MRM (right + left) | HTC_VIVE (right + left)
    handComponentPaths.insert(InputNames::SystemClick, makeHandInputPaths("input/system/click")); // OCULUS_TOUCH (right) | VALVE_INDEX (right + left) | HTC_VIVE (right + left)
    handComponentPaths.insert(InputNames::SystemTouch, makeHandInputPaths("input/system/touch")); // VALVE_INDEX (right + left)

    handComponentPaths.insert(InputNames::SqueezeValue, makeHandInputPaths("input/squeeze/value")); // right + left: OCULUS_TOUCH | VALVE_INDEX
    handComponentPaths.insert(InputNames::SqueezeForce, makeHandInputPaths("input/squeeze/force")); // right + left: VALVE_INDEX
    handComponentPaths.insert(InputNames::SqueezeClick, makeHandInputPaths("input/squeeze/click")); // right + left: MICROSOFT_MRM | HTC_VIVE

    handComponentPaths.insert(InputNames::TriggerValue, makeHandInputPaths("input/trigger/value")); // right + left: OCULUS_TOUCH | VALVE_INDEX | MICROSOFT_MRM | HTC_VIVE
    handComponentPaths.insert(InputNames::TriggerTouch, makeHandInputPaths("input/trigger/touch")); // right + left: OCULUS_TOUCH | VALVE_INDEX
    handComponentPaths.insert(InputNames::TriggerClick, makeHandInputPaths("input/trigger/click")); // right + left: VALVE_INDEX | HTC_VIVE

    handComponentPaths.insert(InputNames::ThumbstickX, makeHandInputPaths("input/thumbstick/x")); // OCULUS_TOUCH (right + left) | VALVE_INDEX (right + left) | MICROSOFT_MRM (left)
    handComponentPaths.insert(InputNames::ThumbstickY, makeHandInputPaths("input/thumbstick/y")); // OCULUS_TOUCH (right + left) | VALVE_INDEX (right + left) | MICROSOFT_MRM (left)
    handComponentPaths.insert(InputNames::ThumbstickClick, makeHandInputPaths("input/thumbstick/click")); // OCULUS_TOUCH (right + left) | VALVE_INDEX (right + left) | MICROSOFT_MRM (left)
    handComponentPaths.insert(InputNames::ThumbstickTouch, makeHandInputPaths("input/thumbstick/touch")); // OCULUS_TOUCH (right + left) | VALVE_INDEX (right + left)
    handComponentPaths.insert(InputNames::ThumbrestTouch, makeHandInputPaths("input/thumbrest/touch")); // OCULUS_TOUCH (right + left)

    handComponentPaths.insert(InputNames::TrackpadX, makeHandInputPaths("input/trackpad/x")); // right + left:  VALVE_INDEX | MICROSOFT_MRM | HTC_VIVE
    handComponentPaths.insert(InputNames::TrackpadY, makeHandInputPaths("input/trackpad/y")); // right + left:  VALVE_INDEX | MICROSOFT_MRM | HTC_VIVE
    handComponentPaths.insert(InputNames::TrackpadForce, makeHandInputPaths("input/trackpad/force")); // right + left:  VALVE_INDEX
    handComponentPaths.insert(InputNames::TrackpadClick, makeHandInputPaths("input/trackpad/click")); // right + left:  VALVE_INDEX | MICROSOFT_MRM | HTC_VIVE
    handComponentPaths.insert(InputNames::TrackpadTouch, makeHandInputPaths("input/trackpad/touch")); // right + left:  MICROSOFT_MRM | HTC_VIVE

    // Bindings
    QList<ControllerBindings> controllerBindingsList;
    loadBindings(&controllerBindingsList);
    setUpBindings(&controllerBindingsList, &handComponentPaths);

    // Setup Action Spaces

    XrActionSpaceCreateInfo actionSpaceInfo{};
    actionSpaceInfo.type = XR_TYPE_ACTION_SPACE_CREATE_INFO;
    actionSpaceInfo.action = m_handActions.gripPoseAction;
    actionSpaceInfo.poseInActionSpace.orientation.w = 1.0f;
    //actionSpaceInfo.poseInActionSpace.orientation.y = 1.0f;
    actionSpaceInfo.subactionPath = m_handSubactionPath[0];
    if (!checkXrResult(xrCreateActionSpace(m_session, &actionSpaceInfo, &m_handGripSpace[0])))
        qWarning("Failed to create action space for handGripSpace[0]");
    actionSpaceInfo.subactionPath = m_handSubactionPath[1];
    if (!checkXrResult(xrCreateActionSpace(m_session, &actionSpaceInfo, &m_handGripSpace[1])))
        qWarning("Failed to create action space for handGripSpace[1]");

    actionSpaceInfo.action = m_handActions.aimPoseAction;
    actionSpaceInfo.subactionPath = m_handSubactionPath[0];
    if (!checkXrResult(xrCreateActionSpace(m_session, &actionSpaceInfo, &m_handAimSpace[0])))
        qWarning("Failed to create action space for handAimSpace[0]");
    actionSpaceInfo.subactionPath = m_handSubactionPath[1];
    if (!checkXrResult(xrCreateActionSpace(m_session, &actionSpaceInfo, &m_handAimSpace[1])))
        qWarning("Failed to create action space for handAimSpace[1]");

    // Attach Action set to session

    XrSessionActionSetsAttachInfo attachInfo{};
    attachInfo.type = XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO;
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &m_actionSet;
    if (!checkXrResult(xrAttachSessionActionSets(m_session, &attachInfo)))
        qWarning("Failed to attach action sets to session");

    m_initialized = true;
}

void QQuick3DXrInputManagerPrivate::teardown()
{
    if (!m_initialized)
        return;

    m_initialized = false;

    xrDestroySpace(m_handGripSpace[0]);
    xrDestroySpace(m_handGripSpace[1]);
    xrDestroySpace(m_handAimSpace[0]);
    xrDestroySpace(m_handAimSpace[1]);

    destroyActions();

    if (xrDestroyHandTrackerEXT_) {
        xrDestroyHandTrackerEXT_(handTracker[Hand::LeftHand]);
        xrDestroyHandTrackerEXT_(handTracker[Hand::RightHand]);
    }

    m_instance = {XR_NULL_HANDLE};
    m_session = {XR_NULL_HANDLE};
}

QQuick3DXrInputManagerPrivate *QQuick3DXrInputManagerPrivate::get(QQuick3DXrInputManager *inputManager)
{
    QSSG_ASSERT(inputManager != nullptr, return nullptr);
    return inputManager->d_func();
}

void QQuick3DXrInputManagerPrivate::pollActions()
{
    if (!m_initialized)
        return;

    // Sync Actions
    const XrActiveActionSet activeActionSet{m_actionSet, XR_NULL_PATH};
    XrActionsSyncInfo syncInfo{};
    syncInfo.type = XR_TYPE_ACTIONS_SYNC_INFO;
    syncInfo.countActiveActionSets = 1;
    syncInfo.activeActionSets = &activeActionSet;
    XrResult result = xrSyncActions(m_session, &syncInfo);
    if (!(result == XR_SUCCESS ||
          result == XR_SESSION_LOSS_PENDING ||
          result == XR_SESSION_NOT_FOCUSED))
    {
        if (!checkXrResult(result)) {
            qWarning("xrSyncActions failed");
            return;
        }
    }

    // Hands
    XrActionStateGetInfo getInfo{};
    getInfo.type = XR_TYPE_ACTION_STATE_GET_INFO;
    for (auto hand : {Hand::LeftHand, Hand::RightHand}) {

        getInfo.subactionPath = m_handSubactionPath[hand];
        auto &inputState = m_handInputState[hand];

        for (const auto &def : m_handInputActionDefs) {
            getInfo.action = m_inputActions[def.id];
            switch (def.type) {
            case XR_ACTION_TYPE_BOOLEAN_INPUT: {
                XrActionStateBoolean boolValue{};
                boolValue.type = XR_TYPE_ACTION_STATE_BOOLEAN;
                if (checkXrResult(xrGetActionStateBoolean(m_session, &getInfo, &boolValue))) {
                    if (boolValue.isActive && boolValue.changedSinceLastSync) {
                        //qDebug() << "ACTION" << i << def.shortName << bool(boolValue.currentState);
                        setInputValue(hand, def.id, def.shortName, float(boolValue.currentState));
                    }
                } else {
                    qWarning("Failed to get action state for bool hand input");
                }
                break;
            }
            case XR_ACTION_TYPE_FLOAT_INPUT: {
                XrActionStateFloat floatValue{};
                floatValue.type = XR_TYPE_ACTION_STATE_FLOAT;
                if (checkXrResult(xrGetActionStateFloat(m_session, &getInfo, &floatValue))) {
                    if (floatValue.isActive && floatValue.changedSinceLastSync) {
                        //qDebug() << "ACTION" << i << def.shortName << floatValue.currentState;
                        setInputValue(hand, def.id, def.shortName, float(floatValue.currentState));
                    }
                } else {
                    qWarning("Failed to get action state for float hand input");
                }
                break;
            }
            case XR_ACTION_TYPE_VECTOR2F_INPUT:
            case XR_ACTION_TYPE_POSE_INPUT:
            case XR_ACTION_TYPE_VIBRATION_OUTPUT:
            case XR_ACTION_TYPE_MAX_ENUM:
                break;
            }
        }

        // Get pose activity status
        getInfo.action = m_handActions.gripPoseAction;
        XrActionStatePose poseState{};
        poseState.type = XR_TYPE_ACTION_STATE_POSE;
        if (checkXrResult(xrGetActionStatePose(m_session, &getInfo, &poseState)))
            inputState->setIsActive(poseState.isActive);
        else
            qWarning("Failed to get action state pose");

    //    XrAction gripPoseAction{XR_NULL_HANDLE};
    //    XrAction aimPoseAction{XR_NULL_HANDLE};
    //    XrAction hapticAction{XR_NULL_HANDLE};

        const QList<QPointer<QQuick3DXrHapticFeedback>> hapticOutputData = QQuick3DXrActionMapper::getHapticEffects(static_cast<QQuick3DXrInputAction::Controller>(hand));

        for (auto &hapticFeedback : hapticOutputData) {
            const bool triggered = hapticFeedback->testAndClear();
            if (triggered) {
                if (auto *hapticEffect = qobject_cast<QQuick3DXrSimpleHapticEffect*>(hapticFeedback->hapticEffect())) {
                    XrHapticVibration vibration {XR_TYPE_HAPTIC_VIBRATION, nullptr, 0, 0, 0};
                    vibration.amplitude = hapticEffect->amplitude();
                    vibration.duration = hapticEffect->duration() * 1000000; // Change from milliseconds to nanoseconds
                    vibration.frequency = hapticEffect->frequency();

                    XrHapticActionInfo hapticActionInfo {XR_TYPE_HAPTIC_ACTION_INFO, nullptr, m_handActions.hapticAction, m_handSubactionPath[hand]};

                    if (!checkXrResult(xrApplyHapticFeedback(m_session, &hapticActionInfo, (const XrHapticBaseHeader*)&vibration))) {
                        qWarning("Failed to trigger haptic feedback");
                    }
                }
            }
        }
    }
}

void QQuick3DXrInputManagerPrivate::updatePoses(XrTime predictedDisplayTime, XrSpace appSpace)
{
    // Update the Hands pose

    for (auto poseSpace : {HandPoseSpace::AimPose, HandPoseSpace::GripPose}) {
        for (auto hand : {Hand::LeftHand, Hand::RightHand}) {
            if (!isPoseInUse(hand, poseSpace))
                continue;
            XrSpaceLocation spaceLocation{};
            spaceLocation.type = XR_TYPE_SPACE_LOCATION;
            XrResult res;
            res = xrLocateSpace(handSpace(hand, poseSpace), appSpace, predictedDisplayTime, &spaceLocation);
            // qDebug() << "LOCATE SPACE hand:" << hand << "res" << res << "flags" << spaceLocation.locationFlags
            //          << "active" << m_handInputState[hand]->isActive()
            //          << "Pos" << spaceLocation.pose.position.x << spaceLocation.pose.position.y << spaceLocation.pose.position.z;
            m_validAimStateFromUpdatePoses[hand] = poseSpace == HandPoseSpace::AimPose
                    && XR_UNQUALIFIED_SUCCESS(res) && (spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
                    && (spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT); // ### Workaround for Quest issue with hand interaction aim pose

            if (XR_UNQUALIFIED_SUCCESS(res)) {
                if ((spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
                    (spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {

                    // Update hand transform
                    setPosePositionAndRotation(hand, poseSpace,
                                               QVector3D(spaceLocation.pose.position.x,
                                                         spaceLocation.pose.position.y,
                                                         spaceLocation.pose.position.z) * 100.0f,
                                               QQuaternion(spaceLocation.pose.orientation.w,
                                                           spaceLocation.pose.orientation.x,
                                                           spaceLocation.pose.orientation.y,
                                                           spaceLocation.pose.orientation.z));
                }
            } else {
                // Tracking loss is expected when the hand is not active so only log a message
                // if the hand is active.
                if (isHandActive(hand)) {
                    const char* handName[] = {"left", "right"};
                    qCDebug(lcQuick3DXr, "Unable to locate %s hand action space in app space: %d", handName[hand], res);
                }
            }
        }
    }
}

void QQuick3DXrInputManagerPrivate::updateHandtracking(XrTime predictedDisplayTime, XrSpace appSpace, bool aimExtensionEnabled)
{
    if (xrLocateHandJointsEXT_) {

        XrHandTrackingAimStateFB aimState[2] = {{}, {}}; // Only used when aim extension is enabled
        XrHandJointVelocitiesEXT velocities[2]{{}, {}};
        XrHandJointLocationsEXT locations[2]{{}, {}};
        XrHandJointsLocateInfoEXT locateInfo[2] = {{}, {}};

        for (auto hand : {Hand::LeftHand, Hand::RightHand}) {
            if (handTracker[hand] == XR_NULL_HANDLE)
                continue;

            aimState[hand].type = XR_TYPE_HAND_TRACKING_AIM_STATE_FB;

            velocities[hand].type = XR_TYPE_HAND_JOINT_VELOCITIES_EXT;
            velocities[hand].jointCount = XR_HAND_JOINT_COUNT_EXT;
            velocities[hand].jointVelocities = jointVelocities[hand];
            velocities[hand].next = aimExtensionEnabled ? &aimState[hand] : nullptr;

            locations[hand].type = XR_TYPE_HAND_JOINT_LOCATIONS_EXT;
            locations[hand].next = &velocities[hand];
            locations[hand].jointCount = XR_HAND_JOINT_COUNT_EXT;
            locations[hand].jointLocations = jointLocations[hand];

            locateInfo[hand].type = XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT;
            locateInfo[hand].baseSpace = appSpace;
            locateInfo[hand].time = predictedDisplayTime;
            if (!checkXrResult(xrLocateHandJointsEXT_(handTracker[hand], &locateInfo[hand], &locations[hand])))
                qWarning("Failed to locate hand joints for hand tracker");

            QList<QVector3D> jp;
            jp.reserve(XR_HAND_JOINT_COUNT_EXT);
            QList<QQuaternion> jr;
            jr.reserve(XR_HAND_JOINT_COUNT_EXT);
            for (uint i = 0; i < locations[hand].jointCount; ++i) {
                auto &pose = jointLocations[hand][i].pose;
                jp.append(OpenXRHelpers::toQVector(pose.position));
                jr.append(OpenXRHelpers::toQQuaternion(pose.orientation));
            }
            m_handInputState[hand]->setJointPositionsAndRotations(jp, jr);
            m_handInputState[hand]->setIsHandTrackingActive(locations[hand].isActive);
        }

        if (aimExtensionEnabled) {
            // Finger pinch handling
            for (auto hand : {Hand::LeftHand, Hand::RightHand}) {
                const uint state = aimState[hand].status;
                const uint oldState = m_aimStateFlags[hand];
                auto updateState = [&](const char *name, QQuick3DXrInputAction::Action id, uint flag) {
                    if ((state & flag) != (oldState & flag))
                        setInputValue(hand, id, name, float(!!(state & flag)));
                };

                updateState("index_pinch", QQuick3DXrInputAction::IndexFingerPinch, XR_HAND_TRACKING_AIM_INDEX_PINCHING_BIT_FB);
                updateState("middle_pinch", QQuick3DXrInputAction::MiddleFingerPinch, XR_HAND_TRACKING_AIM_MIDDLE_PINCHING_BIT_FB);
                updateState("ring_pinch", QQuick3DXrInputAction::RingFingerPinch, XR_HAND_TRACKING_AIM_RING_PINCHING_BIT_FB);
                updateState("little_pinch", QQuick3DXrInputAction::LittleFingerPinch, XR_HAND_TRACKING_AIM_LITTLE_PINCHING_BIT_FB);
                updateState("hand_tracking_menu_press", QQuick3DXrInputAction::HandTrackingMenuPress, XR_HAND_TRACKING_AIM_MENU_PRESSED_BIT_FB);
                m_aimStateFlags[hand] = state;
            }

            // ### Workaround for Quest issue with hand interaction aim pose
            for (auto hand : {Hand::LeftHand, Hand::RightHand}) {
                if (isPoseInUse(hand, HandPoseSpace::AimPose) && !m_validAimStateFromUpdatePoses[hand]) {
                    if ((aimState[hand].status & XR_HAND_TRACKING_AIM_VALID_BIT_FB)) {
                        setPosePositionAndRotation(hand, HandPoseSpace::AimPose,
                                                   QVector3D(aimState[hand].aimPose.position.x,
                                                        aimState[hand].aimPose.position.y,
                                                        aimState[hand].aimPose.position.z) * 100.0f,
                                                   QQuaternion(aimState[hand].aimPose.orientation.w,
                                                          aimState[hand].aimPose.orientation.x,
                                                          aimState[hand].aimPose.orientation.y,
                                                          aimState[hand].aimPose.orientation.z));
                        m_handInputState[hand]->setIsActive(true); // TODO: clean up
                    }
                }
            }
        }
    }
}

void QQuick3DXrInputManagerPrivate::setupHandTracking()
{
    OpenXRHelpers::resolveXrFunction(
        m_instance,
        "xrCreateHandTrackerEXT",
        (PFN_xrVoidFunction*)(&xrCreateHandTrackerEXT_));
    OpenXRHelpers::resolveXrFunction(
        m_instance,
        "xrDestroyHandTrackerEXT",
        (PFN_xrVoidFunction*)(&xrDestroyHandTrackerEXT_));
    OpenXRHelpers::resolveXrFunction(
        m_instance,
        "xrLocateHandJointsEXT",
        (PFN_xrVoidFunction*)(&xrLocateHandJointsEXT_));
    OpenXRHelpers::resolveXrFunction(
        m_instance,
        "xrGetHandMeshFB",
        (PFN_xrVoidFunction*)(&xrGetHandMeshFB_));

    if (xrCreateHandTrackerEXT_) {
        XrHandTrackerCreateInfoEXT createInfo{};
        createInfo.type = XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT;
        createInfo.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;
        createInfo.hand = XR_HAND_LEFT_EXT;
        if (!checkXrResult(xrCreateHandTrackerEXT_(m_session, &createInfo, &handTracker[QtQuick3DXr::LeftHand])))
            qWarning("Failed to create left hand tracker");
        createInfo.hand = XR_HAND_RIGHT_EXT;
        if (!checkXrResult(xrCreateHandTrackerEXT_(m_session, &createInfo, &handTracker[QtQuick3DXr::RightHand])))
            qWarning("Failed to create right hand tracker");
    }
    if (xrGetHandMeshFB_) {
        for (auto hand : {Hand::LeftHand, Hand::RightHand}) {
            if (queryHandMesh(hand))
                createHandModelData(hand);
        }
    }
}

bool QQuick3DXrInputManagerPrivate::queryHandMesh(Hand hand)
{
    XrHandTrackingMeshFB mesh {};
    mesh.type = XR_TYPE_HAND_TRACKING_MESH_FB;
    // Left hand
    if (!checkXrResult(xrGetHandMeshFB_(handTracker[hand], &mesh))) {
        qWarning("Failed to query hand mesh info.");
        return false;
    }

    mesh.jointCapacityInput = mesh.jointCountOutput;
    mesh.vertexCapacityInput = mesh.vertexCountOutput;
    mesh.indexCapacityInput = mesh.indexCountOutput;
    m_handMeshData[hand].vertexPositions.resize(mesh.vertexCapacityInput);
    m_handMeshData[hand].vertexNormals.resize(mesh.vertexCapacityInput);
    m_handMeshData[hand].vertexUVs.resize(mesh.vertexCapacityInput);
    m_handMeshData[hand].vertexBlendIndices.resize(mesh.vertexCapacityInput);
    m_handMeshData[hand].vertexBlendWeights.resize(mesh.vertexCapacityInput);
    m_handMeshData[hand].indices.resize(mesh.indexCapacityInput);
    mesh.jointBindPoses = m_handMeshData[hand].jointBindPoses;
    mesh.jointParents = m_handMeshData[hand].jointParents;
    mesh.jointRadii = m_handMeshData[hand].jointRadii;
    mesh.vertexPositions = m_handMeshData[hand].vertexPositions.data();
    mesh.vertexNormals = m_handMeshData[hand].vertexNormals.data();
    mesh.vertexUVs = m_handMeshData[hand].vertexUVs.data();
    mesh.vertexBlendIndices = m_handMeshData[hand].vertexBlendIndices.data();
    mesh.vertexBlendWeights = m_handMeshData[hand].vertexBlendWeights.data();
    mesh.indices = m_handMeshData[hand].indices.data();

    if (!checkXrResult(xrGetHandMeshFB_(handTracker[hand], &mesh))) {
        qWarning("Failed to get hand mesh data.");
        return false;
    }

    return true;
};

void QQuick3DXrInputManagerPrivate::setupActions()
{
    m_handInputActionDefs = {
        { QQuick3DXrInputAction::Button1Pressed, "b1_pressed", "Button 1 Pressed", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::Button1Touched, "b1_touched", "Button 1 Touched", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::Button2Pressed, "b2_pressed", "Button 2 Pressed", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::Button2Touched, "b2_touched", "Button 2 Touched", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::ButtonMenuPressed, "bmenu_pressed", "Button Menu Pressed", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::ButtonMenuTouched, "bmenu_touched", "Button Menu Touched", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::ButtonSystemPressed, "bsystem_pressed", "Button System Pressed", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::ButtonSystemTouched, "bsystem_touched", "Button System Touched", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::SqueezeValue, "squeeze_value", "Squeeze Value", XR_ACTION_TYPE_FLOAT_INPUT },
        { QQuick3DXrInputAction::SqueezeForce, "squeeze_force", "Squeeze Force", XR_ACTION_TYPE_FLOAT_INPUT },
        { QQuick3DXrInputAction::SqueezePressed, "squeeze_pressed", "Squeeze Pressed", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::TriggerValue, "trigger_value", "Trigger Value", XR_ACTION_TYPE_FLOAT_INPUT },
        { QQuick3DXrInputAction::TriggerPressed, "trigger_pressed", "Trigger Pressed", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::TriggerTouched, "trigger_touched", "Trigger Touched", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::ThumbstickX, "thumbstick_x", "Thumbstick X", XR_ACTION_TYPE_FLOAT_INPUT },
        { QQuick3DXrInputAction::ThumbstickY, "thumbstick_y", "Thumbstick Y", XR_ACTION_TYPE_FLOAT_INPUT },
        { QQuick3DXrInputAction::ThumbstickPressed, "thumbstick_pressed", "Thumbstick Pressed", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::ThumbstickTouched, "thumbstick_touched", "Thumbstick Touched", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::ThumbrestTouched, "thumbrest_touched", "Thumbrest Touched", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::TrackpadX, "trackpad_x", "Trackpad X", XR_ACTION_TYPE_FLOAT_INPUT },
        { QQuick3DXrInputAction::TrackpadY, "trackpad_y", "Trackpad Y", XR_ACTION_TYPE_FLOAT_INPUT },
        { QQuick3DXrInputAction::TrackpadForce, "trackpad_force", "Trackpad Force", XR_ACTION_TYPE_FLOAT_INPUT },
        { QQuick3DXrInputAction::TrackpadTouched, "trackpad_touched", "Trackpad Touched", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { QQuick3DXrInputAction::TrackpadPressed, "trackpad_pressed", "Trackpad Pressed", XR_ACTION_TYPE_BOOLEAN_INPUT }
    };

    // Create an action set.
    {
        XrActionSetCreateInfo actionSetInfo{};
        actionSetInfo.type = XR_TYPE_ACTION_SET_CREATE_INFO;
        strcpy(actionSetInfo.actionSetName, "gameplay");
        strcpy(actionSetInfo.localizedActionSetName, "Gameplay");
        actionSetInfo.priority = 0;
        if (!checkXrResult(xrCreateActionSet(m_instance, &actionSetInfo, &m_actionSet)))
            qWarning("Failed to create gameplay action set");
    }

    // Create Hand Actions
    setPath(m_handSubactionPath[0], "/user/hand/left");
    setPath(m_handSubactionPath[1], "/user/hand/right");

    for (const auto &def : m_handInputActionDefs) {
        createAction(def.type,
                     def.shortName,
                     def.localizedName,
                     2,
                     m_handSubactionPath,
                     m_inputActions[def.id]);
    }

    createAction(XR_ACTION_TYPE_VIBRATION_OUTPUT,
                 "vibrate_hand",
                 "Vibrate Hand",
                 2,
                 m_handSubactionPath,
                 m_handActions.hapticAction);
    createAction(XR_ACTION_TYPE_POSE_INPUT,
                 "hand_grip_pose",
                 "Hand Grip Pose",
                 2,
                 m_handSubactionPath,
                 m_handActions.gripPoseAction);
    createAction(XR_ACTION_TYPE_POSE_INPUT,
                 "hand_aim_pose",
                 "Hand Aim Pose",
                 2,
                 m_handSubactionPath,
                 m_handActions.aimPoseAction);

}

void QQuick3DXrInputManagerPrivate::destroyActions()
{
    for (auto &action : m_inputActions) {
        if (action)
            xrDestroyAction(action);
    }

    xrDestroyAction(m_handActions.gripPoseAction);
    xrDestroyAction(m_handActions.aimPoseAction);
    xrDestroyAction(m_handActions.hapticAction);

    xrDestroyActionSet(m_actionSet);
}

bool QQuick3DXrInputManagerPrivate::checkXrResult(const XrResult &result)
{
    return OpenXRHelpers::checkXrResult(result, m_instance);
}

void QQuick3DXrInputManagerPrivate::setPath(XrPath &path, const QByteArray &pathString)
{
    if (!checkXrResult(xrStringToPath(m_instance, pathString.constData(), &path)))
        qWarning("xrStringToPath failed");
}

void QQuick3DXrInputManagerPrivate::createAction(XrActionType type,
                                       const char *name,
                                       const char *localizedName,
                                       int numSubactions,
                                       XrPath *subactionPath,
                                       XrAction &action)
{
    XrActionCreateInfo actionInfo{};
    actionInfo.type = XR_TYPE_ACTION_CREATE_INFO;
    actionInfo.actionType = type;
    strcpy(actionInfo.actionName, name);
    strcpy(actionInfo.localizedActionName, localizedName);
    actionInfo.countSubactionPaths = quint32(numSubactions);
    actionInfo.subactionPaths = subactionPath;
    if (!checkXrResult(xrCreateAction(m_actionSet, &actionInfo, &action)))
        qCDebug(lcQuick3DXr) << "xrCreateAction failed. Name:" << name << "localizedName:" << localizedName;
}

void QQuick3DXrInputManagerPrivate::getBoolInputState(XrActionStateGetInfo &getInfo, const XrAction &action, std::function<void(bool)> setter)
{
    getInfo.action = action;
    XrActionStateBoolean boolValue{};
    boolValue.type = XR_TYPE_ACTION_STATE_BOOLEAN;
    if (checkXrResult(xrGetActionStateBoolean(m_session, &getInfo, &boolValue))) {
        if (boolValue.isActive == XR_TRUE)
            setter(bool(boolValue.currentState));
    } else {
        qWarning("Failed to get action state: bool");
    }
}

void QQuick3DXrInputManagerPrivate::getFloatInputState(XrActionStateGetInfo &getInfo, const XrAction &action, std::function<void(float)> setter)
{
    getInfo.action = action;
    XrActionStateFloat floatValue{};
    floatValue.type = XR_TYPE_ACTION_STATE_FLOAT;
    if (checkXrResult(xrGetActionStateFloat(m_session, &getInfo, &floatValue))) {
        if (floatValue.isActive == XR_TRUE)
            setter(float(floatValue.currentState));
    } else {
        qWarning("Failed to get action state: float");
    }
}

XrSpace QQuick3DXrInputManagerPrivate::handSpace(QQuick3DXrInputManagerPrivate::Hand hand, HandPoseSpace poseSpace)
{
    if (poseSpace == HandPoseSpace::GripPose)
        return m_handGripSpace[hand];
    else
        return m_handAimSpace[hand];
}

bool QQuick3DXrInputManagerPrivate::isHandActive(QQuick3DXrInputManagerPrivate::Hand hand)
{
    return m_handInputState[hand]->isActive();
}

bool QQuick3DXrInputManagerPrivate::isHandTrackerActive(Hand hand)
{
    return m_handInputState[hand]->isHandTrackingActive();
}

void QQuick3DXrInputManagerPrivate::setPosePositionAndRotation(Hand hand, HandPoseSpace poseSpace, const QVector3D &position, const QQuaternion &rotation)
{
    for (auto *controller : std::as_const(m_controllers)) {
        if (QtQuick3DXr::handForController(controller->controller()) == hand && QtQuick3DXr::pose_cast(controller->poseSpace()) == poseSpace) {
            controller->setPosition(position);
            controller->setRotation(rotation);
        }
    }
}

void QQuick3DXrInputManagerPrivate::setInputValue(Hand hand, int id, const char *shortName, float value)
{
    QSSG_ASSERT(hand < 2, hand = Hand::LeftHand);
    QQuick3DXrActionMapper::handleInput(QQuick3DXrInputAction::Action(id), static_cast<QQuick3DXrInputAction::Controller>(hand), shortName, value);
}

QQuick3DXrHandInput *QQuick3DXrInputManagerPrivate::leftHandInput() const
{
    return m_handInputState[Hand::LeftHand];
}

QQuick3DXrHandInput *QQuick3DXrInputManagerPrivate::rightHandInput() const
{
    return m_handInputState[Hand::RightHand];
}

static inline QMatrix4x4 transformMatrix(const QVector3D &position, const QQuaternion &rotation)
{
    QMatrix4x4 transform = QMatrix4x4{rotation.toRotationMatrix()};

    transform(0, 3) += position[0];
    transform(1, 3) += position[1];
    transform(2, 3) += position[2];

    return transform;
}

void QQuick3DXrInputManagerPrivate::setupHandModelInternal(QQuick3DXrHandModel *model, Hand hand)
{
    QQuick3DGeometry *geometry = m_handGeometryData[hand].geometry;
    if (!geometry)
        return;

    model->setGeometry(geometry);

    QQuick3DSkin *skin = new QQuick3DSkin(model);
    auto jointListProp = skin->joints();
    QList<QMatrix4x4> inverseBindPoses;
    inverseBindPoses.reserve(XR_HAND_JOINT_COUNT_EXT);

    const auto &handMeshData = m_handMeshData[hand];

    for (int i = 0; i < XR_HAND_JOINT_COUNT_EXT; ++i) {
        const auto &pose = handMeshData.jointBindPoses[i];
        const QVector3D pos = OpenXRHelpers::toQVector(pose.position);
        const QQuaternion rot = OpenXRHelpers::toQQuaternion(pose.orientation);
        inverseBindPoses.append(transformMatrix(pos, rot).inverted());
        QQuick3DNode *joint = new QQuick3DNode(model);
        joint->setPosition(pos);
        joint->setRotation(rot);
        jointListProp.append(&jointListProp, joint);
    }
    skin->setInverseBindPoses(inverseBindPoses);
    model->setSkin(skin);
}

void QQuick3DXrInputManagerPrivate::setupHandModel(QQuick3DXrHandModel *model)
{
    QSSG_ASSERT(model != nullptr, return);

    if (model->geometry() != nullptr || model->skin() != nullptr) {
        qWarning() << "Hand model already has geometry or skin set.";
        return;
    }

    auto hand = model->hand();
    if (hand == QQuick3DXrHandModel::LeftHand)
        setupHandModelInternal(model, Hand::LeftHand);
    else if (hand == QQuick3DXrHandModel::RightHand)
        setupHandModelInternal(model, Hand::RightHand);
    else
        qWarning() << "No matching hand tracker input found for hand model.";
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

void QQuick3DXrInputManagerPrivate::createHandModelData(Hand hand)
{
    const auto &handMeshData = m_handMeshData[hand];

    auto &geometry = m_handGeometryData[hand].geometry;
    delete geometry;
    geometry = createHandMeshGeometry(handMeshData);
}

QT_END_NAMESPACE
