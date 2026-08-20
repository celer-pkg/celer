// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


#include "imagehelper.h"

#include <QtGui/QImageReader>

ImageHelper::ImageHelper(QObject *parent)
    : QObject{parent}
{

}


QString ImageHelper::getSupportedImageFormatsFilter() const
{
    auto formats = QImageReader::supportedImageFormats();
    QString imageFilter = QStringLiteral("Image files (");
    for (const auto &format : std::as_const(formats))
        imageFilter += QStringLiteral("*.") + format + QStringLiteral(" ");
    imageFilter += QStringLiteral(")");
    return imageFilter;
}
