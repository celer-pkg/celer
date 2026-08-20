// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SURFACEGRAPH_H
#define SURFACEGRAPH_H

#include <QtCore/qobject.h>
#include <QtDataVisualization/q3dsurface.h>

class SurfaceGraph : public QObject
{
    Q_OBJECT
public:
    SurfaceGraph();
    ~SurfaceGraph();

    bool initialize(const QSize &minimumGraphSize, const QSize &maximumGraphSize);
    QWidget *surfaceWidget() { return m_surfaceWidget; }

private:
    Q3DSurface *m_surfaceGraph = nullptr;
    QWidget *m_container = nullptr;
    QWidget *m_surfaceWidget = nullptr;
};

#endif
