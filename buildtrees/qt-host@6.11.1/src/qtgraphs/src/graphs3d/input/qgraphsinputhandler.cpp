// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qgraphsinputhandler_p.h"

#include <QtQuick/private/qquickdraghandler_p.h>
#include <QtQuick/private/qquickpinchhandler_p.h>
#include <QtQuick/private/qquicktaphandler_p.h>
#include <QtQuick/private/qquickwheelhandler_p.h>

#include "qquickgraphsitem_p.h"
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

QGraphsInputHandler::QGraphsInputHandler(QQuickItem *parent)
    : QQuickItem(parent)
    , m_zoomEnabled(true)
    , m_zoomAtTarget(true)
    , m_rotationEnabled(true)
    , m_selectionEnabled(true)
    , m_pendingPoint(QPoint())
    , m_pinchDiff(.0f)
    , m_graphsItem(nullptr)
{
    m_pinchHandler = new QQuickPinchHandler(this);
    m_tapHandler = new QQuickTapHandler(this);

    // This is to support QQuickGraphsItem's mouseMove signal.
    setAcceptHoverEvents(true);
    m_dragHandler = new QQuickDragHandler(this);
    m_wheelHandler = new QQuickWheelHandler(this);
    m_dragHandler->setAcceptedButtons(Qt::MouseButton::RightButton);
    m_wheelHandler->setAcceptedDevices(QInputDevice::DeviceType::Mouse
                                       | QInputDevice::DeviceType::TouchPad);
    QObject::connect(m_tapHandler, &QQuickTapHandler::tapped, this, &QGraphsInputHandler::onTapped);
    QObject::connect(m_dragHandler,
                     &QQuickDragHandler::translationChanged,
                     this,
                     &QGraphsInputHandler::onTranslationChanged);
    QObject::connect(m_dragHandler,
                     &QQuickDragHandler::grabChanged,
                     this,
                     &QGraphsInputHandler::onGrabChanged);
    QObject::connect(m_wheelHandler,
                     &QQuickWheelHandler::wheel,
                     this,
                     &QGraphsInputHandler::onWheel);
    QObject::connect(m_pinchHandler,
                     &QQuickPinchHandler::scaleChanged,
                     this,
                     &QGraphsInputHandler::onPinchScaleChanged);
    QObject::connect(m_pinchHandler,
                     &QQuickPinchHandler::grabChanged,
                     this,
                     &QGraphsInputHandler::onGrabChanged);
}

QGraphsInputHandler::~QGraphsInputHandler() {}

void QGraphsInputHandler::setZoomEnabled(bool enable)
{
    if (m_zoomEnabled == enable) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enable;
        return;
    }
    m_zoomEnabled = enable;
    if (m_graphsItem)
        emit m_graphsItem->zoomEnabledChanged(enable);
}

bool QGraphsInputHandler::isZoomEnabled()
{
    return m_zoomEnabled;
}

void QGraphsInputHandler::setZoomAtTargetEnabled(bool enable)
{
    if (m_zoomAtTarget == enable) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enable;
        return;
    }
    m_zoomAtTarget = enable;
    if (m_graphsItem)
        emit m_graphsItem->zoomAtTargetEnabledChanged(enable);
}

bool QGraphsInputHandler::isZoomAtTargetEnabled()
{
    return m_zoomAtTarget;
}

void QGraphsInputHandler::setRotationEnabled(bool enable)
{
    if (m_rotationEnabled == enable) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enable;
    }
    m_rotationEnabled = enable;
    if (m_graphsItem)
        emit m_graphsItem->rotationEnabledChanged(enable);
}

bool QGraphsInputHandler::isRotationEnabled()
{
    return m_rotationEnabled;
}

void QGraphsInputHandler::setSelectionEnabled(bool enable)
{
    if (m_selectionEnabled == enable) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enable;
        return;
    }
    m_selectionEnabled = enable;
    if (m_graphsItem)
        emit m_graphsItem->selectionEnabledChanged(enable);
}

bool QGraphsInputHandler::isSelectionEnabled()
{
    return m_selectionEnabled;
}

void QGraphsInputHandler::setDefaultInputHandler()
{
    setVisible(true);
}

void QGraphsInputHandler::unsetDefaultInputHandler()
{
    setVisible(false);
}

void QGraphsInputHandler::unsetDefaultTapHandler()
{
    QObject::disconnect(m_tapHandler,
                        &QQuickTapHandler::tapped,
                        this,
                        &QGraphsInputHandler::onTapped);
}

void QGraphsInputHandler::unsetDefaultDragHandler()
{
    QObject::disconnect(m_dragHandler,
                        &QQuickDragHandler::translationChanged,
                        this,
                        &QGraphsInputHandler::onTranslationChanged);
}

