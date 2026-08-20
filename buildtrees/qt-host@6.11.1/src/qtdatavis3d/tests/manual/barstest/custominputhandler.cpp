// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "custominputhandler.h"

#include <QtDataVisualization/Q3DCamera>

CustomInputHandler::CustomInputHandler(QObject *parent) :
    QAbstract3DInputHandler(parent)
{
}

//! [0]
void CustomInputHandler::mouseMoveEvent(QMouseEvent *event, const QPoint &mousePos)
{
    Q_UNUSED(event);
    setInputPosition(mousePos);
}
//! [0]

//! [1]
void CustomInputHandler::wheelEvent(QWheelEvent *event)
{
    // Adjust zoom level based on what zoom range we're in.
    int zoomLevel = scene()->activeCamera()->zoomLevel();
    if (zoomLevel > 100)
        zoomLevel += event->angleDelta().y() / 12;
    else if (zoomLevel > 50)
        zoomLevel += event->angleDelta().y() / 60;
    else
        zoomLevel += event->angleDelta().y() / 120;
    if (zoomLevel > 500)
        zoomLevel = 500;
    else if (zoomLevel < 10)
        zoomLevel = 10;

    scene()->activeCamera()->setZoomLevel(zoomLevel);
}
//! [1]
