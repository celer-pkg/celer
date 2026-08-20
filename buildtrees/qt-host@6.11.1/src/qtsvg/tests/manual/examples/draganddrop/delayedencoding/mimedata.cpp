// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>
#include "mimedata.h"

MimeData::MimeData()
    : QMimeData()
{
}

//![0]
QStringList MimeData::formats() const
{
    return QMimeData::formats() << "image/png";
}
//![0]

//![1]
QVariant MimeData::retrieveData(const QString &mimeType, QMetaType type)
         const
{
    emit dataRequested(mimeType);

    return QMimeData::retrieveData(mimeType, type);
}
//![1]

