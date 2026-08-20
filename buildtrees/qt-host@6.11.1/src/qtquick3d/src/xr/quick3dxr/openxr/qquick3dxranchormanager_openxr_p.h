// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRANCHORMANAGER_OPENXR_P_H
#define QQUICK3DXRANCHORMANAGER_OPENXR_P_H

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

#include <openxr/openxr.h>

#if __has_include(<openxr/fb_spatial_entity.h>) && __has_include(<openxr/fb_scene.h>)
#include <openxr/fb_spatial_entity.h>
#if __has_include(<openxr/fb_spatial_entity_query.h>) && __has_include(<openxr/fb_spatial_entity_storage.h>) && __has_include(<openxr/fb_spatial_entity_container.h>)
#include <openxr/fb_spatial_entity_query.h>
#include <openxr/fb_spatial_entity_storage.h>
#include <openxr/fb_spatial_entity_container.h>
#endif
#include <openxr/fb_scene.h>
#if __has_include(<openxr/fb_scene_capture.h>)
#include <openxr/fb_scene_capture.h>
#endif
#endif
// Otherwise, these may be all provided in openxr.h, if it is new enough, no
// need for the Mobile SDK specific headers anymore. If that's not the case
// (too old openxr.h), compilation will fail.

#include <QList>
#include <QSet>
#include <QUuid>

#include <QObject>

#include <QtQuick3DXr/private/qtquick3dxrglobal_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DXrSpatialAnchor;

class QQuick3DXrAnchorManager : public QObject
{
    Q_OBJECT
public:
    static QQuick3DXrAnchorManager* instance();

    void initialize(XrInstance instance, XrSession session);
    void teardown();

    QList<const char*> requiredExtensions() const;

    void handleEvent(const XrEventDataBaseHeader* event);

    void requestSceneCapture();
    bool queryAllAnchors();
    const QList<QQuick3DXrSpatialAnchor *> &anchors() const;
    qsizetype anchorCount() const;

    void updateAnchors(XrTime predictedDisplayTime, XrSpace appSpace);
    QString getSemanticLabels(const XrSpace space);
    QSet<QUuid> collectRoomLayoutUuids(XrSpace space);
    QSet<QUuid> collectSpaceContainerUuids(XrSpace space);

    bool isComponentSupported(XrSpace space, XrSpaceComponentTypeFB type);
    bool isComponentEnabled(XrSpace space, XrSpaceComponentTypeFB type);

    bool getBoundingBox3D(XrSpace space, QVector3D &offset, QVector3D &extent);
    bool getBoundingBox2D(XrSpace space, QVector2D &offset, QVector2D &extent);

    bool setupSpatialAnchor(XrSpace space, QQuick3DXrSpatialAnchor &anchor);

Q_SIGNALS:
    void anchorAdded(QQuick3DXrSpatialAnchor* anchor);
    void anchorRemoved(QUuid uuid);
    void anchorUpdated(QQuick3DXrSpatialAnchor *anchor);
    void sceneCaptureCompleted();

private:
    QQuick3DXrAnchorManager();
    ~QQuick3DXrAnchorManager();

    XrInstance m_instance{XR_NULL_HANDLE};
    XrSession m_session{XR_NULL_HANDLE};

    [[nodiscard]] bool checkXrResult(const XrResult &result);

    bool queryAllAnchorsWithSpecificComponentEnabled(const XrSpaceComponentTypeFB componentType);
    bool queryAnchorsByUuids(const QSet<QUuid>& uuidSet);

    void addAnchor(XrSpace space, XrUuidEXT uuid);

    // API Wrappers
    XrResult enumerateSpaceSupportedComponents(XrSpace space,
                                               uint32_t componentTypeCapacityInput,
                                               uint32_t* componentTypeCountOutput,
                                               XrSpaceComponentTypeFB* componentTypes);
    XrResult getSpaceComponentStatus(XrSpace space,
                                     XrSpaceComponentTypeFB componentType,
                                     XrSpaceComponentStatusFB* status);
    XrResult setSpaceComponentStatus(XrSpace space,
                                     const XrSpaceComponentStatusSetInfoFB* info,
                                     XrAsyncRequestIdFB* requestId);
    XrResult getSpaceUuid(XrSpace space, XrUuidEXT *uuid);
    XrResult querySpaces(const XrSpaceQueryInfoBaseHeaderFB* info, XrAsyncRequestIdFB* requestId);
    XrResult retrieveSpaceQueryResults(XrAsyncRequestIdFB requestId, XrSpaceQueryResultsFB* results);
    XrResult getSpaceBoundingBox2D(XrSpace space, XrRect2Df* boundingBox2DOutput);
    XrResult getSpaceBoundingBox3D(XrSpace space, XrRect3DfFB* boundingBox3DOutput);
    XrResult getSpaceSemanticLabels(XrSpace space, XrSemanticLabelsFB* semanticLabelsOutput);
    XrResult getSpaceBoundary2D(XrSpace space, XrBoundary2DFB* boundary2DOutput);
    XrResult getSpaceRoomLayout(XrSpace space, XrRoomLayoutFB* roomLayoutOutput);
    XrResult getSpaceContainer(XrSpace space, XrSpaceContainerFB* spaceContainerOutput);
    XrResult requestSceneCapture(const XrSceneCaptureRequestInfoFB* info, XrAsyncRequestIdFB* requestId);

    PFN_xrEnumerateSpaceSupportedComponentsFB xrEnumerateSpaceSupportedComponentsFB = nullptr;
    PFN_xrGetSpaceComponentStatusFB xrGetSpaceComponentStatusFB = nullptr;
    PFN_xrSetSpaceComponentStatusFB xrSetSpaceComponentStatusFB = nullptr;
    PFN_xrGetSpaceUuidFB xrGetSpaceUuidFB = nullptr;
    PFN_xrQuerySpacesFB xrQuerySpacesFB = nullptr;
    PFN_xrRetrieveSpaceQueryResultsFB xrRetrieveSpaceQueryResultsFB = nullptr;
    PFN_xrGetSpaceBoundingBox2DFB xrGetSpaceBoundingBox2DFB = nullptr;
    PFN_xrGetSpaceBoundingBox3DFB xrGetSpaceBoundingBox3DFB = nullptr;
    PFN_xrGetSpaceSemanticLabelsFB xrGetSpaceSemanticLabelsFB = nullptr;
    PFN_xrGetSpaceBoundary2DFB xrGetSpaceBoundary2DFB = nullptr;
    PFN_xrGetSpaceRoomLayoutFB xrGetSpaceRoomLayoutFB = nullptr;
    PFN_xrGetSpaceContainerFB xrGetSpaceContainerFB = nullptr;
    PFN_xrRequestSceneCaptureFB xrRequestSceneCaptureFB = nullptr;

    QList<QQuick3DXrSpatialAnchor *> m_anchors;
    QHash<QUuid,QQuick3DXrSpatialAnchor *> m_anchorsByUuid;
};

QT_END_NAMESPACE

#endif // QQUICK3DXRANCHORMANAGER_OPENXR_P_H
