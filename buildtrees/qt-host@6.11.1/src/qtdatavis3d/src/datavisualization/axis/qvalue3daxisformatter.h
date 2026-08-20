// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QVALUE3DAXISFORMATTER_H
#define QTDATAVIS3D_QVALUE3DAXISFORMATTER_H

#include <QtDataVisualization/qdatavisualizationglobal.h>
#include <QtCore/QList>
#include <QtCore/QLocale>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QValue3DAxisFormatterPrivate;
class QValue3DAxis;

class Q_DATAVISUALIZATION_EXPORT QValue3DAxisFormatter : public QObject
{
    Q_OBJECT
protected:
    explicit QValue3DAxisFormatter(QValue3DAxisFormatterPrivate *d, QObject *parent = nullptr);
public:
    explicit QValue3DAxisFormatter(QObject *parent = nullptr);
    virtual ~QValue3DAxisFormatter();

protected:
    void setAllowNegatives(bool allow);
    bool allowNegatives() const;
    void setAllowZero(bool allow);
    bool allowZero() const;

    virtual QValue3DAxisFormatter *createNewInstance() const;
    virtual void recalculate();
    virtual QString stringForValue(qreal value, const QString &format) const;
    virtual float positionAt(float value) const;
    virtual float valueAt(float position) const;
    virtual void populateCopy(QValue3DAxisFormatter &copy) const;

    void markDirty(bool labelsChange = false);
    QValue3DAxis *axis() const;

    QList<float> &gridPositions() const;
    QList<float> &subGridPositions() const;
    QList<float> &labelPositions() const;
    QStringList &labelStrings() const;

    void setLocale(const QLocale &locale);
    QLocale locale() const;

    QScopedPointer<QValue3DAxisFormatterPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QValue3DAxisFormatter)

    friend class Abstract3DController;
    friend class Abstract3DRenderer;
    friend class Bars3DRenderer;
    friend class Scatter3DRenderer;
    friend class Surface3DRenderer;
    friend class SurfaceObject;
    friend class QValue3DAxisFormatterPrivate;
    friend class QLogValue3DAxisFormatter;
    friend class QValue3DAxis;
    friend class QValue3DAxisPrivate;
    friend class AxisRenderCache;
    friend class QBar3DSeriesPrivate;
    friend class QScatter3DSeriesPrivate;
    friend class QSurface3DSeriesPrivate;
};

QT_END_NAMESPACE

#endif
