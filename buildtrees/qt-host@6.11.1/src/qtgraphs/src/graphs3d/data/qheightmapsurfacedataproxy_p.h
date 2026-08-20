// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QHEIGHTMAPSURFACEDATAPROXY_P_H
#define QHEIGHTMAPSURFACEDATAPROXY_P_H

#include "qheightmapsurfacedataproxy.h"
#include "qsurfacedataproxy_p.h"
#include <QtCore/QTimer>

QT_BEGIN_NAMESPACE

class QHeightMapSurfaceDataProxyPrivate : public QSurfaceDataProxyPrivate
{
    Q_DECLARE_PUBLIC(QHeightMapSurfaceDataProxy)

public:
    QHeightMapSurfaceDataProxyPrivate();
    ~QHeightMapSurfaceDataProxyPrivate() override;

    void setValueRanges(float minX, float maxX, float minZ, float maxZ);
    void setMinXValue(float min);
    void setMaxXValue(float max);
    void setMinZValue(float min);
    void setMaxZValue(float max);
    void setMinYValue(float min);
    void setMaxYValue(float max);
    void setAutoScaleY(bool enabled);

private:
    void handlePendingResolve();

    QImage m_heightMap;
    QString m_heightMapFile;
    QTimer m_resolveTimer;

    float m_minXValue;
    float m_maxXValue;
    float m_minZValue;
    float m_maxZValue;
    float m_minYValue;
    float m_maxYValue;
    bool m_autoScaleY;
};

QT_END_NAMESPACE

#endif
