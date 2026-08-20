// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxranchormanager_openxr_p.h"
#include "openxr/qopenxrhelpers_p.h"
#include "qquick3dxrspatialanchor_p.h"

#include <QtQuick3DUtils/private/qssgassert_p.h>

#include <QtGui/qquaternion.h>

#include <QLoggingCategory>

#if defined(Q_OS_ANDROID)
# include <QtCore/private/qandroidextras_p.h>
#endif

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcQuick3DXr);

//<uses-permission android:name="com.oculus.permission.USE_ANCHOR_API" />

static const uint32_t MAX_PERSISTENT_SPACES = 100;

static const char qssgXrRecognizedLabels[] = "TABLE,COUCH,FLOOR,CEILING,WALL_FACE,WINDOW_FRAME,DOOR_FRAME,STORAGE,BED,SCREEN,LAMP,PLANT,WALL_ART,OTHER";

[[nodiscard]] static QQuick3DXrSpatialAnchor::Classification getLabelForString(const QString &label)
{
    if (label == QStringLiteral("TABLE"))
        return QQuick3DXrSpatialAnchor::Classification::Table;
    if (label == QStringLiteral("COUCH"))
        return QQuick3DXrSpatialAnchor::Classification::Seat;
    if (label == QStringLiteral("FLOOR"))
        return QQuick3DXrSpatialAnchor::Classification::Floor;
    if (label == QStringLiteral("CEILING"))
        return QQuick3DXrSpatialAnchor::Classification::Ceiling;
    if (label == QStringLiteral("WALL_FACE"))
        return QQuick3DXrSpatialAnchor::Classification::Wall;
    if (label == QStringLiteral("WINDOW_FRAME"))
        return QQuick3DXrSpatialAnchor::Classification::Window;
    if (label == QStringLiteral("DOOR_FRAME"))
        return QQuick3DXrSpatialAnchor::Classification::Door;

    return QQuick3DXrSpatialAnchor::Classification::Other;
}


QQuick3DXrAnchorManager::QQuick3DXrAnchorManager()
{

}

QQuick3DXrAnchorManager::~QQuick3DXrAnchorManager()
{

}

QQuick3DXrAnchorManager *QQuick3DXrAnchorManager::instance()
{
    static QQuick3DXrAnchorManager instance;
    return &instance;
}

void QQuick3DXrAnchorManager::initialize(XrInstance instance, XrSession session)
{
#if defined(Q_OS_ANDROID)
    auto res = QtAndroidPrivate::requestPermission(QLatin1StringView("com.oculus.permission.USE_SCENE"));
    res.waitForFinished();
#endif

    m_instance = instance;
    m_session = session;

    // Get the function pointers
    OpenXRHelpers::resolveXrFunction(
                    m_instance,
                    "xrEnumerateSpaceSupportedComponentsFB",
                    (PFN_xrVoidFunction*)(&xrEnumerateSpaceSupportedComponentsFB));
    OpenXRHelpers::resolveXrFunction(
                    m_instance,
                    "xrGetSpaceComponentStatusFB",
                    (PFN_xrVoidFunction*)(&xrGetSpaceComponentStatusFB));
    OpenXRHelpers::resolveXrFunction(
                    m_instance,
                    "xrSetSpaceComponentStatusFB",
                    (PFN_xrVoidFunction*)(&xrSetSpaceComponentStatusFB));
    OpenXRHelpers::resolveXrFunction(
                    m_instance,
                    "xrGetSpaceUuidFB",
                    (PFN_xrVoidFunction*)(&xrGetSpaceUuidFB));
    OpenXRHelpers::resolveXrFunction(
                    m_instance,
                    "xrQuerySpacesFB",
                    (PFN_xrVoidFunction*)(&xrQuerySpacesFB));
    OpenXRHelpers::resolveXrFunction(
                    m_instance,
                    "xrRetrieveSpaceQueryResultsFB",
                    (PFN_xrVoidFunction*)(&xrRetrieveSpaceQueryResultsFB));
    OpenXRHelpers::resolveXrFunction(
                    m_instance,
                    "xrGetSpaceBoundingBox2DFB",
                    (PFN_xrVoidFunction*)(&xrGetSpaceBoundingBox2DFB));
    OpenXRHelpers::resolveXrFunction(
                    m_instance,
                    "xrGetSpaceBoundingBox3DFB",
                    (PFN_xrVoidFunction*)(&xrGetSpaceBoundingBox3DFB));
    OpenXRHelpers::resolveXrFunction(
                    m_instance,
                    "xrGetSpaceSemanticLabelsFB",
                    (PFN_xrVoidFunction*)(&xrGetSpaceSemanticLabelsFB));
    OpenXRHelpers::resolveXrFunction(
                    m_instance,
                    "xrGetSpaceBoundary2DFB",
                    (PFN_xrVoidFunction*)(&xrGetSpaceBoundary2DFB));
    OpenXRHelpers::resolveXrFunction(
                    m_instance,
                    "xrGetSpaceRoomLayoutFB",
                    (PFN_xrVoidFunction*)(&xrGetSpaceRoomLayoutFB));
    OpenXRHelpers::resolveXrFunction(
                    m_instance,
                    "xrGetSpaceContainerFB",
                    (PFN_xrVoidFunction*)(&xrGetSpaceContainerFB));
    OpenXRHelpers::resolveXrFunction(
                    m_instance,
                    "xrRequestSceneCaptureFB",
                    (PFN_xrVoidFunction*)(&xrRequestSceneCaptureFB));
}

