// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef AXISGRID_H
#define AXISGRID_H

//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include <QtQuick/private/qquickshadereffect_p.h>

QT_BEGIN_NAMESPACE

class AxisGrid : public QQuickShaderEffect
{
    Q_OBJECT
    Q_PROPERTY(QVector3D iResolution READ iResolution NOTIFY iResolutionChanged FINAL)
    Q_PROPERTY(qreal smoothing READ smoothing WRITE setSmoothing NOTIFY smoothingChanged FINAL)
    Q_PROPERTY(int origo READ origo WRITE setOrigo NOTIFY origoChanged FINAL)
    Q_PROPERTY(QVector4D gridVisibility READ gridVisibility WRITE setGridVisibility NOTIFY gridVisibilityChanged FINAL)
    Q_PROPERTY(qreal gridWidth READ gridWidth WRITE setGridWidth NOTIFY gridWidthChanged FINAL)
    Q_PROPERTY(qreal gridHeight READ gridHeight WRITE setGridHeight NOTIFY gridHeightChanged FINAL)
    Q_PROPERTY(QPointF gridMovement READ gridMovement WRITE setGridMovement NOTIFY gridMovementChanged FINAL)
    Q_PROPERTY(QColor subGridColor READ subGridColor WRITE setSubGridColor NOTIFY subGridColorChanged FINAL)
    Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor NOTIFY gridColorChanged FINAL)
    Q_PROPERTY(QColor plotAreaBackgroundColor READ plotAreaBackgroundColor
                       WRITE setPlotAreaBackgroundColor NOTIFY plotAreaBackgroundColorChanged FINAL)
    Q_PROPERTY(qreal subGridLineWidth READ subGridLineWidth WRITE setSubGridLineWidth NOTIFY subGridLineWidthChanged FINAL)
    Q_PROPERTY(qreal gridLineWidth READ gridLineWidth WRITE setGridLineWidth NOTIFY gridLineWidthChanged FINAL)
    Q_PROPERTY(qreal verticalSubGridScale READ verticalSubGridScale WRITE setVerticalSubGridScale NOTIFY verticalSubGridScaleChanged FINAL)
    Q_PROPERTY(qreal horizontalSubGridScale READ horizontalSubGridScale WRITE setHorizontalSubGridScale NOTIFY horizontalSubGridScaleChanged FINAL)
public:
    AxisGrid(QQuickItem *parent = nullptr);
    ~AxisGrid() override;
    void setupShaders();

    QVector3D iResolution() const;

    qreal smoothing() const;
    void setSmoothing(qreal newSmoothing);

    int origo() const;
    void setOrigo(int newOrigo);

    QVector4D gridVisibility() const;
    void setGridVisibility(const QVector4D &newGridVisibility);

    qreal gridWidth() const;
    void setGridWidth(qreal newGridWidth);

    qreal gridHeight() const;
    void setGridHeight(qreal newGridHeight);

    QPointF gridMovement() const;
    void setGridMovement(QPointF newGridMovement);

    QColor subGridColor() const;
    void setSubGridColor(QColor newSubGridColor);

    QColor gridColor() const;
    void setGridColor(QColor newGridColor);

    QColor plotAreaBackgroundColor() const;
    void setPlotAreaBackgroundColor(QColor color);

    qreal subGridLineWidth() const;
    void setSubGridLineWidth(qreal newSubGridLineWidth);

    qreal gridLineWidth() const;
    void setGridLineWidth(qreal newGridLineWidth);

    qreal verticalSubGridScale() const;
    void setVerticalSubGridScale(qreal newVerticalSubGridScale);

    qreal horizontalSubGridScale() const;
    void setHorizontalSubGridScale(qreal newHorizontalSubGridScale);

protected:
    void componentComplete() override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

Q_SIGNALS:
    void iResolutionChanged();
    void smoothingChanged();
    void origoChanged();
    void gridVisibilityChanged();
    void gridWidthChanged();
    void gridHeightChanged();
    void gridMovementChanged();
    void subGridColorChanged();
    void gridColorChanged();
    void plotAreaBackgroundColorChanged();
    void subGridLineWidthChanged();
    void gridLineWidthChanged();
    void verticalSubGridScaleChanged();
    void horizontalSubGridScaleChanged();

private:
    friend class AxisRenderer;
    qreal m_smoothing = 1.0;
    QVector3D m_iResolution;
    int m_origo = 0;
    QVector4D m_gridVisibility = QVector4D(1, 1, 1, 1);
    qreal m_gridWidth = 100;
    qreal m_gridHeight = 100;
    QPointF m_gridMovement;
    QColor m_subGridColor = QColor(150, 150, 150);
    QColor m_gridColor = QColor(255, 255, 255);
    QColor m_plotAreaBackgroundColor = QColor(0, 0, 0, 0);
    qreal m_subGridLineWidth = 1.0;
    qreal m_gridLineWidth = 2.0;
    qreal m_verticalSubGridScale = 0.1;
    qreal m_horizontalSubGridScale = 0.1;
};

QT_END_NAMESPACE

#endif // AXISGRID_H
