// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef THEDATAMODEL_H
#define THEDATAMODEL_H

#include "qscxmlcppdatamodel.h"
#include <QtQml/qqml.h>

class TheDataModel: public QScxmlCppDataModel
{
    Q_OBJECT
    Q_SCXML_DATAMODEL
    QML_NAMED_ELEMENT(MediaPlayerDataModel)
    QML_ADDED_IN_VERSION(1, 0)

private:
    bool isValidMedia() const;
    QVariantMap eventData() const;

    QString media;
};

#endif // THEDATAMODEL_H