void QQuick3DXrAnchorManager::teardown()
{

}

QList<const char *> QQuick3DXrAnchorManager::requiredExtensions() const
{
    return {
        // XR_EXT_UUID_EXTENSION_NAME, // ### Crashes on Quest 3. Theoretically required for XR_FB_spatial_entity (would work anyway, but is a validation error)
        XR_FB_SPATIAL_ENTITY_EXTENSION_NAME,
        XR_FB_SPATIAL_ENTITY_QUERY_EXTENSION_NAME,
        XR_FB_SPATIAL_ENTITY_STORAGE_EXTENSION_NAME,
        XR_FB_SPATIAL_ENTITY_CONTAINER_EXTENSION_NAME,
        XR_FB_SCENE_EXTENSION_NAME,
#ifdef Q_OS_ANDROID // no scene capture with Simulator as of version 57
        XR_FB_SCENE_CAPTURE_EXTENSION_NAME
#endif
    };
}

void QQuick3DXrAnchorManager::handleEvent(const XrEventDataBaseHeader *event)
{
    if (event->type == XR_TYPE_EVENT_DATA_SPACE_SET_STATUS_COMPLETE_FB) {
        qCDebug(lcQuick3DXr, "QQuick3DXrAnchorManager::handleEvent: received XR_TYPE_EVENT_DATA_SPACE_SET_STATUS_COMPLETE_FB");
        const XrEventDataSpaceSetStatusCompleteFB* setStatusComplete = (const XrEventDataSpaceSetStatusCompleteFB*)(event);
        if (setStatusComplete->result == XR_SUCCESS) {
            if (setStatusComplete->componentType == XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB) {
                addAnchor(setStatusComplete->space, setStatusComplete->uuid);
            }
        }
    } else if (event->type == XR_TYPE_EVENT_DATA_SCENE_CAPTURE_COMPLETE_FB) {
        qCDebug(lcQuick3DXr, "QQuick3DXrAnchorManager::handleEvent: received XR_TYPE_EVENT_DATA_SCENE_CAPTURE_COMPLETE_FB");
        const XrEventDataSceneCaptureCompleteFB* captureResult = (const XrEventDataSceneCaptureCompleteFB*)(event);
        if (captureResult->result == XR_SUCCESS) {
            Q_EMIT sceneCaptureCompleted();
            qCDebug(lcQuick3DXr,
                "QQuick3DXrAnchorManager::handleEvent: Scene capture (ID = %llu) succeeded",
                static_cast<long long unsigned int>(captureResult->requestId));
        } else {
            qCDebug(lcQuick3DXr,
                "QQuick3DXrAnchorManager::handleEvent: Scene capture (ID = %llu) failed with an error %d",
                static_cast<long long unsigned int>(captureResult->requestId),
                captureResult->result);
        }

    } else if (event->type == XR_TYPE_EVENT_DATA_SPACE_QUERY_RESULTS_AVAILABLE_FB) {
        qCDebug(lcQuick3DXr, "QQuick3DXrAnchorManager::handleEvent: received XR_TYPE_EVENT_DATA_SPACE_QUERY_RESULTS_AVAILABLE_FB");
        const XrEventDataSpaceQueryResultsAvailableFB* resultsAvailable = (const XrEventDataSpaceQueryResultsAvailableFB*)(event);

        XrSpaceQueryResultsFB queryResults{};
        queryResults.type = XR_TYPE_SPACE_QUERY_RESULTS_FB;
        queryResults.resultCapacityInput = 0;
        queryResults.resultCountOutput = 0;
        queryResults.results = nullptr;

        if (!checkXrResult(retrieveSpaceQueryResults(resultsAvailable->requestId, &queryResults))) {
            qWarning("Failed to retrieve space query results");
            return;
        }

        QVector<XrSpaceQueryResultFB> results(queryResults.resultCountOutput);
        queryResults.resultCapacityInput = results.size();
        queryResults.resultCountOutput = 0;
        queryResults.results = results.data();

        if (!checkXrResult(retrieveSpaceQueryResults(resultsAvailable->requestId, &queryResults))) {
            qWarning("Failed to retrieve space query results");
            return;
        }

        qCDebug(lcQuick3DXr, "retrieveSpaceQueryResults: num of results received: %d", queryResults.resultCountOutput);
        for (const auto &result : results) {
            if (isComponentSupported(result.space, XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB)) {
                XrSpaceComponentStatusSetInfoFB request = {
                                                            XR_TYPE_SPACE_COMPONENT_STATUS_SET_INFO_FB,
                                                            nullptr,
                                                            XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB,
                                                            true,
                                                            0
                };
                XrAsyncRequestIdFB requestId;
                XrResult res = setSpaceComponentStatus(result.space, &request, &requestId);
                if (res == XR_ERROR_SPACE_COMPONENT_STATUS_ALREADY_SET_FB) {
                    addAnchor(result.space, result.uuid);
                }
            }
        }
    } else if (event->type == XR_TYPE_EVENT_DATA_SPACE_QUERY_COMPLETE_FB) {
        qCDebug(lcQuick3DXr, "QQuick3DXrAnchorManager::handleEvent: received XR_TYPE_EVENT_DATA_SPACE_QUERY_COMPLETE_FB");
    }
}

