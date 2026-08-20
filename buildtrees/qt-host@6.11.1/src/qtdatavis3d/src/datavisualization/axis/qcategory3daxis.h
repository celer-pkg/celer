// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QCATEGORY3DAXIS_H
#define QTDATAVIS3D_QCATEGORY3DAXIS_H

#include <QtDataVisualization/qabstract3daxis.h>

QT_BEGIN_NAMESPACE

class QCategory3DAxisPrivate;

class Q_DATAVISUALIZATION_EXPORT QCategory3DAxis : public QAbstract3DAxis
{
    Q_OBJECT
    Q_PROPERTY(QStringList labels READ labels WRITE setLabels NOTIFY labelsChanged)

public:
    explicit QCategory3DAxis(QObject *parent = nullptr);
    virtual ~QCategory3DAxis();

    void setLabels(const QStringList &labels);
    QStringList labels() const;

Q_SIGNALS:
    void labelsChanged();

protected:
    QCategory3DAxisPrivate *dptr();

private:
    Q_DISABLE_COPY(QCategory3DAxis)
    friend class Bars3DController;
};

QT_END_NAMESPACE

#endif
