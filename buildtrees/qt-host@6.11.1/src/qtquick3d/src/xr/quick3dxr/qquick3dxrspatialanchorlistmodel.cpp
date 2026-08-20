// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxrspatialanchorlistmodel_p.h"
#include "qquick3dxrspatialanchor_p.h"

#if defined(Q_OS_VISIONOS)
#include "visionos/qquick3dxranchormanager_visionos_p.h"
#else
#include "openxr/qquick3dxranchormanager_openxr_p.h"
#endif

QT_BEGIN_NAMESPACE

/*!
    \qmltype XrSpatialAnchorListModel
    \inherits ListModel
    \inqmlmodule QtQuick3D.Xr
    \brief Provides a model containing spatial anchors.

    This type provides a list of spatial anchors, which are points in
    the physical world that can be tracked and associated with virtual content.

    The list contains elements that have an \c anchor property with the type \l XrSpatialAnchor.

    You can use it like this:

    \qml
    Repeater3D {
        model: XrSpatialAnchorListModel {
        }
        delegate: Node {
            required property XrSpatialAnchor anchor
            position: anchor.position
            rotation: anchor.rotation
            // Further use of anchor properties...
        }
    }
    \endqml
*/

static QString getClassificationString(QQuick3DXrSpatialAnchorListModel::ClassificationFlag classification)
{
    switch (classification) {
        case QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Wall:
            return QStringLiteral("wall");
        case QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Ceiling:
            return QStringLiteral("ceiling");
        case QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Floor:
            return QStringLiteral("floor");
        case QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Table:
            return QStringLiteral("table");
        case QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Seat:
            return QStringLiteral("seat");
        case QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Window:
            return QStringLiteral("window");
        case QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Door:
            return QStringLiteral("door");
        case QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Other:
            return QStringLiteral("other");
    }

    return {};
}

static QQuick3DXrSpatialAnchorListModel::ClassificationFlag getClassificationFlagType(QQuick3DXrSpatialAnchor::Classification classification)
{
    switch (classification) {
        case QQuick3DXrSpatialAnchor::Classification::Unknown:
            return QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Other;
        case QQuick3DXrSpatialAnchor::Classification::Wall:
            return QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Wall;
        case QQuick3DXrSpatialAnchor::Classification::Ceiling:
            return QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Ceiling;
        case QQuick3DXrSpatialAnchor::Classification::Floor:
            return QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Floor;
        case QQuick3DXrSpatialAnchor::Classification::Table:
            return QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Table;
        case QQuick3DXrSpatialAnchor::Classification::Seat:
            return QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Seat;
        case QQuick3DXrSpatialAnchor::Classification::Window:
            return QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Window;
        case QQuick3DXrSpatialAnchor::Classification::Door:
            return QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Door;
        case QQuick3DXrSpatialAnchor::Classification::Other:
            return QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Other;
    }

        Q_UNREACHABLE_RETURN(QQuick3DXrSpatialAnchorListModel::ClassificationFlag::Other);
}

QQuick3DXrSpatialAnchorListModel::QQuick3DXrSpatialAnchorListModel(QObject *parent)
    : QAbstractListModel{parent}
{
    m_anchorManager = QQuick3DXrAnchorManager::instance();
    if (m_anchorManager) {
        connect(m_anchorManager, &QQuick3DXrAnchorManager::anchorAdded, this, &QQuick3DXrSpatialAnchorListModel::handleAnchorAdded);
        connect(m_anchorManager, &QQuick3DXrAnchorManager::anchorUpdated, this, &QQuick3DXrSpatialAnchorListModel::handleAnchorUpdated);
        connect(m_anchorManager, &QQuick3DXrAnchorManager::anchorRemoved, this, &QQuick3DXrSpatialAnchorListModel::handleAnchorRemoved);
        connect(m_anchorManager, &QQuick3DXrAnchorManager::sceneCaptureCompleted, this, [this]{queryAnchors();});
        queryAnchors();
    } else {
        qWarning("SpatialAnchorModel: Failed to get anchor manager instance");
    }
}