void QGraphsInputHandler::unsetDefaultWheelHandler()
{
    QObject::disconnect(m_wheelHandler,
                        &QQuickWheelHandler::wheel,
                        this,
                        &QGraphsInputHandler::onWheel);
}

void QGraphsInputHandler::unsetDefaultPinchHandler()
{
    QObject::disconnect(m_pinchHandler,
                        &QQuickPinchHandler::scaleChanged,
                        this,
                        &QGraphsInputHandler::onPinchScaleChanged);
}

void QGraphsInputHandler::setDragButton(Qt::MouseButtons button)
{
    m_dragHandler->setAcceptedButtons(button);
}

void QGraphsInputHandler::setGraphsItem(QQuickGraphsItem *item)
{
    m_graphsItem = item;
    QObject::connect(m_tapHandler, &QQuickTapHandler::tapped, item, &QQuickGraphsItem::tapped);
    QObject::connect(m_tapHandler,
                     &QQuickTapHandler::doubleTapped,
                     item,
                     &QQuickGraphsItem::doubleTapped);
    QObject::connect(m_tapHandler,
                     &QQuickTapHandler::longPressed,
                     item,
                     &QQuickGraphsItem::longPressed);
    QObject::connect(m_dragHandler,
                     &QQuickDragHandler::translationChanged,
                     item,
                     &QQuickGraphsItem::dragged);
    QObject::connect(m_wheelHandler, &QQuickWheelHandler::wheel, item, &QQuickGraphsItem::wheel);
    QObject::connect(m_pinchHandler,
                     &QQuickPinchHandler::scaleChanged,
                     item,
                     &QQuickGraphsItem::pinch);
    QObject::connect(this, &QGraphsInputHandler::mouseMove, item, &QQuickGraphsItem::mouseMove);
}

void QGraphsInputHandler::onTapped()
{
    if (!m_selectionEnabled)
        return;

    if (m_graphsItem->isSlicingActive()) {
        m_graphsItem->setSliceActivatedChanged(true);
        m_graphsItem->update();
        return;
    }
    m_graphsItem->doPicking(m_tapHandler->point().position());
}

void QGraphsInputHandler::onTranslationChanged(QVector2D delta)
{
    if (!m_rotationEnabled)
        return;

    if (!m_dragHandler->centroid().pressedButtons().testFlags(m_dragHandler->acceptedButtons()))
        return;

    float rotationSpeed = 1.0f;
#if !defined(Q_OS_IOS)
    rotationSpeed = 10.0f;
#endif
    QQuickGraphsItem *item = m_graphsItem;
    // Calculate mouse movement since last frame
    float xRotation = item->cameraXRotation();
    float yRotation = item->cameraYRotation();
    // Apply to rotations
    xRotation += (delta.x() / rotationSpeed);
    yRotation += (delta.y() / rotationSpeed);
    item->setCameraXRotation(xRotation);
    item->setCameraYRotation(yRotation);
}

void QGraphsInputHandler::onGrabChanged(QPointingDevice::GrabTransition transition,
                                        QEventPoint point)
{
    static QPointF pickPoint;

    if (transition == QPointingDevice::GrabPassive) {
        pickPoint = point.position().toPoint();
    } else if (transition == QPointingDevice::GrabExclusive) {
        if (m_dragHandler->centroid().pressedButtons().testFlag(Qt::LeftButton))
            m_graphsItem->doPicking(pickPoint);
    } else if (transition == QPointingDevice::UngrabExclusive
               || transition == QPointingDevice::UngrabPassive) {
        setPosition(QPointF(.0f, .0f));
        setScale(1.f);
        setRotation(.0f);
        emit m_graphsItem->selectedElementChanged(QtGraphs3D::ElementType::None);
        pickPoint = QPointF();
    }
}

