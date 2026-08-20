// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qquick3dxranchormanager_visionos_p.h"

#include "../qquick3dxrspatialanchor_p.h"
#include "visionos/qquick3dxrinputmanager_visionos_p.h"

#include <QtQuick3DUtils/private/qssgassert_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>


QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcQuick3DXr);

QQuick3DXrAnchorManager *QQuick3DXrAnchorManager::instance()
{
    static QQuick3DXrAnchorManager instance;
    return &instance;
}

QQuick3DXrAnchorManager::QQuick3DXrAnchorManager(QObject *parent)
    : QObject(parent)
{
}

QQuick3DXrAnchorManager::~QQuick3DXrAnchorManager()
{

}

void QQuick3DXrAnchorManager::teardown()
{

}

void QQuick3DXrAnchorManager::addAnchor(QQuick3DXrSpatialAnchor *anchor)
{
    anchor->moveToThread(qApp->thread());

    m_anchorsByUuid.insert(anchor->uuid(), anchor);

    Q_EMIT anchorAdded(anchor);
}

void QQuick3DXrAnchorManager::removeAnchor(QUuid uuid)
{
    if (QQuick3DXrSpatialAnchor *anchor = m_anchorsByUuid.take(uuid))
        anchor->deleteLater();
    else
        qWarning() << "Anchor not found for removal: " << uuid;

    Q_EMIT anchorRemoved(uuid);
}

void QQuick3DXrAnchorManager::updateAnchor(QQuick3DXrSpatialAnchor *anchor)
{
    anchor->moveToThread(qApp->thread());

    Q_EMIT anchorUpdated(anchor);
}

void QQuick3DXrAnchorManager::populateAnchorsList()
{
    m_anchors = m_anchorsByUuid.values();
}

enum class AnchorClassifcation
{
    Unknown = 0,
    NotAvailable,
    Undetermined,
    Wall,
    Ceiling,
    Floor,
    Table,
    Seat,
    Window,
    Door,
};

static constexpr size_t anchorClassificationStart = size_t(AnchorClassifcation::Wall);

struct AnchorClassificationMap {
    ar_plane_classification_t classification;
    AnchorClassifcation classificationEnum;
    QQuick3DXrSpatialAnchor::Classification label;
    const char classificationName[16];
};
static const AnchorClassificationMap anchorClassificationMap[] = {
    {ar_plane_classification_status_unknown, AnchorClassifcation::Unknown, QQuick3DXrSpatialAnchor::Classification::Unknown, "Unknown"},
    {ar_plane_classification_status_not_available, AnchorClassifcation::NotAvailable, QQuick3DXrSpatialAnchor::Classification::Unknown, "Not Available"},
    {ar_plane_classification_status_undetermined, AnchorClassifcation::Undetermined, QQuick3DXrSpatialAnchor::Classification::Unknown, "Undetermined"},
    {ar_plane_classification_wall, AnchorClassifcation::Wall, QQuick3DXrSpatialAnchor::Classification::Wall, "Wall"},
    {ar_plane_classification_ceiling, AnchorClassifcation::Ceiling, QQuick3DXrSpatialAnchor::Classification::Ceiling, "Ceiling"},
    {ar_plane_classification_floor, AnchorClassifcation::Floor, QQuick3DXrSpatialAnchor::Classification::Floor, "Floor"},
    {ar_plane_classification_table, AnchorClassifcation::Table, QQuick3DXrSpatialAnchor::Classification::Table, "Table"},
    {ar_plane_classification_seat, AnchorClassifcation::Seat, QQuick3DXrSpatialAnchor::Classification::Seat, "Seat"},
    {ar_plane_classification_window, AnchorClassifcation::Window, QQuick3DXrSpatialAnchor::Classification::Window, "Window"},
    {ar_plane_classification_door, AnchorClassifcation::Door, QQuick3DXrSpatialAnchor::Classification::Door, "Door"},
};

static const AnchorClassificationMap &getAnchorClassificationName(ar_plane_classification_t classification, bool *identified = nullptr)
{
    size_t foundIndex = 0;
    for (size_t i = 0, end = std::size(anchorClassificationMap); i != end; ++i) {
        if (anchorClassificationMap[i].classification == classification) {
            foundIndex = i;
            break;
        }
    }

    if (identified) // If the caller wants to know if the classification was found (I.e. not Unknown, Not Available, or Undetermined)
        *identified = (foundIndex >= anchorClassificationStart);

    return anchorClassificationMap[foundIndex];
}