void QQuick3DXrAnchorManager::requestSceneCapture()
{
    XrAsyncRequestIdFB requestId;
    XrSceneCaptureRequestInfoFB request{};
    request.type = XR_TYPE_SCENE_CAPTURE_REQUEST_INFO_FB;
    request.requestByteCount = 0;
    request.request = nullptr;
    if (!checkXrResult(requestSceneCapture(&request, &requestId)))
        qWarning("Failed to request scene capture");
}

bool QQuick3DXrAnchorManager::isComponentSupported(XrSpace space, XrSpaceComponentTypeFB type)
{
    uint32_t numComponents = 0;
    if (!checkXrResult(enumerateSpaceSupportedComponents(space, 0, &numComponents, nullptr))) {
        qWarning("Failed to enumerate supported space components");
        return false;
    }

    QVector<XrSpaceComponentTypeFB> components(numComponents);
    if (!checkXrResult(enumerateSpaceSupportedComponents(space, numComponents, &numComponents, components.data()))) {
        qWarning("Failed to enumerate supported space components");
        return false;
    }

    bool supported = false;
    for (const auto &component : components) {
        if (component == type) {
            supported = true;
            break;
        }
    }

    return supported;
}

bool QQuick3DXrAnchorManager::isComponentEnabled(XrSpace space, XrSpaceComponentTypeFB type)
{
    XrSpaceComponentStatusFB status = {XR_TYPE_SPACE_COMPONENT_STATUS_FB, nullptr, 0, 0};
    if (!checkXrResult(getSpaceComponentStatus(space, type, &status))) {
        qWarning("Failed to get space component status");
        return false;
    }
    return (status.enabled && !status.changePending);
}

bool QQuick3DXrAnchorManager::getBoundingBox2D(XrSpace space, QVector2D &offset, QVector2D &extent)
{
    if (isComponentEnabled(space, XR_SPACE_COMPONENT_TYPE_BOUNDED_2D_FB)) {
        // Get the 2D bounds
        XrRect2Df boundingBox2D;
        if (!checkXrResult(getSpaceBoundingBox2D(space, &boundingBox2D))) {
            qWarning("Failed to get bounding box 2D for space");
            return false;
        }
        offset = QVector2D(boundingBox2D.offset.x, boundingBox2D.offset.y);
        extent = QVector2D(boundingBox2D.extent.width, boundingBox2D.extent.height);
        return true;
    }
    return false;
}

