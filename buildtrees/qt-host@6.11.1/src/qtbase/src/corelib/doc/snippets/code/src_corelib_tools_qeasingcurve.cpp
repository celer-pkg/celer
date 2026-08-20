// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QEasingCurve>
#include <QDebug> // check if it is needed
#include <QPropertyAnimation>

//! [typedef]
qreal myEasingFunction(qreal progress);
//! [typedef]

void examples()
{
    //! [0]
    QEasingCurve easing(QEasingCurve::InOutQuad);

    for (qreal t = 0.0; t < 1.0; t += 0.1)
        qWarning() << "Effective progress" << t << "is"
                    << easing.valueForProgress(t);
    //! [0]

    //! [1]
    QPropertyAnimation animation;
    animation.setStartValue(0);
    animation.setEndValue(1000);
    animation.setDuration(1000);
    animation.setEasingCurve(QEasingCurve::InOutQuad);
    //! [1]
}
