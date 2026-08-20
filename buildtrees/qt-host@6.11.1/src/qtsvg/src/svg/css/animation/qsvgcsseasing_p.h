// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSVGCSSEASING_P_H
#define QSVGCSSEASING_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtSvg/private/qtsvgglobal_p.h>
#include <QtSvg/private/qsvgeasinginterface_p.h>
#include <QtSvg/private/qsvgcssvalues_p.h>
#include <QtCore/qeasingcurve.h>
#include <QtCore/qpoint.h>

QT_BEGIN_NAMESPACE

class Q_SVG_EXPORT QSvgCssEasing : public QSvgEasingInterface
{
public:
    QSvgCssEasing(QSvgCssValues::EasingFunction easingFunction);
    QSvgCssValues::EasingFunction easingFunction() const;

private:
    QSvgCssValues::EasingFunction m_easingFunction;
};

class Q_SVG_EXPORT QSvgCssCubicBezierEasing : public QSvgCssEasing
{
public:
    QSvgCssCubicBezierEasing(QSvgCssValues::EasingFunction easingFunction, const QPointF &c1, const QPointF &c2);
    virtual qreal progress(qreal t) override;
    QPointF c1() const;
    QPointF c2() const;

private:
    QEasingCurve m_easingCurve;
    QPointF m_c1, m_c2;
};

class Q_SVG_EXPORT QSvgCssStepsEasing : public QSvgCssEasing
{
public:
    QSvgCssStepsEasing(quint32 stops, QSvgCssValues::StepPosition position);
    virtual qreal progress(qreal t) override;
    quint32 stops() const;
    QSvgCssValues::StepPosition stepPosition();

private:
    quint32 m_stops;
    QSvgCssValues::StepPosition m_stepPosition;
};

using QSvgCssEasingPtr = std::unique_ptr<QSvgCssEasing>;

QT_END_NAMESPACE

#endif // QSVGCSSEASING_P_H