bool QQuick3DXrAnchorManager::getBoundingBox3D(XrSpace space, QVector3D &offset, QVector3D &extent)
{
    if (isComponentEnabled(space, XR_SPACE_COMPONENT_TYPE_BOUNDED_3D_FB)) {
        // Get the 3D bounds
        XrRect3DfFB boundingBox3D;
        if (!checkXrResult(getSpaceBoundingBox3D(space, &boundingBox3D))) {
            qWarning("Failed to get bounding box 3D for space");
            return false;
        }
        offset = QVector3D(boundingBox3D.offset.x, boundingBox3D.offset.y, boundingBox3D.offset.z);
        extent = QVector3D(boundingBox3D.extent.width, boundingBox3D.extent.height, boundingBox3D.extent.depth);
        return true;
    }

    return false;
}

bool QQuick3DXrAnchorManager::setupSpatialAnchor(XrSpace space, QQuick3DXrSpatialAnchor &anchor)
{
    QVector2D extent2D;
    QVector2D offset2D;

    QVector3D extent3D;
    QVector3D offset3D;

    QSSG_ASSERT(space != XR_NULL_HANDLE, return false);

    const bool m_has2DBounds = getBoundingBox2D(space, offset2D, extent2D);
    const bool m_has3DBounds = getBoundingBox3D(space, offset3D, extent3D);
    if (isComponentSupported(space, XR_SPACE_COMPONENT_TYPE_SPACE_CONTAINER_FB) &&
        isComponentEnabled(space, XR_SPACE_COMPONENT_TYPE_SPACE_CONTAINER_FB)) {
        // Get the space container UUIDs
        anchor.setSpaceContainerUuids(collectSpaceContainerUuids(space));
    } else if (isComponentSupported(space, XR_SPACE_COMPONENT_TYPE_ROOM_LAYOUT_FB) &&
               isComponentEnabled(space, XR_SPACE_COMPONENT_TYPE_ROOM_LAYOUT_FB)) {
        anchor.setRoomLayoutUuids(collectRoomLayoutUuids(space));
    }

    if (m_has2DBounds)
        anchor.setBounds2D(offset2D, extent2D);
    if (m_has3DBounds)
        anchor.setBounds3D(offset3D, extent3D);

    auto stringLabel = getSemanticLabels(space);
    auto semanticLable = getLabelForString(stringLabel);

    anchor.setClassification(semanticLable);
    anchor.setClassificationString(stringLabel);

    return true;
}

namespace {
bool isValidUuid(const XrUuidEXT& uuid) {
    // The best, and reasonable way we can say if a uuid is valid, is to check if it's not null.
    // Anyting more then that is outside the scope of this function (There's no real way to check if a uuid is valid
    // that makese sense here anyways).
    return !QtQuick3DXr::isNullUuid(uuid.data);
}

QUuid fromXrUuidExt(XrUuidEXT uuid) {
    return QUuid::fromBytes(uuid.data);
}

XrUuidEXT fromQUuid(QUuid uuid) {
    XrUuidEXT xrUuid;
    auto bytes = uuid.toBytes();
    memcpy(xrUuid.data, bytes.data, XR_UUID_SIZE_EXT);
    return xrUuid;
}

}

QSet<QUuid> QQuick3DXrAnchorManager::collectRoomLayoutUuids(XrSpace space)
{
    XrRoomLayoutFB roomLayout{};
    roomLayout.type = XR_TYPE_ROOM_LAYOUT_FB;
    QVector<XrUuidEXT> wallUuids;
    QSet<QUuid> uuidSet;

    // First call
    if (!checkXrResult(getSpaceRoomLayout(space, &roomLayout))) {
        qWarning("Failed to get room layout");
        return uuidSet;
    }

    // If wallUuidCountOutput == 0, no walls in the component. The UUIDs of the ceiling and floor
    // has been returned if available
    if (roomLayout.wallUuidCountOutput != 0) {
        // Second call
        wallUuids.resize(roomLayout.wallUuidCountOutput);
        roomLayout.wallUuidCapacityInput = wallUuids.size();
        roomLayout.wallUuids = wallUuids.data();
        if (!checkXrResult(getSpaceRoomLayout(space, &roomLayout))) {
            qWarning("Failed to get room layout");
            return uuidSet;
        }
    }
    if (isValidUuid(roomLayout.floorUuid))
        uuidSet.insert(fromXrUuidExt(roomLayout.floorUuid));
    if (isValidUuid(roomLayout.ceilingUuid))
        uuidSet.insert(fromXrUuidExt(roomLayout.ceilingUuid));
    for (uint32_t i = 0; i < roomLayout.wallUuidCountOutput; i++)
        uuidSet.insert(fromXrUuidExt(roomLayout.wallUuids[i]));
    return uuidSet;
}

