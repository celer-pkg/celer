// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef AXISLINE_H
#define AXISLINE_H

//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include <QtQuick/private/qquickshadereffect_p.h>
#include <QQmlEngine>

QT_BEGIN_NAMESPACE

class AxisLine : public QQuickShaderEffect
{
    Q_OBJECT
    Q_PROPERTY(QVector3D iResolution READ iResolution NOTIFY iResolutionChanged FINAL)
    Q_PROPERTY(qreal smoothing READ smoothing WRITE setSmoothing NOTIFY smoothingChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged FINAL)
    Q_PROPERTY(bool isHorizontal READ isHorizontal WRITE setIsHorizontal NOTIFY isHorizontalChanged FINAL)
public:
    explicit AxisLine(QQuickItem *parent = nullptr);
    ~AxisLine() override;
    void setupShaders();

    QVector3D iResolution() const;

    qreal smoothing() const;
    void setSmoothing(qreal newSmoothing);

    QColor color() const;
    void setColor(QColor newColor);

    qreal lineWidth() const;
    void setLineWidth(qreal newLineWidth);

    bool isHorizontal() const;
    void setIsHorizontal(bool newIsHorizontal);

protected:
    void componentComplete() override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

Q_SIGNALS:
    void iResolutionChanged();
    void smoothingChanged();
    void colorChanged();
    void lineWidthChanged();
    void isHorizontalChanged();

private:
    friend class AxisRenderer;
    QVector3D m_iResolution;
    qreal m_smoothing = 1.0;
    QColor m_color = QColor(255, 255, 255);
    qreal m_lineWidth = 2.0;
    bool m_isHorizontal = false;
};

QT_END_NAMESPACE

#endif // AXISLINE_H
