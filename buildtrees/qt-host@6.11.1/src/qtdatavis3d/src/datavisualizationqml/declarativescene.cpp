// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "declarativescene_p.h"

QT_BEGIN_NAMESPACE

Declarative3DScene::Declarative3DScene(QObject *parent)
    : Q3DScene(parent)
{
    QObject::connect(this, &Q3DScene::selectionQueryPositionChanged, this,
                     &Declarative3DScene::selectionQueryPositionChanged);
}

Declarative3DScene::~Declarative3DScene()
{
}

void Declarative3DScene::setSelectionQueryPosition(const QPointF &point)
{
    Q3DScene::setSelectionQueryPosition(point.toPoint());
}

QPointF Declarative3DScene::selectionQueryPosition() const
{
    return QPointF(Q3DScene::selectionQueryPosition());
}

QPoint Declarative3DScene::invalidSelectionPoint() const
{
    return Q3DScene::invalidSelectionPoint();
}

QT_END_NAMESPACE
