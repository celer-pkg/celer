// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QABSTRACT3DAXIS_H
#define QTDATAVIS3D_QABSTRACT3DAXIS_H

#include <QtDataVisualization/qdatavisualizationglobal.h>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QAbstract3DAxisPrivate;

class Q_DATAVISUALIZATION_EXPORT QAbstract3DAxis : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QStringList labels READ labels WRITE setLabels NOTIFY labelsChanged)
    Q_PROPERTY(AxisOrientation orientation READ orientation NOTIFY orientationChanged)
    Q_PROPERTY(AxisType type READ type CONSTANT)
    Q_PROPERTY(float min READ min WRITE setMin NOTIFY minChanged)
    Q_PROPERTY(float max READ max WRITE setMax NOTIFY maxChanged)
    Q_PROPERTY(bool autoAdjustRange READ isAutoAdjustRange WRITE setAutoAdjustRange NOTIFY autoAdjustRangeChanged)
    Q_PROPERTY(float labelAutoRotation READ labelAutoRotation WRITE setLabelAutoRotation NOTIFY labelAutoRotationChanged REVISION(1, 1))
    Q_PROPERTY(bool titleVisible READ isTitleVisible WRITE setTitleVisible NOTIFY titleVisibilityChanged REVISION(1, 1))
    Q_PROPERTY(bool titleFixed READ isTitleFixed WRITE setTitleFixed NOTIFY titleFixedChanged REVISION(1, 1))

public:
    enum AxisOrientation {
        AxisOrientationNone = 0,
        AxisOrientationX = 1,
        AxisOrientationY = 2,
        AxisOrientationZ = 4
    };
    Q_ENUM(AxisOrientation)

    enum AxisType {
        AxisTypeNone = 0,
        AxisTypeCategory = 1,
        AxisTypeValue = 2
    };
    Q_ENUM(AxisType)

protected:
    explicit QAbstract3DAxis(QAbstract3DAxisPrivate *d, QObject *parent = nullptr);

public:
    virtual ~QAbstract3DAxis();

    void setTitle(const QString &title);
    QString title() const;

    void setLabels(const QStringList &labels);
    QStringList labels() const;

    AxisOrientation orientation() const;
    AxisType type() const;

    void setMin(float min);
    float min() const;

    void setMax(float max);
    float max() const;

    void setAutoAdjustRange(bool autoAdjust);
    bool isAutoAdjustRange() const;

    void setRange(float min, float max);

    void setLabelAutoRotation(float angle);
    float labelAutoRotation() const;

    void setTitleVisible(bool visible);
    bool isTitleVisible() const;

    void setTitleFixed(bool fixed);
    bool isTitleFixed() const;

Q_SIGNALS:
    void titleChanged(const QString &newTitle);
    void labelsChanged();
    void orientationChanged(QAbstract3DAxis::AxisOrientation orientation);
    void minChanged(float value);
    void maxChanged(float value);
    void rangeChanged(float min, float max);
    void autoAdjustRangeChanged(bool autoAdjust);
    Q_REVISION(1, 1) void labelAutoRotationChanged(float angle);
    Q_REVISION(1, 1) void titleVisibilityChanged(bool visible);
    Q_REVISION(1, 1) void titleFixedChanged(bool fixed);

protected:
    QScopedPointer<QAbstract3DAxisPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QAbstract3DAxis)

    friend class Abstract3DController;
    friend class Bars3DController;
    friend class QScatterDataProxyPrivate;
    friend class QSurfaceDataProxyPrivate;
};

QT_END_NAMESPACE

#endif
