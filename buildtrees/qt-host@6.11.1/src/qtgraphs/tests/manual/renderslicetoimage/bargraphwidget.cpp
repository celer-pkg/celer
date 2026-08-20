// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "bargraphwidget.h"

#include <QtGraphsWidgets/q3dbarswidgetitem.h>

BarGraphWidget::BarGraphWidget() {}

BarGraphWidget::~BarGraphWidget() {}

void BarGraphWidget::initialize()
{
    m_barGraph = new Q3DBarsWidgetItem();
    m_barGraph->setWidget(this);
}

Q3DBarsWidgetItem *BarGraphWidget::barGraph() const
{
    return m_barGraph;
}