QSet<QUuid> QQuick3DXrAnchorManager::collectSpaceContainerUuids(XrSpace space)
{
    XrSpaceContainerFB spaceContainer{};
    spaceContainer.type = XR_TYPE_SPACE_CONTAINER_FB;
    QSet<QUuid> uuidSet;
    // First call
    if (!checkXrResult(getSpaceContainer(space, &spaceContainer))) {
        qWarning("Failed to get container");
        return uuidSet;
    }
    if (spaceContainer.uuidCountOutput != 0) {
        // Second call
        QVector<XrUuidEXT> uuids(spaceContainer.uuidCountOutput);
        spaceContainer.uuidCapacityInput = uuids.size();
        spaceContainer.uuids = uuids.data();
        if (!checkXrResult(getSpaceContainer(space, &spaceContainer))) {
            qWarning("Failed to get container");
            return uuidSet;
        }

        for (uint32_t i = 0; i < spaceContainer.uuidCountOutput; i++)
            uuidSet.insert(fromXrUuidExt(spaceContainer.uuids[i]));
    }

    return uuidSet;
}

QString QQuick3DXrAnchorManager::getSemanticLabels(const XrSpace space) {
    const XrSemanticLabelsSupportInfoFB semanticLabelsSupportInfo = {
                                                                     XR_TYPE_SEMANTIC_LABELS_SUPPORT_INFO_FB,
                                                                     nullptr,
                                                                     XR_SEMANTIC_LABELS_SUPPORT_MULTIPLE_SEMANTIC_LABELS_BIT_FB | XR_SEMANTIC_LABELS_SUPPORT_ACCEPT_DESK_TO_TABLE_MIGRATION_BIT_FB,
                                                                     qssgXrRecognizedLabels
    };

    XrSemanticLabelsFB labels{};
    labels.type = XR_TYPE_SEMANTIC_LABELS_FB;
    labels.next = &semanticLabelsSupportInfo;

    if (!isComponentEnabled(space, XR_SPACE_COMPONENT_TYPE_SEMANTIC_LABELS_FB))
        return QString();

    // First call.
    if (!checkXrResult(getSpaceSemanticLabels(space, &labels))) {
        qWarning("Failed to get semantic labels");
        return {};
    }
    // Second call
    QByteArray labelData(labels.bufferCountOutput, Qt::Uninitialized);
    labels.bufferCapacityInput = labelData.size();
    labels.buffer = labelData.data();
    if (!checkXrResult(getSpaceSemanticLabels(space, &labels))) {
        qWarning("Failed to get semantic labels");
        return {};
    }

    return QString::fromLocal8Bit(labelData);
}

bool QQuick3DXrAnchorManager::queryAllAnchors() {
    XrSpaceQueryInfoFB queryInfo = {
                                    XR_TYPE_SPACE_QUERY_INFO_FB,
                                    nullptr,
                                    XR_SPACE_QUERY_ACTION_LOAD_FB,
                                    MAX_PERSISTENT_SPACES,
                                    0,
                                    nullptr,
                                    nullptr};

    XrAsyncRequestIdFB requestId;
    return checkXrResult(querySpaces((XrSpaceQueryInfoBaseHeaderFB*)&queryInfo, &requestId));
}

const QList<QQuick3DXrSpatialAnchor *> &QQuick3DXrAnchorManager::anchors() const
{
    return m_anchors;
}

qsizetype QQuick3DXrAnchorManager::anchorCount() const
{
    return m_anchors.count();
}

