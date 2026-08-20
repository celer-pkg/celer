// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef BARGRAPH_H
#define BARGRAPH_H

#include <QtCore/qobject.h>
#include <QtGraphsWidgets/q3dbarswidgetitem.h>
#include "graphmodifier.h"

class BarGraph : public QObject
{
    Q_OBJECT
public:
    BarGraph(QWidget *parent = nullptr);

    void initialize();
    QWidget *barsWidget() { return m_container; }

private:
    GraphModifier *m_modifier = nullptr;
    Q3DBarsWidgetItem *m_barGraph = nullptr;
    QQuickWidget *m_quickWidget = nullptr;
    QWidget *m_container = nullptr;
};

#endif