int QQuick3DXrSpatialAnchorListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || m_anchorManager == nullptr)
        return 0;

    const auto &anchors = anchorsFiltered();

    return anchors.size();
}

QVariant QQuick3DXrSpatialAnchorListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_anchorManager == nullptr)
        return QVariant();

    const auto &anchors = anchorsFiltered();

    // check bounds
    if (index.row() < 0 || index.row() >= anchors.count())
        return QVariant();

    const auto &anchor = anchors.at(index.row());

    if (role == Anchor)
        return QVariant::fromValue(anchor);

    // shouldn't be reachable under normal circumstances.
    return QVariant();
}

QHash<int, QByteArray> QQuick3DXrSpatialAnchorListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Anchor] = "anchor";
    return roles;
}

/*!
    \qmlmethod void XrSpatialAnchorListModel::requestSceneCapture()
    \brief  This method triggers a scan to capture or update spatial data for the current environment.

    This method triggers the underlying XR system to perform a scene capture of the user's current physical environment,
    to update or refine the spatial mesh, enabling more accurate placement and tracking of spatial anchors.

    \note Some platforms do not perform this operation automatically.
    For example, on Quest 3, if the user is in a previously uncaptured space, this method will not be called automatically,
    resulting in no available anchors until a capture is manually triggered.
 */

void QQuick3DXrSpatialAnchorListModel::requestSceneCapture()
{
    if (m_anchorManager == nullptr)
        return;

    // not supported on the Simulator, this will be a no-op there
    m_anchorManager->requestSceneCapture();
}

void QQuick3DXrSpatialAnchorListModel::queryAnchors()
{
    if (m_anchorManager == nullptr)
        return;

    m_anchorManager->queryAllAnchors();
}

void QQuick3DXrSpatialAnchorListModel::handleAnchorAdded(QQuick3DXrSpatialAnchor *anchor)
{
    Q_UNUSED(anchor)
    if (matchesAnchorFilter(anchor)) {
        // Brute Force :-p
        beginResetModel();
        endResetModel();
    }
}

void QQuick3DXrSpatialAnchorListModel::handleAnchorRemoved(QUuid uuid)
{
    Q_UNUSED(uuid)
    // Brute Force :-p
    beginResetModel();
    endResetModel();
}

void QQuick3DXrSpatialAnchorListModel::handleAnchorUpdated(QQuick3DXrSpatialAnchor *anchor)
{
    Q_UNUSED(anchor)
    if (matchesAnchorFilter(anchor)) {
        // Brute Force :-p
        beginResetModel();
        endResetModel();
    }
}

/*!
    \qmlproperty enumeration XrSpatialAnchorListModel::filterMode
    \brief Specifies the filter mode for spatial anchors.

    Holds the filter mode.
    The filter mode can be one of the following:
    \value XrSpatialAnchorListModel.All Show all spatial anchors.
    \value XrSpatialAnchorListModel.Classification Show spatial anchors based on the provided classification filter flag.
    \value XrSpatialAnchorListModel.Identifier Show spatial anchors based on matching the provided Identifiers.
 */

QQuick3DXrSpatialAnchorListModel::FilterMode QQuick3DXrSpatialAnchorListModel::filterMode() const
{
    return m_filterMode;
}

void QQuick3DXrSpatialAnchorListModel::setFilterMode(FilterMode newFilterMode)
{
    if (m_filterMode == newFilterMode)
        return;
    m_filterMode = newFilterMode;

    // Make sure we reset the model when changing filter mode.
    handleAnchorRemoved({});

    emit filterModeChanged();
}

/*!
    \qmlproperty list<string> XrSpatialAnchorListModel::identifierFilter
    \brief Holds the list of identifiers for filtering spatial anchors.
 */

QStringList QQuick3DXrSpatialAnchorListModel::identifierFilter() const
{
    return m_uuids.values();
}

void QQuick3DXrSpatialAnchorListModel::setIdentifierFilter(const QStringList &filter)
{
    QSet<QString> newFilter { filter.cbegin(), filter.cend() };

    if (m_uuids == newFilter)
        return;

    m_uuids = newFilter;

    // Make sure we reset the model.
    handleAnchorRemoved({});

    emit identifierFilterChanged();
}

