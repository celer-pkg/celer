// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SCATTERGRAPH_H
#define SCATTERGRAPH_H

#include <QtCore/qobject.h>
#include <QtGraphsWidgets/q3dscatterwidgetitem.h>
#include "scatterdatamodifier.h"
#include "scattergraphwidget.h"

class ScatterGraph : public QObject
{
    Q_OBJECT
public:
    ScatterGraph(QWidget *parent = nullptr);

    void initialize();
    QWidget *scatterWidget() { return m_scatterWidget; }

private:
    ScatterDataModifier *m_modifier = nullptr;
    ScatterGraphWidget *m_scatterGraphWidget = nullptr;
    QWidget *m_scatterWidget = nullptr;
};

#endif
