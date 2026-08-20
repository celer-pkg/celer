// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QTOUCH3DINPUTHANDLER_H
#define QTDATAVIS3D_QTOUCH3DINPUTHANDLER_H

#include <QtDataVisualization/q3dinputhandler.h>

QT_BEGIN_NAMESPACE

class QTouch3DInputHandlerPrivate;

class Q_DATAVISUALIZATION_EXPORT QTouch3DInputHandler : public Q3DInputHandler
{
    Q_OBJECT

public:
    explicit QTouch3DInputHandler(QObject *parent = nullptr);
    virtual ~QTouch3DInputHandler();

    // Input event listeners
    void touchEvent(QTouchEvent *event) override;

private:
    Q_DISABLE_COPY(QTouch3DInputHandler)

    QScopedPointer<QTouch3DInputHandlerPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif
