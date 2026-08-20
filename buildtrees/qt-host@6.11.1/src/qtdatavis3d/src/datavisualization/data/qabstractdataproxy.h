// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QABSTRACTDATAPROXY_H
#define QTDATAVIS3D_QABSTRACTDATAPROXY_H

#include <QtDataVisualization/qdatavisualizationglobal.h>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE

class QAbstractDataProxyPrivate;

class Q_DATAVISUALIZATION_EXPORT QAbstractDataProxy : public QObject
{
    Q_OBJECT
    Q_PROPERTY(DataType type READ type CONSTANT)

public:
    enum DataType {
        DataTypeNone = 0,
        DataTypeBar = 1,
        DataTypeScatter = 2,
        DataTypeSurface = 4
    };
    Q_ENUM(DataType)

protected:
    explicit QAbstractDataProxy(QAbstractDataProxyPrivate *d, QObject *parent = nullptr);

public:
    virtual ~QAbstractDataProxy();

    DataType type() const;

protected:
    QScopedPointer<QAbstractDataProxyPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QAbstractDataProxy)

    friend class QAbstract3DSeriesPrivate;
};

QT_END_NAMESPACE

#endif
