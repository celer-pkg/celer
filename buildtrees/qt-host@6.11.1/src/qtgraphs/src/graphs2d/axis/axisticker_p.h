// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef AXISTICKER_H
#define AXISTICKER_H

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

class AxisTicker : public QQuickShaderEffect
{
    Q_OBJECT
    Q_PROPERTY(QVector3D iResolution READ iResolution NOTIFY iResolutionChanged FINAL)
    Q_PROPERTY(qreal smoothing READ smoothing WRITE setSmoothing NOTIFY smoothingChanged FINAL)
    Q_PROPERTY(int origo READ origo WRITE setOrigo NOTIFY origoChanged FINAL)
    Q_PROPERTY(bool subTicksVisible READ subTicksVisible WRITE setSubTicksVisible NOTIFY subTicksVisibleChanged FINAL)
    Q_PROPERTY(qreal spacing READ spacing WRITE setSpacing NOTIFY spacingChanged FINAL)
    Q_PROPERTY(qreal displacement READ displacement WRITE setDisplacement NOTIFY displacementChanged FINAL)
    Q_PROPERTY(QColor subTickColor READ subTickColor WRITE setSubTickColor NOTIFY subTickColorChanged FINAL)
    Q_PROPERTY(QColor tickColor READ tickColor WRITE setTickColor NOTIFY tickColorChanged FINAL)
    Q_PROPERTY(qreal subTickLineWidth READ subTickLineWidth WRITE setSubTickLineWidth NOTIFY subTickLineWidthChanged FINAL)
    Q_PROPERTY(qreal tickLineWidth READ tickLineWidth WRITE setTickLineWidth NOTIFY tickLineWidthChanged FINAL)
    Q_PROPERTY(qreal subTickScale READ subTickScale WRITE setSubTickScale NOTIFY subTickScaleChanged FINAL)
    Q_PROPERTY(qreal subTickLength READ subTickLength WRITE setSubTickLength NOTIFY subTickLengthChanged FINAL)
    Q_PROPERTY(bool isHorizontal READ isHorizontal WRITE setIsHorizontal NOTIFY isHorizontalChanged FINAL)
    Q_PROPERTY(bool flipped READ isFlipped WRITE setFlipped NOTIFY flippedChanged FINAL)
public:
    AxisTicker(QQuickItem *parent = nullptr);
    ~AxisTicker() override;

    void setupShaders();

    QVector3D iResolution() const;

    qreal smoothing() const;
    void setSmoothing(qreal newSmoothing);

    int origo() const;
    void setOrigo(int newOrigo);

    bool subTicksVisible() const;
    void setSubTicksVisible(bool newSubTicksVisible);

    qreal spacing() const;
    void setSpacing(qreal newSpacing);

    qreal displacement() const;
    void setDisplacement(qreal newDisplacement);

    QColor subTickColor() const;
    void setSubTickColor(QColor newSubTickColor);

    QColor tickColor() const;
    void setTickColor(QColor newTickColor);

    qreal subTickLineWidth() const;
    void setSubTickLineWidth(qreal newSubTickLineWidth);

    qreal tickLineWidth() const;
    void setTickLineWidth(qreal newTickLineWidth);

    qreal subTickScale() const;
    void setSubTickScale(qreal newSubTickScale);

    qreal subTickLength() const;
    void setSubTickLength(qreal newSubTickLength);

    bool isHorizontal() const;
    void setIsHorizontal(bool newIsHorizontal);

    bool isFlipped() const;
    void setFlipped(bool newFlipped);

protected:
    void componentComplete() override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

Q_SIGNALS:
    void iResolutionChanged();
    void smoothingChanged();
    void origoChanged();
    void subTicksVisibleChanged();
    void spacingChanged();
    void displacementChanged();
    void subTickColorChanged();
    void tickColorChanged();
    void subTickLineWidthChanged();
    void tickLineWidthChanged();
    void subTickScaleChanged();
    void subTickLengthChanged();
    void isHorizontalChanged();
    void flippedChanged();

private:
    friend class AxisRenderer;
    QVector3D m_iResolution;
    qreal m_smoothing = 1.0;
    int m_origo = 0;
    bool m_subTicksVisible = true;
    qreal m_spacing = 100;
    qreal m_displacement = 0;
    QColor m_subTickColor = QColor(150, 150, 150);
    QColor m_tickColor = QColor(255, 255, 255);
    qreal m_subTickLineWidth = 1.0;
    qreal m_tickLineWidth = 2.0;
    qreal m_subTickScale = 0.1;
    qreal m_subTickLength = 0.1;
    bool m_isHorizontal = false;
    bool m_flipped = false;
};

QT_END_NAMESPACE

#endif // AXISTICKER_H
