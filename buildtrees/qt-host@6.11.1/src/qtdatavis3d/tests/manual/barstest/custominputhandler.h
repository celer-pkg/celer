// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CUSTOMINPUTHANDLER_H
#define CUSTOMINPUTHANDLER_H

#include <QtDataVisualization/QAbstract3DInputHandler>

class CustomInputHandler : public QAbstract3DInputHandler
{
    Q_OBJECT
public:
    explicit CustomInputHandler(QObject *parent = 0);

    virtual void mouseMoveEvent(QMouseEvent *event, const QPoint &mousePos) override;
    virtual void wheelEvent(QWheelEvent *event) override;
};

#endif
