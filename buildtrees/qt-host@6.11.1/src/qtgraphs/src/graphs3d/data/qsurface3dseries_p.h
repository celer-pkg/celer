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

#ifndef QSURFACE3DSERIES_P_H
#define QSURFACE3DSERIES_P_H

#include "qabstract3dseries_p.h"
#include "qsurface3dseries.h"
#include "qvalue3daxis.h"

QT_BEGIN_NAMESPACE

class QSurface3DSeriesPrivate : public QAbstract3DSeriesPrivate
{
    Q_DECLARE_PUBLIC(QSurface3DSeries)

public:
    QSurface3DSeriesPrivate();
    ~QSurface3DSeriesPrivate() override;

    void setDataProxy(QAbstractDataProxy *proxy) override;
    void connectGraphAndProxy(QQuickGraphsItem *newGraph) override;
    void createItemLabel() override;

    void setSelectedPoint(QPoint position);
    void setShading(const QSurface3DSeries::Shading shading);
    void setDrawMode(QSurface3DSeries::DrawFlags mode);
    void setTexture(const QImage &texture);
    void setWireframeColor(QColor color);
    void setRowsSanitized(bool enabled);

    void setDataArray(const QSurfaceDataArray &newDataArray);
    void clearRow(qsizetype rowIndex);
    void clearArray();

    void setAxisX(QValue3DAxis *axis);
    void setAxisY(QValue3DAxis *axis);
    void setAxisZ(QValue3DAxis *axis);
    void resetAxisX();
    void resetAxisY();
    void resetAxisZ();

private:
    QSurfaceDataArray m_dataArray;
    QPoint m_selectedPoint;
    QSurface3DSeries::Shading m_shading;
    QSurface3DSeries::DrawFlags m_drawMode;
    QImage m_texture;
    QString m_textureFile;
    QColor m_wireframeColor;
    bool m_rowsSanitized;
    QValue3DAxis *m_axisX = nullptr;
    QValue3DAxis *m_axisY = nullptr;
    QValue3DAxis *m_axisZ = nullptr;
};

QT_END_NAMESPACE

#endif
