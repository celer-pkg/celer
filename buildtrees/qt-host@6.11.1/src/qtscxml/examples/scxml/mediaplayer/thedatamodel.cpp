// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "thedatamodel.h"

#include <QtScxml/qscxmlevent.h>

using namespace Qt::Literals::StringLiterals;

bool TheDataModel::isValidMedia() const
{
    QString eventMedia = eventData().value(u"media"_s).toString();
    return eventMedia.size() > 0;
}

QVariantMap TheDataModel::eventData() const
{
    return scxmlEvent().data().value<QVariantMap>();
}
