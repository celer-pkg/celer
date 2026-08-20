// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "demoqcpainteritem.h"
#include "demoqcpainterrenderer.h"

DemoQCPainterItem::DemoQCPainterItem(QQuickItem *parent)
:  QCanvasPainterItem(parent)
{
}

QCanvasPainterItemRenderer* DemoQCPainterItem::createItemRenderer() const
{
    return new DemoQCPainterRenderer();
}
