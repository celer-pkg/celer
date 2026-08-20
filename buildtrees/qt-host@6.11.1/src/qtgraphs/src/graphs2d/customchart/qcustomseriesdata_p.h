// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Graphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QCUSTOMSERIESDATA_H
#define QCUSTOMSERIESDATA_H

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE

class QCustomSeriesData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap data READ data WRITE setData FINAL)

    QML_NAMED_ELEMENT(CustomSeriesData)
    QML_ADDED_IN_VERSION(6, 11)

public:
    explicit QCustomSeriesData(QObject *parent = 0);
    QVariantMap data() const;
    void setData(const QVariantMap &newData);

private:
    QVariantMap m_data;
};

QT_END_NAMESPACE

#endif // QCUSTOMSERIESDATA_H
