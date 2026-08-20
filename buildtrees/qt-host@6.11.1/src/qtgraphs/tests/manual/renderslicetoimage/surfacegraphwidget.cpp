// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "surfacegraphwidget.h"

#include <QtGraphsWidgets/q3dsurfacewidgetitem.h>

SurfaceGraphWidget::SurfaceGraphWidget() {}

SurfaceGraphWidget::~SurfaceGraphWidget() {}

void SurfaceGraphWidget::initialize()
{
    m_surfaceGraph = new Q3DSurfaceWidgetItem();
    m_surfaceGraph->setWidget(this);
}

Q3DSurfaceWidgetItem *SurfaceGraphWidget::surfaceGraph() const
{
    return m_surfaceGraph;
}
