// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SURFACEGRAPHMODIFIER_H
#define SURFACEGRAPHMODIFIER_H

#include <QtCore/qobject.h>
#include <QtGraphsWidgets/q3dsurfacewidgetitem.h>

class QSurfaceDataProxy;
class QSurface3DSeries;
class QQuickItemGrabResult;

class SurfaceGraphModifier : public QObject
{
    Q_OBJECT

public:
    explicit SurfaceGraphModifier(Q3DSurfaceWidgetItem *surface, QObject *parent);
    ~SurfaceGraphModifier();

    void renderSliceToImage(QtGraphs3D::SliceCaptureType sliceType, int requestedIndex);
    void changeSelectionMode(bool checked, bool nopick = true);

Q_SIGNALS:
    void updateSliceImage(const QImage &image);

private:
    void fillSqrtSinProxy();

private:
    Q3DSurfaceWidgetItem *m_graph = nullptr;
    QSurfaceDataProxy *m_sqrtSinProxy = nullptr;
    QSurface3DSeries *m_sqrtSinSeries = nullptr;
};

#endif // SURFACEGRAPHMODIFIER_H
