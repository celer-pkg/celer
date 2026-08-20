// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QPIESLICE_P_H
#define QPIESLICE_P_H

//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#include <QtCore/private/qobject_p.h>
#include <QtCore/qpoint.h>
#include <QtGraphs/qpieslice.h>
#include <QtGui/QColor>
#include <QtGui/QFont>

QT_BEGIN_NAMESPACE

class QPieSeries;
class QQuickPathArc;
class QQuickPathLine;
class QQuickShape;
class QQuickShapePath;
class QQuickText;

class QPieSlicePrivate : public QObjectPrivate
{
public:
    static QPieSlicePrivate *get(QPieSlice *item) { return item->d_func(); }
    static const QPieSlicePrivate *get(const QPieSlice *item) { return item->d_func(); }

    QPieSlicePrivate();
    ~QPieSlicePrivate() override;

    void setPercentage(qreal percentage);
    void setStartAngle(qreal angle);
    void setAngleSpan(qreal span);

    void updateData(bool clearHidden = false);
    void updateSeries(QPieSeries *series);
    void handleSliceChange();

private:
    friend class QPieSeries;
    friend class QPieSeriesPrivate;
    friend class PieRenderer;

    void setLabelVisible(bool visible, bool forceHidden = false);
    void setLabelPosition(QPieSlice::LabelPosition position);

    QString m_labelText;
    bool m_isLabelVisible;
    QPieSlice::LabelPosition m_labelPosition;
    QColor m_color;
    QColor m_labelColor;
    QFont m_labelFont;
    qreal m_labelArmLengthFactor;
    qreal m_value;
    qreal m_percentage;
    qreal m_startAngle;
    qreal m_angleSpan;
    bool m_hideLabel;
    bool m_isExploded;
    qreal m_explodeDistanceFactor;

    bool m_labelDirty;

    QColor m_borderColor;
    qreal m_borderWidth;

    QQuickShapePath *m_shapePath;
    QQuickText *m_labelItem = nullptr;
    QQuickShape *m_labelShape = nullptr;
    QQuickShapePath *m_labelPath = nullptr;

    QPointF m_largeArc;
    QPointF m_centerLine;
    QPointF m_labelArm;

    QPieSeries *m_series = nullptr;

    qreal m_sum = 0;
    QList<QPieSlice *> m_subSlices;
    qreal m_subSlicesRatio = 0.7;

    Q_DECLARE_PUBLIC(QPieSlice)
};

QT_END_NAMESPACE

#endif // QPIESLICE_P_H
