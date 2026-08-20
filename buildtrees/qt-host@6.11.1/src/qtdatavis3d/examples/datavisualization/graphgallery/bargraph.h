// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef BARGRAPH_H
#define BARGRAPH_H

#include <QtDataVisualization/q3dbars.h>
#include <QtCore/qobject.h>

class BarGraph : public QObject
{
    Q_OBJECT
public:
    BarGraph();
    ~BarGraph();

    bool initialize(const QSize &minimumGraphSize, const QSize &maximumGraphSize);
    QWidget *barsWidget() { return m_barsWidget; }

private:
    Q3DBars *m_barsGraph = nullptr;
    QWidget *m_container = nullptr;
    QWidget *m_barsWidget = nullptr;
};

#endif
