// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QPixmap>
#include <QPainter>

void example()
{
    //! [1]
    QPixmap p1, p2;
    p1.load("image.bmp");
    p2 = p1;                        // p1 and p2 share data

    QPainter paint;
    paint.begin(&p2);               // cuts p2 loose from p1
    paint.drawText(0,50, "Hi");
    paint.end();
    //! [1]
}
