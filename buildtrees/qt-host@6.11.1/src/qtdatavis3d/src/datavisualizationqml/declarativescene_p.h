// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef DECLARATIVESCENE_P_H
#define DECLARATIVESCENE_P_H

#include <QtQml/qqml.h>
#include <private/datavisualizationglobal_p.h>
#include <QtDataVisualization/q3dscene.h>

QT_BEGIN_NAMESPACE

class Declarative3DScene : public Q3DScene
{
    Q_OBJECT
    // This property is overloaded to use QPointF instead of QPoint to work around qml bug
    // where Qt.point(0, 0) can't be assigned due to error "Cannot assign QPointF to QPoint".
    Q_PROPERTY(QPointF selectionQueryPosition READ selectionQueryPosition WRITE setSelectionQueryPosition NOTIFY selectionQueryPositionChanged)
    // This is static method in parent class, overload as constant property for qml.
    Q_PROPERTY(QPoint invalidSelectionPoint READ invalidSelectionPoint CONSTANT)

    QML_NAMED_ELEMENT(Scene3D)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("Trying to create uncreatable: Scene3D.")

public:
    Declarative3DScene(QObject *parent = 0);
    virtual ~Declarative3DScene();

    void setSelectionQueryPosition(const QPointF &point);
    QPointF selectionQueryPosition() const;
    QPoint invalidSelectionPoint() const;

Q_SIGNALS:
    void selectionQueryPositionChanged(const QPointF &position);
};

QT_END_NAMESPACE

#endif
