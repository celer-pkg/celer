// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QVALUE3DAXIS_H
#define QTDATAVIS3D_QVALUE3DAXIS_H

#include <QtDataVisualization/qabstract3daxis.h>
#include <QtDataVisualization/qvalue3daxisformatter.h>

QT_BEGIN_NAMESPACE

class QValue3DAxisPrivate;

class Q_DATAVISUALIZATION_EXPORT QValue3DAxis : public QAbstract3DAxis
{
    Q_OBJECT
    Q_PROPERTY(int segmentCount READ segmentCount WRITE setSegmentCount NOTIFY segmentCountChanged)
    Q_PROPERTY(int subSegmentCount READ subSegmentCount WRITE setSubSegmentCount NOTIFY subSegmentCountChanged)
    Q_PROPERTY(QString labelFormat READ labelFormat WRITE setLabelFormat NOTIFY labelFormatChanged)
    Q_PROPERTY(QValue3DAxisFormatter* formatter READ formatter WRITE setFormatter NOTIFY formatterChanged REVISION(1, 1))
    Q_PROPERTY(bool reversed READ reversed WRITE setReversed NOTIFY reversedChanged REVISION(1, 1))

public:
    explicit QValue3DAxis(QObject *parent = nullptr);
    virtual ~QValue3DAxis();

    void setSegmentCount(int count);
    int segmentCount() const;

    void setSubSegmentCount(int count);
    int subSegmentCount() const;

    void setLabelFormat(const QString &format);
    QString labelFormat() const;

    void setFormatter(QValue3DAxisFormatter *formatter);
    QValue3DAxisFormatter *formatter() const;

    void setReversed(bool enable);
    bool reversed() const;

Q_SIGNALS:
    void segmentCountChanged(int count);
    void subSegmentCountChanged(int count);
    void labelFormatChanged(const QString &format);
    Q_REVISION(1, 1) void formatterChanged(QValue3DAxisFormatter *formatter);
    Q_REVISION(1, 1) void reversedChanged(bool enable);

protected:
    QValue3DAxisPrivate *dptr();
    const QValue3DAxisPrivate *dptrc() const;

private:
    Q_DISABLE_COPY(QValue3DAxis)
    friend class Abstract3DController;
    friend class Bars3DController;
    friend class Scatter3DController;
    friend class Surface3DController;
    friend class QValue3DAxisFormatterPrivate;
};

QT_END_NAMESPACE

#endif
