// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef Q3DBARSCONTROLLER_p_H
#define Q3DBARSCONTROLLER_p_H

#include <private/datavisualizationglobal_p.h>
#include <private/abstract3dcontroller_p.h>

QT_BEGIN_NAMESPACE

class Bars3DRenderer;
class QBar3DSeries;

struct Bars3DChangeBitField {
    bool multiSeriesScalingChanged  : 1;
    bool barSpecsChanged            : 1;
    bool selectedBarChanged         : 1;
    bool rowsChanged                : 1;
    bool itemChanged                : 1;
    bool floorLevelChanged          : 1;
    bool barSeriesMarginChanged     : 1;

    Bars3DChangeBitField() :
        multiSeriesScalingChanged(true),
        barSpecsChanged(true),
        selectedBarChanged(true),
        rowsChanged(false),
        itemChanged(false),
        floorLevelChanged(false),
        barSeriesMarginChanged(false)
    {
    }
};

class Q_DATAVISUALIZATION_EXPORT Bars3DController : public Abstract3DController
{
    Q_OBJECT

public:
    struct ChangeItem {
        QBar3DSeries *series;
        QPoint point;
    };
    struct ChangeRow {
        QBar3DSeries *series;
        int row;
    };

private:
    Bars3DChangeBitField m_changeTracker;
    QList<ChangeItem> m_changedItems;
    QList<ChangeRow> m_changedRows;

    // Interaction
    QPoint m_selectedBar;     // Points to row & column in data window.
    QBar3DSeries *m_selectedBarSeries; // Points to the series for which the bar is selected in
                                       // single series selection cases.
    QBar3DSeries *m_primarySeries; // Category axis labels are taken from the primary series

    // Look'n'feel
    bool m_isMultiSeriesUniform;
    bool m_isBarSpecRelative;
    GLfloat m_barThicknessRatio;
    QSizeF m_barSpacing;
    float m_floorLevel;
    QSizeF m_barSeriesMargin;

    // Rendering
    Bars3DRenderer *m_renderer;

public:
    explicit Bars3DController(QRect rect, Q3DScene *scene = 0);
    ~Bars3DController();

    void initializeOpenGL() override;
    void synchDataToRenderer() override;

    void setMultiSeriesScaling(bool uniform);
    bool multiSeriesScaling() const;

    // bar thickness, spacing between bars, and is spacing relative to thickness or absolute
    // y -component sets the thickness/spacing of z -direction
    // With relative 0.0f means side-to-side, 1.0f = one thickness in between
    void setBarSpecs(GLfloat thicknessRatio = 1.0f,
                     const QSizeF &spacing = QSizeF(1.0, 1.0),
                     bool relative = true);
    void setBarSeriesMargin(const QSizeF &margin);
    QSizeF barSeriesMargin();

    GLfloat barThickness();
    QSizeF barSpacing();
    bool isBarSpecRelative();
    void setFloorLevel(float level);
    float floorLevel() const;

    inline QBar3DSeries *selectedSeries() const { return m_selectedBarSeries; }

    void setSelectionMode(QAbstract3DGraph::SelectionFlags mode) override;
    void setSelectedBar(const QPoint &position, QBar3DSeries *series, bool enterSlice);
    void clearSelection() override;

    void handleAxisAutoAdjustRangeChangedInOrientation(
            QAbstract3DAxis::AxisOrientation orientation, bool autoAdjust) override;
    void handleSeriesVisibilityChangedBySender(QObject *sender) override;
    void handlePendingClick() override;

    static QPoint invalidSelectionPosition();

    void setAxisX(QAbstract3DAxis *axis) override;
    void setAxisZ(QAbstract3DAxis *axis) override;

    virtual void setPrimarySeries(QBar3DSeries *series);
    virtual QBar3DSeries *primarySeries() const;
    void addSeries(QAbstract3DSeries *series) override;
    void removeSeries(QAbstract3DSeries *series) override;
    void insertSeries(int index, QAbstract3DSeries *series) override;
    virtual QList<QBar3DSeries *> barSeriesList();

    void handleAxisRangeChangedBySender(QObject *sender) override;
    void adjustAxisRanges() override;

public Q_SLOTS:
    void handleArrayReset();
    void handleRowsAdded(int startIndex, int count);
    void handleRowsChanged(int startIndex, int count);
    void handleRowsRemoved(int startIndex, int count);
    void handleRowsInserted(int startIndex, int count);
    void handleItemChanged(int rowIndex, int columnIndex);
    void handleDataRowLabelsChanged();
    void handleDataColumnLabelsChanged();
    void handleRowColorsChanged();

Q_SIGNALS:
    void primarySeriesChanged(QBar3DSeries *series);
    void selectedSeriesChanged(QBar3DSeries *series);

protected:
    QAbstract3DAxis *createDefaultAxis(QAbstract3DAxis::AxisOrientation orientation) override;

private:
    void adjustSelectionPosition(QPoint &pos, const QBar3DSeries *series);

    Q_DISABLE_COPY(Bars3DController)
};

QT_END_NAMESPACE

#endif
