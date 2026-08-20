// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_Q3DBARSWIDGETITEM_H
#define QTGRAPHS_Q3DBARSWIDGETITEM_H

#include <QtGraphs/qbar3dseries.h>
#include <QtGraphs/qcategory3daxis.h>
#include <QtGraphs/qvalue3daxis.h>
#include <QtGraphsWidgets/q3dgraphswidgetitem.h>

QT_BEGIN_NAMESPACE

class QQuickGraphsBars;
class Q3DBarsWidgetItemPrivate;

class Q_GRAPHSWIDGETS_EXPORT Q3DBarsWidgetItem : public Q3DGraphsWidgetItem
{
    Q_OBJECT
    Q_PROPERTY(bool multiSeriesUniform READ isMultiSeriesUniform WRITE setMultiSeriesUniform NOTIFY
                   multiSeriesUniformChanged)
    Q_PROPERTY(float barThickness READ barThickness WRITE setBarThickness NOTIFY barThicknessChanged)
    Q_PROPERTY(QSizeF barSpacing READ barSpacing WRITE setBarSpacing NOTIFY barSpacingChanged)
    Q_PROPERTY(bool barSpacingRelative READ isBarSpacingRelative WRITE setBarSpacingRelative NOTIFY
                   barSpacingRelativeChanged)
    Q_PROPERTY(QSizeF barSeriesMargin READ barSeriesMargin WRITE setBarSeriesMargin NOTIFY
                   barSeriesMarginChanged)
    Q_PROPERTY(QCategory3DAxis *rowAxis READ rowAxis WRITE setRowAxis NOTIFY rowAxisChanged)
    Q_PROPERTY(
        QCategory3DAxis *columnAxis READ columnAxis WRITE setColumnAxis NOTIFY columnAxisChanged)
    Q_PROPERTY(QValue3DAxis *valueAxis READ valueAxis WRITE setValueAxis NOTIFY valueAxisChanged)
    Q_PROPERTY(QBar3DSeries *primarySeries READ primarySeries WRITE setPrimarySeries NOTIFY
                   primarySeriesChanged)
    Q_PROPERTY(QBar3DSeries *selectedSeries READ selectedSeries NOTIFY selectedSeriesChanged)
    Q_PROPERTY(float floorLevel READ floorLevel WRITE setFloorLevel NOTIFY floorLevelChanged)

public:
    explicit Q3DBarsWidgetItem(QObject *parent = nullptr);
    ~Q3DBarsWidgetItem() override;

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

    void setBarSpacing(QSizeF spacing);
    QSizeF barSpacing() const;

    void setBarSpacingRelative(bool relative);
    bool isBarSpacingRelative() const;

    void setBarSeriesMargin(QSizeF margin);
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

    Q_REVISION(6, 10)
    void renderSliceToImage(int requestedIndex, QtGraphs3D::SliceCaptureType sliceType);

protected:
    bool event(QEvent *event) override;

Q_SIGNALS:
    void multiSeriesUniformChanged(bool uniform);
    void barThicknessChanged(float thicknessRatio);
    void barSpacingChanged(QSizeF spacing);
    void barSpacingRelativeChanged(bool relative);
    void barSeriesMarginChanged(QSizeF margin);
    void rowAxisChanged(QCategory3DAxis *axis);
    void columnAxisChanged(QCategory3DAxis *axis);
    void valueAxisChanged(QValue3DAxis *axis);
    void primarySeriesChanged(QBar3DSeries *series);
    void selectedSeriesChanged(QBar3DSeries *series);
    void floorLevelChanged(float level);
    Q_REVISION(6, 10)
    void sliceImageChanged(const QImage &image);

private:
    Q_DECLARE_PRIVATE(Q3DBarsWidgetItem)
    Q_DISABLE_COPY_MOVE(Q3DBarsWidgetItem)
    QQuickGraphsBars *graphBars();
    const QQuickGraphsBars *graphBars() const;
};

QT_END_NAMESPACE

#endif