/*!
    \qmlproperty enumeration XrSpatialAnchorListModel::classificationFilter
    \brief  Holds the classification flag used for filtering spatial anchors.

    The ClassificationFlag filter is represented as a combination of flags:

    \value XrSpatialAnchorListModel.Wall
    \value XrSpatialAnchorListModel.Ceiling
    \value XrSpatialAnchorListModel.Floor
    \value XrSpatialAnchorListModel.Table
    \value XrSpatialAnchorListModel.Seat
    \value XrSpatialAnchorListModel.Window
    \value XrSpatialAnchorListModel.Door
    \value XrSpatialAnchorListModel.Other
 */

QQuick3DXrSpatialAnchorListModel::ClassificationFlags QQuick3DXrSpatialAnchorListModel::classificationFilter() const
{
    return m_classFilter;
}

void QQuick3DXrSpatialAnchorListModel::setClassificationFilter(ClassificationFlags newClassFilter)
{
    if (m_classFilter == newClassFilter)
        return;

    m_classFilter = newClassFilter;

    m_classStringFilter.clear();

    constexpr size_t classifcationCount = (sizeof(std::underlying_type_t<ClassificationFlag>) * 8) - 1;
    for (size_t i = 0; i < classifcationCount; ++i) {
        ClassificationFlag classification = static_cast<ClassificationFlag>(size_t(m_classFilter) & (size_t(1) << i));
        auto name = getClassificationString(classification);
        if (!name.isEmpty())
            m_classStringFilter.insert(name);
    }

    // Make sure we reset the model.
    handleAnchorRemoved({});

    emit classificationFilterChanged();
}

/*!
    \qmlproperty list<string> XrSpatialAnchorListModel::classificationStringFilter
    \brief Holds the classification strings used for filtering spatial anchors.

    If the \l filterMode is set to \c Classification, this property can be used to provide a
    list of additional classification string to filter on. These labels will then be matched against
    the same value as reported by \l {XrSpatialAnchor::classificationString} property
    of the spatial anchor.

    \note Only \l {XrSpatialAnchor}{spatial anchors} that are classified as
    \l {XrSpatialAnchor::Classification}{Other} will be checked against this filter.
 */
QStringList QQuick3DXrSpatialAnchorListModel::classificationStringFilter() const
{
    return m_classStringFilter.values();
}

void QQuick3DXrSpatialAnchorListModel::setClassificationStringFilter(const QStringList &newClassStringFilter)
{
    for (const auto &entry : newClassStringFilter) {
        if (!entry.isEmpty())
             m_classStringFilter.insert(entry.toLower());
    }

    // Make sure we reset the model.
    handleAnchorRemoved({});

    emit classificationStringFilterChanged();
}

bool QQuick3DXrSpatialAnchorListModel::matchesAnchorFilter(QQuick3DXrSpatialAnchor *anchor) const
{
    if (m_filterMode == FilterMode::Classification) {
        QString classificationString;
        const auto classification = anchor->classification();
        if (classification != QQuick3DXrSpatialAnchor::Classification::Other) {
            const auto classificationFlagType = getClassificationFlagType(classification);
            classificationString = getClassificationString(classificationFlagType);
        } else {
            // NOTE: Other can mean there is no classification or the classification is not one that
            // we have defined in our enums, so we will use the string as the user can specify any string.
            classificationString = anchor->classificationString().toLower();
        }
        return m_classStringFilter.contains(classificationString.toLower());
    } else if (m_filterMode == FilterMode::Identifier) {
        const auto uuid = anchor->identifier();
        return m_uuids.contains(uuid);
    }

    // 'All' mode
    return true;
}

QList<QQuick3DXrSpatialAnchor *> QQuick3DXrSpatialAnchorListModel::anchorsFiltered() const
{
    QList<QQuick3DXrSpatialAnchor *> anchors;
    const auto &unfilteredAnchors = m_anchorManager->anchors();
    if (m_filterMode == FilterMode::All) {
        anchors = unfilteredAnchors;
    } else {
        for (const auto &anchor : unfilteredAnchors) {
            if (matchesAnchorFilter(anchor))
                anchors.push_back(anchor);
        }
    }

    return anchors;
}

QT_END_NAMESPACE
