// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QABSTRACT3DINPUTHANDLER_H
#define QTDATAVIS3D_QABSTRACT3DINPUTHANDLER_H

#include <QtDataVisualization/qdatavisualizationglobal.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QTouchEvent>

QT_BEGIN_NAMESPACE

class QAbstract3DInputHandlerPrivate;

class Q_DATAVISUALIZATION_EXPORT QAbstract3DInputHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(InputView inputView READ inputView WRITE setInputView NOTIFY inputViewChanged)
    Q_PROPERTY(QPoint inputPosition READ inputPosition WRITE setInputPosition NOTIFY positionChanged)
    Q_PROPERTY(Q3DScene *scene READ scene WRITE setScene NOTIFY sceneChanged)

public:
    enum InputView {
        InputViewNone = 0,
        InputViewOnPrimary,
        InputViewOnSecondary
    };
    Q_ENUM(InputView)

protected:
    explicit QAbstract3DInputHandler(QObject *parent = nullptr);
public:
    virtual ~QAbstract3DInputHandler();

    // Input event listeners
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void touchEvent(QTouchEvent *event);
    virtual void mousePressEvent(QMouseEvent *event, const QPoint &mousePos);
    virtual void mouseReleaseEvent(QMouseEvent *event, const QPoint &mousePos);
    virtual void mouseMoveEvent(QMouseEvent *event, const QPoint &mousePos);
#if QT_CONFIG(wheelevent)
    virtual void wheelEvent(QWheelEvent *event);
#endif

    InputView inputView() const;
    void setInputView(InputView inputView);

    QPoint inputPosition() const;
    void setInputPosition(const QPoint &position);

    Q3DScene *scene() const;
    void setScene(Q3DScene *scene);

Q_SIGNALS:
    void positionChanged(const QPoint &position);
    void inputViewChanged(QAbstract3DInputHandler::InputView view);
    void sceneChanged(Q3DScene *scene);

protected:
    void setPrevDistance(int distance);
    int prevDistance() const;
    void setPreviousInputPos(const QPoint &position);
    QPoint previousInputPos() const;

private:
    Q_DISABLE_COPY(QAbstract3DInputHandler)

    QScopedPointer<QAbstract3DInputHandlerPrivate> d_ptr;

    friend class Abstract3DController;
    friend class QTouch3DInputHandlerPrivate;
};

QT_END_NAMESPACE

#endif
