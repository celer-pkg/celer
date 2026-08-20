// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_Q3DBARS_H
#define QTDATAVIS3D_Q3DBARS_H

#include <QtDataVisualization/qabstract3dgraph.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/qcategory3daxis.h>
#include <QtDataVisualization/qbar3dseries.h>

QT_BEGIN_NAMESPACE

class Q3DBarsPrivate;

class Q_DATAVISUALIZATION_EXPORT Q3DBars : public QAbstract3DGraph
{
    Q_OBJECT
    Q_PROPERTY(bool multiSeriesUniform READ isMultiSeriesUniform WRITE setMultiSeriesUniform NOTIFY multiSeriesUniformChanged)
    Q_PROPERTY(float barThickness READ barThickness WRITE setBarThickness NOTIFY barThicknessChanged)
    Q_PROPERTY(QSizeF barSpacing READ barSpacing WRITE setBarSpacing NOTIFY barSpacingChanged)
    Q_PROPERTY(bool barSpacingRelative READ isBarSpacingRelative WRITE setBarSpacingRelative NOTIFY barSpacingRelativeChanged)
    Q_PROPERTY(QSizeF barSeriesMargin READ barSeriesMargin WRITE setBarSeriesMargin NOTIFY barSeriesMarginChanged REVISION(6, 3))
    Q_PROPERTY(QCategory3DAxis *rowAxis READ rowAxis WRITE setRowAxis NOTIFY rowAxisChanged)
    Q_PROPERTY(QCategory3DAxis *columnAxis READ columnAxis WRITE setColumnAxis NOTIFY columnAxisChanged)
    Q_PROPERTY(QValue3DAxis *valueAxis READ valueAxis WRITE setValueAxis NOTIFY valueAxisChanged)
    Q_PROPERTY(QBar3DSeries *primarySeries READ primarySeries WRITE setPrimarySeries NOTIFY primarySeriesChanged)
    Q_PROPERTY(QBar3DSeries *selectedSeries READ selectedSeries NOTIFY selectedSeriesChanged)
    Q_PROPERTY(float floorLevel READ floorLevel WRITE setFloorLevel NOTIFY floorLevelChanged)

public:
    explicit Q3DBars(const QSurfaceFormat *format = nullptr, QWindow *parent = nullptr);
    virtual ~Q3DBars();

    void setPrimarySeries(QBar3DSeries *series);
    QBar3DSeries *primarySeries() const;
    void addSeries(QBar3DSeries *series);
    void removeSeries(QBar3DSeries *series);
    void insertSeries(int index, QBar3DSeries *series);
    QList<QBar3DSeries *> seriesList() const;

    void setMultiSeriesUniform(bool uniform);
    bool isMultiSeriesUniform() const;

    void setBarThickness(float thicknessRatio);
    float barThickness() const;

    void setBarSpacing(const QSizeF &spacing);
    QSizeF barSpacing() const;

    void setBarSpacingRelative(bool relative);
    bool isBarSpacingRelative() const;

    void setBarSeriesMargin(const QSizeF &margin);
    QSizeF barSeriesMargin() const;

    void setRowAxis(QCategory3DAxis *axis);
    QCategory3DAxis *rowAxis() const;
    void setColumnAxis(QCategory3DAxis *axis);
    QCategory3DAxis *columnAxis() const;
    void setValueAxis(QValue3DAxis *axis);
    QValue3DAxis *valueAxis() const;
    void addAxis(QAbstract3DAxis *axis);
    void releaseAxis(QAbstract3DAxis *axis);
    QList<QAbstract3DAxis *> axes() const;

    QBar3DSeries *selectedSeries() const;
    void setFloorLevel(float level);
    float floorLevel() const;

Q_SIGNALS:
    void multiSeriesUniformChanged(bool uniform);
    void barThicknessChanged(float thicknessRatio);
    void barSpacingChanged(const QSizeF &spacing);
    void barSpacingRelativeChanged(bool relative);
    Q_REVISION(6, 3) void barSeriesMarginChanged(const QSizeF &margin);
    void rowAxisChanged(QCategory3DAxis *axis);
    void columnAxisChanged(QCategory3DAxis *axis);
    void valueAxisChanged(QValue3DAxis *axis);
    void primarySeriesChanged(QBar3DSeries *series);
    void selectedSeriesChanged(QBar3DSeries *series);
    void floorLevelChanged(float level);

private:
    Q3DBarsPrivate *dptr();
    const Q3DBarsPrivate *dptrc() const;
    Q_DISABLE_COPY(Q3DBars)
};

QT_END_NAMESPACE

#endif
