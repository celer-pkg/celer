// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QGRAPHSINPUTHANDLER_P_H
#define QGRAPHSINPUTHANDLER_P_H

#include <QtQuick/qquickitem.h>

QT_BEGIN_NAMESPACE

class QQuickGraphsItem;
class QQuickTapHandler;
class QQuickDragHandler;
class QQuickPinchHandler;
class QQuickWheelHandler;
class QQuickWheelEvent;

class QGraphsInputHandler : public QQuickItem
{
    Q_OBJECT

public:
    QGraphsInputHandler(QQuickItem *parent = nullptr);

    ~QGraphsInputHandler() override;

    void setGraphsItem(QQuickGraphsItem *item);
    QPoint pendingPoint() { return m_pendingPoint; }

    void setZoomEnabled(bool enable);
    bool isZoomEnabled();
    void setZoomAtTargetEnabled(bool enable);
    bool isZoomAtTargetEnabled();
    void setRotationEnabled(bool enable);
    bool isRotationEnabled();
    void setSelectionEnabled(bool enable);
    bool isSelectionEnabled();

    void setDefaultInputHandler();
    void unsetDefaultInputHandler();
    void unsetDefaultTapHandler();
    void unsetDefaultDragHandler();
    void unsetDefaultWheelHandler();
    void unsetDefaultPinchHandler();
    void setDragButton(Qt::MouseButtons button);

    void onTapped();
    void onTranslationChanged(QVector2D delta);
    void onGrabChanged(QPointingDevice::GrabTransition transition, QEventPoint point);
    void onWheel(QQuickWheelEvent *event);
    void onPinchScaleChanged(qreal delta);

    Q_SIGNAL void mouseMove(QPoint mousePos);

protected:
    void hoverMoveEvent(QHoverEvent *event) override;

private:
    bool m_zoomEnabled;
    bool m_zoomAtTarget;
    bool m_rotationEnabled;
    bool m_selectionEnabled;
    QPoint m_pendingPoint;
    qreal m_pinchDiff;

    QQuickTapHandler *m_tapHandler;
    QQuickPinchHandler *m_pinchHandler;
    QQuickWheelHandler *m_wheelHandler;
    QQuickDragHandler *m_dragHandler;

    QQuickGraphsItem *m_graphsItem;
};

QT_END_NAMESPACE

#endif // QGRAPHSINPUTHANDLER_P_H
