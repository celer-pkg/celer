// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QVariantAnimation>
#include <QColor>

//! [0]
QVariant myColorInterpolator(const QColor &start, const QColor &end, qreal progress)
{
    // ...
    return QColor(/*...*/);
}
// ...
void someFunc()
{
    // ...
    qRegisterAnimationInterpolator<QColor>(myColorInterpolator);
    // ...
}
//! [0]

//! [1]
    QVariant myInterpolator(const QVariant &from, const QVariant &to, qreal progress);
//! [1]
