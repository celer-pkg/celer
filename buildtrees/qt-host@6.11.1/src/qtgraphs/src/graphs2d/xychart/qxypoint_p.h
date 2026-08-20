// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Graphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef DECLARATIVE_XY_POINT_H
#define DECLARATIVE_XY_POINT_H

#include <QtQml/qqmlregistration.h>
#include <QtCore/QObject>
#include <QtCore/QPointF>

QT_BEGIN_NAMESPACE

class QXYPoint : public QObject, public QPointF
{
    Q_OBJECT
    Q_PROPERTY(qreal x READ x WRITE setX FINAL)
    Q_PROPERTY(qreal y READ y WRITE setY FINAL)
    QML_NAMED_ELEMENT(XYPoint)

public:
    explicit QXYPoint(QObject *parent = 0);
};

QT_END_NAMESPACE

#endif // DECLARATIVE_XY_POINT_H
