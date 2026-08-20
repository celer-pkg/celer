// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SCATTERGRAPH_H
#define SCATTERGRAPH_H

#include <QtCore/qobject.h>
#include <QtDataVisualization/q3dscatter.h>

class ScatterGraph : public QObject
{
    Q_OBJECT
public:
    ScatterGraph();
    ~ScatterGraph();

    bool initialize(const QSize &minimumGraphSize, const QSize &maximumGraphSize);
    QWidget *scatterWidget() { return m_scatterWidget; }

private:
    Q3DScatter *m_scatterGraph = nullptr;
    QWidget *m_container = nullptr;
    QWidget *m_scatterWidget = nullptr;
};

#endif
