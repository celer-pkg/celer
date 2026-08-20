// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_Q3DINPUTHANDLER_H
#define QTDATAVIS3D_Q3DINPUTHANDLER_H

#include <QtDataVisualization/qabstract3dinputhandler.h>

QT_BEGIN_NAMESPACE

class Q3DInputHandlerPrivate;

class Q_DATAVISUALIZATION_EXPORT Q3DInputHandler : public QAbstract3DInputHandler
{
    Q_OBJECT
    Q_PROPERTY(bool rotationEnabled READ isRotationEnabled WRITE setRotationEnabled NOTIFY rotationEnabledChanged)
    Q_PROPERTY(bool zoomEnabled READ isZoomEnabled WRITE setZoomEnabled NOTIFY zoomEnabledChanged)
    Q_PROPERTY(bool selectionEnabled READ isSelectionEnabled WRITE setSelectionEnabled NOTIFY selectionEnabledChanged)
    Q_PROPERTY(bool zoomAtTargetEnabled READ isZoomAtTargetEnabled WRITE setZoomAtTargetEnabled NOTIFY zoomAtTargetEnabledChanged)

public:
    explicit Q3DInputHandler(QObject *parent = nullptr);
    virtual ~Q3DInputHandler();

    void setRotationEnabled(bool enable);
    bool isRotationEnabled() const;
    void setZoomEnabled(bool enable);
    bool isZoomEnabled() const;
    void setSelectionEnabled(bool enable);
    bool isSelectionEnabled() const;
    void setZoomAtTargetEnabled(bool enable);
    bool isZoomAtTargetEnabled() const;

    // Input event listeners
    void mousePressEvent(QMouseEvent *event, const QPoint &mousePos) override;
    void mouseReleaseEvent(QMouseEvent *event, const QPoint &mousePos) override;
    void mouseMoveEvent(QMouseEvent *event, const QPoint &mousePos) override;
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override;
#endif

Q_SIGNALS:
    void rotationEnabledChanged(bool enable);
    void zoomEnabledChanged(bool enable);
    void selectionEnabledChanged(bool enable);
    void zoomAtTargetEnabledChanged(bool enable);

private:
    Q_DISABLE_COPY(Q3DInputHandler)

    QScopedPointer<Q3DInputHandlerPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif
