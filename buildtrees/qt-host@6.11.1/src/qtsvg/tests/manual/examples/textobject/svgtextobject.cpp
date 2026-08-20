// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>
#include <QtSvg>

#include "svgtextobject.h"
#include "window.h"

//![0]
QSizeF SvgTextObject::intrinsicSize(QTextDocument * /*doc*/, int /*posInDocument*/,
                                    const QTextFormat &format)
{
    QImage bufferedImage = qvariant_cast<QImage>(format.property(Window::SvgData));
    QSize size = bufferedImage.size();
    
    if (size.height() > 25)
        size *= 25.0 / (double) size.height();

    return QSizeF(size);
}
//![0]

//![1]
void SvgTextObject::drawObject(QPainter *painter, const QRectF &rect,
                               QTextDocument * /*doc*/, int /*posInDocument*/,
                               const QTextFormat &format)
{
    QImage bufferedImage = qvariant_cast<QImage>(format.property(Window::SvgData));

    painter->drawImage(rect, bufferedImage);
}
//![1]

