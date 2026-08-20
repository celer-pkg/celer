// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSVGCSSVALUES_P_H
#define QSVGCSSVALUES_P_H

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

#include <QtSvg/qtsvgglobal.h>
#include <QPointF>
#include <variant>

QT_BEGIN_NAMESPACE

namespace QSvgCssValues {
enum class EasingFunction : quint8 {
    Ease,
    EaseIn,
    EaseOut,
    EaseInOut,
    CubicBezier,
    Linear,
    Steps,
};

enum class StepPosition : quint8 {
    JumpStart,
    JumpEnd,
    JumpNone,
    JumpBoth,
    Start = JumpStart,
    End = JumpEnd,
};

struct BezierControlPoints
{
    QPointF c1;
    QPointF c2;
};

struct StepValues
{
    quint32 steps;
    StepPosition stepPosition;
};

using EasingValues = std::variant<BezierControlPoints, StepValues>;

}

QT_END_NAMESPACE
#endif //QSVGCSSVALUES_P_H