static void updateAnchorProperties(QQuick3DXrSpatialAnchor &anchor, ar_plane_anchor_t planeAnchor)
{
    constexpr auto s_rot90X = QQuaternion{M_SQRT1_2, -M_SQRT1_2, 0, 0}; // fromEulerAngles(-90, 0, 0);

    simd_float4x4 originFromAnchorTransform = ar_anchor_get_origin_from_anchor_transform(planeAnchor);
    QMatrix4x4 transform{originFromAnchorTransform.columns[0].x, originFromAnchorTransform.columns[1].x, originFromAnchorTransform.columns[2].x, originFromAnchorTransform.columns[3].x,
                           originFromAnchorTransform.columns[0].y, originFromAnchorTransform.columns[1].y, originFromAnchorTransform.columns[2].y, originFromAnchorTransform.columns[3].y,
                           originFromAnchorTransform.columns[0].z, originFromAnchorTransform.columns[1].z, originFromAnchorTransform.columns[2].z, originFromAnchorTransform.columns[3].z,
                           0.0f, 0.0f, 0.0f, 1.0f};

    ar_plane_classification_t classification = ar_plane_anchor_get_plane_classification(planeAnchor);
    const auto &classificationEntry = getAnchorClassificationName(classification);

    if (anchor.classification() != classificationEntry.label) {
        anchor.setClassification(classificationEntry.label);
        anchor.setClassificationString(QString::fromLatin1(classificationEntry.classificationName));
    }

    ar_plane_geometry_t planeGeometry = ar_plane_anchor_get_geometry(planeAnchor);
    ar_plane_extent_t planeExtent = ar_plane_geometry_get_plane_extent(planeGeometry);

    const float width = ar_plane_extent_get_width(planeExtent);
    const float height = ar_plane_extent_get_height(planeExtent);

    // position and rotation
    QVector3D pos;
    QVector3D scale;
    QQuaternion rot;
    QSSGUtils::mat44::decompose(transform, pos, scale, rot);

    // The y-axis of the plane anchor is the plane’s normal vector.
    rot = rot * s_rot90X;

    anchor.setPosition(pos * 100.0f);
    anchor.setRotation(rot);

    QVector2D offset2D{};
    QVector2D extent2D(width, height);
    anchor.setBounds2D(offset2D, extent2D);
}

