// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef THEDATAMODEL_H
#define THEDATAMODEL_H

//! [Declaration1]
#include <QScxmlCppDataModel>

class TheDataModel: public QScxmlCppDataModel
{
    Q_OBJECT
    Q_SCXML_DATAMODEL
//! [Declaration1]

private:
    bool isValidMedia() const;
    QVariantMap eventData() const;

    QString media;
//! [Declaration2]
};
//! [Declaration2]

#endif // THEDATAMODEL_H