void QQuick3DXrAnchorManager::updateAnchors(XrTime predictedDisplayTime, XrSpace appSpace)
{
    // basically for each anchor, we need to call xrLocateSpace
    for (auto &anchor : m_anchors) {
        XrSpaceLocation spaceLocation{};
        spaceLocation.type = XR_TYPE_SPACE_LOCATION;
        XrResult res = xrLocateSpace(QtQuick3DXr::fromXrSpaceId<XrSpace>(anchor->space()), appSpace, predictedDisplayTime, &spaceLocation);
        if (XR_UNQUALIFIED_SUCCESS(res)) {
            if ((spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
                (spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {

                // Update transform
                anchor->setPosition(QVector3D(spaceLocation.pose.position.x,
                                              spaceLocation.pose.position.y,
                                              spaceLocation.pose.position.z) * 100.0f);
                anchor->setRotation(QQuaternion(spaceLocation.pose.orientation.w,
                                                spaceLocation.pose.orientation.x,
                                                spaceLocation.pose.orientation.y,
                                                spaceLocation.pose.orientation.z));
            }
        }
    }
}

bool QQuick3DXrAnchorManager::queryAllAnchorsWithSpecificComponentEnabled(const XrSpaceComponentTypeFB componentType) {
    XrSpaceStorageLocationFilterInfoFB storageLocationFilterInfo = {
                                                                        XR_TYPE_SPACE_STORAGE_LOCATION_FILTER_INFO_FB,
                                                                        nullptr,
                                                                        XR_SPACE_STORAGE_LOCATION_LOCAL_FB
                                                                    };

    XrSpaceComponentFilterInfoFB componentFilterInfo = {
                                                            XR_TYPE_SPACE_COMPONENT_FILTER_INFO_FB,
                                                            &storageLocationFilterInfo,
                                                            componentType
                                                        };

    XrSpaceQueryInfoFB queryInfo = {
                                        XR_TYPE_SPACE_QUERY_INFO_FB,
                                        nullptr,
                                        XR_SPACE_QUERY_ACTION_LOAD_FB,
                                        MAX_PERSISTENT_SPACES,
                                        0,
                                        (XrSpaceFilterInfoBaseHeaderFB*)&componentFilterInfo,
                                        nullptr
                                    };

    XrAsyncRequestIdFB requestId;
    if (!checkXrResult(querySpaces((XrSpaceQueryInfoBaseHeaderFB*)&queryInfo, &requestId))) {
        qWarning("Failed to query spaces");
        return false;
    }
    return true;
}

bool QQuick3DXrAnchorManager::queryAnchorsByUuids(const QSet<QUuid>& uuidSet) {
    if (uuidSet.isEmpty())
        return false;

    QVector<XrUuidEXT> uuidsToQuery;

    for (const auto &uuid : uuidSet) {
        XrUuidEXT xrUuid = fromQUuid(uuid);
        uuidsToQuery.append(xrUuid);
    }

    XrSpaceStorageLocationFilterInfoFB storageLocationFilterInfo = {
                                                                        XR_TYPE_SPACE_STORAGE_LOCATION_FILTER_INFO_FB,
                                                                        nullptr,
                                                                        XR_SPACE_STORAGE_LOCATION_LOCAL_FB
                                                                    };

    XrSpaceUuidFilterInfoFB uuidFilterInfo = {
                                                XR_TYPE_SPACE_UUID_FILTER_INFO_FB,
                                                &storageLocationFilterInfo,
                                                (uint32_t)uuidsToQuery.size(),
                                                uuidsToQuery.data()
                                            };

    XrSpaceQueryInfoFB queryInfo = {
                                        XR_TYPE_SPACE_QUERY_INFO_FB,
                                        nullptr,
                                        XR_SPACE_QUERY_ACTION_LOAD_FB,
                                        MAX_PERSISTENT_SPACES,
                                        0,
                                        (XrSpaceFilterInfoBaseHeaderFB*)&uuidFilterInfo,
                                        nullptr
                                    };

    XrAsyncRequestIdFB requestId;
    if (!checkXrResult(querySpaces((XrSpaceQueryInfoBaseHeaderFB*)&queryInfo, &requestId))) {
        qWarning("Failed to query spaces");
        return false;
    }
    return true;
}

void QQuick3DXrAnchorManager::addAnchor(XrSpace space, XrUuidEXT uuid)
{
    auto quuid = fromXrUuidExt(uuid);
    // Check if we already have this anchor
    if (m_anchorsByUuid.contains(quuid))
        return;

    QQuick3DXrSpatialAnchor *anchor = new QQuick3DXrSpatialAnchor(QtQuick3DXr::toXrSpaceId(space), quuid, this);
    setupSpatialAnchor(space, *anchor);
    m_anchorsByUuid.insert(quuid, anchor);
    m_anchors.append(anchor);
    Q_EMIT anchorAdded(anchor);
}


XrResult QQuick3DXrAnchorManager::enumerateSpaceSupportedComponents(XrSpace space, uint32_t componentTypeCapacityInput, uint32_t *componentTypeCountOutput, XrSpaceComponentTypeFB *componentTypes)
{
    return OpenXRHelpers::safeCall(xrEnumerateSpaceSupportedComponentsFB, space, componentTypeCapacityInput, componentTypeCountOutput, componentTypes);
}

XrResult QQuick3DXrAnchorManager::getSpaceComponentStatus(XrSpace space, XrSpaceComponentTypeFB componentType, XrSpaceComponentStatusFB *status)
{
    return OpenXRHelpers::safeCall(xrGetSpaceComponentStatusFB, space, componentType, status);
}

XrResult QQuick3DXrAnchorManager::setSpaceComponentStatus(XrSpace space, const XrSpaceComponentStatusSetInfoFB *info, XrAsyncRequestIdFB *requestId)
{
    return OpenXRHelpers::safeCall(xrSetSpaceComponentStatusFB, space, info, requestId);
}

XrResult QQuick3DXrAnchorManager::getSpaceUuid(XrSpace space, XrUuidEXT *uuid)
{
    return OpenXRHelpers::safeCall(xrGetSpaceUuidFB, space, uuid);
}

XrResult QQuick3DXrAnchorManager::querySpaces(const XrSpaceQueryInfoBaseHeaderFB *info, XrAsyncRequestIdFB *requestId)
{
    return OpenXRHelpers::safeCall(xrQuerySpacesFB, m_session, info, requestId);
}

XrResult QQuick3DXrAnchorManager::retrieveSpaceQueryResults(XrAsyncRequestIdFB requestId, XrSpaceQueryResultsFB *results)
{
    return OpenXRHelpers::safeCall(xrRetrieveSpaceQueryResultsFB, m_session, requestId, results);
}

XrResult QQuick3DXrAnchorManager::getSpaceBoundingBox2D(XrSpace space, XrRect2Df *boundingBox2DOutput)
{
    return OpenXRHelpers::safeCall(xrGetSpaceBoundingBox2DFB, m_session, space, boundingBox2DOutput);
}

XrResult QQuick3DXrAnchorManager::getSpaceBoundingBox3D(XrSpace space, XrRect3DfFB *boundingBox3DOutput)
{
    return OpenXRHelpers::safeCall(xrGetSpaceBoundingBox3DFB, m_session, space, boundingBox3DOutput);
}

XrResult QQuick3DXrAnchorManager::getSpaceSemanticLabels(XrSpace space, XrSemanticLabelsFB *semanticLabelsOutput)
{
    return OpenXRHelpers::safeCall(xrGetSpaceSemanticLabelsFB, m_session, space, semanticLabelsOutput);
}

XrResult QQuick3DXrAnchorManager::getSpaceBoundary2D(XrSpace space, XrBoundary2DFB *boundary2DOutput)
{
    return OpenXRHelpers::safeCall(xrGetSpaceBoundary2DFB, m_session, space, boundary2DOutput);
}

XrResult QQuick3DXrAnchorManager::getSpaceRoomLayout(XrSpace space, XrRoomLayoutFB *roomLayoutOutput)
{
    return OpenXRHelpers::safeCall(xrGetSpaceRoomLayoutFB, m_session, space, roomLayoutOutput);
}

XrResult QQuick3DXrAnchorManager::getSpaceContainer(XrSpace space, XrSpaceContainerFB *spaceContainerOutput)
{
    return OpenXRHelpers::safeCall(xrGetSpaceContainerFB, m_session, space, spaceContainerOutput);
}

XrResult QQuick3DXrAnchorManager::requestSceneCapture(const XrSceneCaptureRequestInfoFB *info, XrAsyncRequestIdFB *requestId)
{
    return OpenXRHelpers::safeCall(xrRequestSceneCaptureFB, m_session, info, requestId);
}

bool QQuick3DXrAnchorManager::checkXrResult(const XrResult &result)
{
    return OpenXRHelpers::checkXrResult(result, m_instance);
}

QT_END_NAMESPACE