void QQuick3DXrAnchorManager::planeUpdateHandler(void *context, ar_plane_anchors_t added_anchors, ar_plane_anchors_t updated_anchors, ar_plane_anchors_t removed_anchors)
{
    QSSG_ASSERT_X(context != nullptr, "Context is null!", return);

    QQuick3DXrAnchorManager *that = reinterpret_cast<QQuick3DXrAnchorManager *>(context);
    // Lock for writes to the anchors list (Might as well use a simple mutex, the data is only modified by this handler).
    QWriteLocker locker(&that->m_anchorsLock);

    // 1. Remove
    if (removed_anchors) {
        ar_plane_anchors_enumerate_anchors_f(removed_anchors, context, [](void *context, ar_plane_anchor_t planeAnchor) -> bool {
            QSSG_ASSERT_X(QSSG_DEBUG_COND(context != nullptr), "Unexpected context!", return false);

            QQuick3DXrAnchorManager *that = reinterpret_cast<QQuick3DXrAnchorManager *>(context);

            unsigned char identifier[16] {};
            ar_anchor_get_identifier(planeAnchor, identifier);

            if (Q_UNLIKELY(QtQuick3DXr::isNullUuid(identifier))) {
                qWarning() << "Invalid UUID for anchor";
                return false;
            }

            QUuid uuid = QUuid::fromRfc4122(QByteArrayView(reinterpret_cast<const char *>(identifier), 16));
            that->removeAnchor(uuid);

            return true;
        });
    }

    // 2. Add
    if (added_anchors) {
        ar_plane_anchors_enumerate_anchors_f(added_anchors, context, [](void *context, ar_plane_anchor_t planeAnchor) -> bool {
            QSSG_ASSERT_X(QSSG_DEBUG_COND(context != nullptr), "Unexpected context!", return false);

            QQuick3DXrAnchorManager *that = reinterpret_cast<QQuick3DXrAnchorManager *>(context);

            // NOTE: The API documentation for this is severely lacking, but the swift docs have some more information
            // saying the identifier follows RFC 4122 for UUIDs, meaning the identifier is a 128-bit value (Big Endian)
            unsigned char identifier[16] {};
            ar_anchor_get_identifier(planeAnchor, identifier);

            if (Q_UNLIKELY(QtQuick3DXr::isNullUuid(identifier))) {
                qWarning() << "Invalid UUID for anchor";
                return false;
            }

            QUuid uuid = QUuid::fromRfc4122(QByteArrayView(reinterpret_cast<const char *>(identifier), 16));
            const QtQuick3DXr::XrSpaceId space = that->getCurrentSpaceId();
            QQuick3DXrSpatialAnchor *anchor = new QQuick3DXrSpatialAnchor(space, uuid);
            updateAnchorProperties(*anchor, planeAnchor);
            that->addAnchor(anchor);

            return true;
        });
    }

    // 3. Update
    if (updated_anchors) {
        ar_plane_anchors_enumerate_anchors_f(updated_anchors, context, [](void *context, ar_plane_anchor_t planeAnchor) -> bool {
            QSSG_ASSERT_X(QSSG_DEBUG_COND(context != nullptr), "Unexpected context!", return false);

            QQuick3DXrAnchorManager *that = reinterpret_cast<QQuick3DXrAnchorManager *>(context);

            unsigned char identifier[16] {};
            ar_anchor_get_identifier(planeAnchor, identifier);

            if (Q_UNLIKELY(QtQuick3DXr::isNullUuid(identifier))) {
                qWarning() << "Invalid UUID for anchor";
                return false;
            }

            QUuid uuid = QUuid::fromRfc4122(QByteArrayView(reinterpret_cast<const char *>(identifier), 16));

            QQuick3DXrSpatialAnchor *anchor = that->m_anchorsByUuid.value(uuid);
            // This does occur in some cases, so we need to create a new anchor if it's not found
            // NOTE: Ideally this should not happen, but it does, reason unknown atm.
            if (!anchor) {
                anchor = new QQuick3DXrSpatialAnchor(that->getCurrentSpaceId(), uuid);
                updateAnchorProperties(*anchor, planeAnchor);
                that->addAnchor(anchor);
            } else {
                updateAnchorProperties(*anchor, planeAnchor);
                that->updateAnchor(anchor);
            }

            return true;
        });
    }

    that->populateAnchorsList();
}

void QQuick3DXrAnchorManager::prepareAnchorManager(ar_data_providers_t dataProviders)
{
    QSSG_ASSERT_X(!m_isInitialized, "Anchor manager already initialized", return);

    m_isPlaneDetectionSupported = (m_requestedAnchorType == AnchorType::Plane) && ar_plane_detection_provider_is_supported();
    if (m_isPlaneDetectionSupported) {
        ar_plane_detection_configuration_t planeDetectionConfiguration = ar_plane_detection_configuration_create();
        m_planeDetectionProvider = ar_plane_detection_provider_create(planeDetectionConfiguration);
        ar_plane_detection_provider_set_update_handler_f(m_planeDetectionProvider, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), this, &planeUpdateHandler);
        ar_data_providers_add_data_provider(dataProviders, m_planeDetectionProvider);
    } else {
        qCWarning(lcQuick3DXr, "Plane detection is not supported on this device.");
    }
}

void QQuick3DXrAnchorManager::initAnchorManager()
{
    QSSG_ASSERT_X(!m_isInitialized, "Anchor manager already initialized", return);

    m_isInitialized = m_isPlaneDetectionSupported;
}

void QQuick3DXrAnchorManager::requestSceneCapture()
{
    // Scene is continuously captured in the background,
    // no need to request it explicitly.
}

bool QQuick3DXrAnchorManager::queryAllAnchors()
{
    return true;
}

QList<QQuick3DXrSpatialAnchor *> QQuick3DXrAnchorManager::anchors() const
{
    QReadLocker locker(&m_anchorsLock);
    return m_anchors;
}

qsizetype QQuick3DXrAnchorManager::anchorCount() const
{
    QReadLocker locker(&m_anchorsLock);
    return m_anchors.size();
}

QT_END_NAMESPACE
