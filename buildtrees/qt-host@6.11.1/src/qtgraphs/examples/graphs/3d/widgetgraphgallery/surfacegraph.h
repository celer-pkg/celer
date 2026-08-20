// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SURFACEGRAPH_H
#define SURFACEGRAPH_H

#include <QtCore/qobject.h>
#include <QtGraphsWidgets/q3dsurfacewidgetitem.h>
#include "surfacegraphmodifier.h"
#include "surfacegraphwidget.h"

class SurfaceGraph : public QObject
{
    Q_OBJECT
public:
    SurfaceGraph(QWidget *parent = nullptr);

    void initialize();
    QWidget *surfaceWidget() { return m_surfaceWidget; }

private:
    SurfaceGraphModifier *m_modifier = nullptr;
    SurfaceGraphWidget *m_surfaceGraphWidget = nullptr;
    QWidget *m_surfaceWidget = nullptr;
};

#endif