void QGraphsInputHandler::onWheel(QQuickWheelEvent *event)
{
    if (!m_zoomEnabled) {
        qCWarning(lcEvents3D, "%s zooming needs to be enabled",
                qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    }

    int halfSizeZoomLevel = 50;
    int oneToOneZoomLevel = 100;

    int driftTowardCenterLevel = 175;
    float wheelZoomDrift = 0.1f;

    int nearZoomRangeDivider = 12;
    int midZoomRangeDivider = 60;
    int farZoomRangeDivider = 120;

    if (m_graphsItem->isSlicingActive())
        return;

    // Adjust zoom level based on what zoom range we're in.
    QQuickGraphsItem *item = m_graphsItem;
    int zoomLevel = int(item->cameraZoomLevel());
    const int minZoomLevel = int(item->minCameraZoomLevel());
    const int maxZoomLevel = int(item->maxCameraZoomLevel());
    if (zoomLevel > oneToOneZoomLevel)
        zoomLevel += event->angleDelta().y() / nearZoomRangeDivider;
    else if (zoomLevel > halfSizeZoomLevel)
        zoomLevel += event->angleDelta().y() / midZoomRangeDivider;
    else
        zoomLevel += event->angleDelta().y() / farZoomRangeDivider;
    zoomLevel = qBound(minZoomLevel, zoomLevel, maxZoomLevel);

    if (m_zoomAtTarget) {
        QVector3D targetPosition = item->graphPositionAt(QPoint(event->x(), event->y()));
        float previousZoom = item->cameraZoomLevel();
        item->setCameraZoomLevel(zoomLevel);

        float diffAdj = 0.0f;

        // If zooming in/out outside the graph, or zooming out after certain point,
        // move towards the center.
        if ((qAbs(targetPosition.x()) > 2.0f || qAbs(targetPosition.y()) > 2.0f
             || qAbs(targetPosition.z()) > 2.0f)
            || (previousZoom > zoomLevel && zoomLevel <= driftTowardCenterLevel)) {
            targetPosition = QVector3D();
            // Add some extra correction so that we actually reach the center
            // eventually
            diffAdj = wheelZoomDrift;
            if (previousZoom > zoomLevel)
                diffAdj *= 2.0f; // Correct towards center little more when zooming out
        }

        float zoomFraction = 1.0f - (previousZoom / zoomLevel);

        // Adjust camera towards the zoom point, attempting to keep the cursor at
        // same graph point
        QVector3D oldTarget = item->cameraTargetPosition();
        QVector3D origDiff = targetPosition - oldTarget;
        QVector3D diff = origDiff * zoomFraction + (origDiff.normalized() * diffAdj);
        if (diff.length() > origDiff.length())
            diff = origDiff;
        item->setCameraTargetPosition(oldTarget + diff);
    } else {
        item->setCameraZoomLevel(zoomLevel);
    }

    item->update();
}

void QGraphsInputHandler::onPinchScaleChanged(qreal delta)
{
    if (!m_zoomEnabled) {
        qCWarning(lcEvents3D, "%s zooming needs to be enabled",
                qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    }

    m_pinchDiff += (delta - 1.0f);
    int driftTowardCenterLevel = 175;
    float wheelZoomDrift = 0.1f;

    QQuickGraphsItem *item = m_graphsItem;
    int zoomLevel = int(item->cameraZoomLevel());
    const int minZoomLevel = int(item->minCameraZoomLevel());
    const int maxZoomLevel = int(item->maxCameraZoomLevel());
    float zoomRate = qSqrt(qSqrt(zoomLevel));
    if (m_pinchDiff > .0f)
        zoomLevel += zoomRate;
    else
        zoomLevel -= zoomRate;
    zoomLevel = qBound(minZoomLevel, zoomLevel, maxZoomLevel);
    if (m_zoomAtTarget) {
        QVector3D targetPosition = item->graphPositionAt(
            m_pinchHandler->centroid().position().toPoint());
        item->setCameraZoomLevel(zoomLevel);

        float diffAdj = 0.0f;

        // If zooming in/out outside the graph, or zooming out after certain point,
        // move towards the center.
        if ((qAbs(targetPosition.x()) > 2.0f || qAbs(targetPosition.y()) > 2.0f
             || qAbs(targetPosition.z()) > 2.0f)
            || (m_pinchDiff > .0f && zoomLevel <= driftTowardCenterLevel)) {
            targetPosition = QVector3D();
            // Add some extra correction so that we actually reach the center
            // eventually
            diffAdj = wheelZoomDrift;
            if (m_pinchDiff > .0f)
                diffAdj *= 2.0f; // Correct towards center little more when zooming out
        }

        // Adjust camera towards the zoom point, attempting to keep the cursor at
        // same graph point
        QVector3D oldTarget = item->cameraTargetPosition();
        QVector3D origDiff = targetPosition - oldTarget;
        QVector3D diff = origDiff * m_pinchDiff + (origDiff.normalized() * diffAdj);
        if (diff.length() > origDiff.length())
            diff = origDiff;
        item->setCameraTargetPosition(oldTarget + diff);
    } else {
        item->setCameraZoomLevel(zoomLevel);
    }
    m_pinchDiff = .0f;
}

void QGraphsInputHandler::hoverMoveEvent(QHoverEvent *event)
{
    Q_EMIT mouseMove(event->oldPos());
}

QT_END_NAMESPACE

#include "moc_qgraphsinputhandler_p.cpp"
