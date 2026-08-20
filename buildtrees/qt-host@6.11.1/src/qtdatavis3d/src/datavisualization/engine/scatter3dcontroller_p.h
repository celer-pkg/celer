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

#ifndef Q3DSCATTERCONTROLLER_p_H
#define Q3DSCATTERCONTROLLER_p_H

#include <private/datavisualizationglobal_p.h>
#include <private/abstract3dcontroller_p.h>

QT_BEGIN_NAMESPACE

class Scatter3DRenderer;
class QScatterDataProxy;
class QScatter3DSeries;

struct Scatter3DChangeBitField {
    bool selectedItemChanged : 1;
    bool itemChanged         : 1;

    Scatter3DChangeBitField() :
        selectedItemChanged(true),
        itemChanged(false)
    {
    }
};

class Q_DATAVISUALIZATION_EXPORT Scatter3DController : public Abstract3DController
{
    Q_OBJECT

public:
    struct ChangeItem {
        QScatter3DSeries *series;
        int index;
    };
private:
    Scatter3DChangeBitField m_changeTracker;
    QList<ChangeItem> m_changedItems;

    // Rendering
    Scatter3DRenderer *m_renderer;
    int m_selectedItem;
    QScatter3DSeries *m_selectedItemSeries; // Points to the series for which the bar is selected
                                            // in single series selection cases.

    struct InsertRemoveRecord {
        bool m_isInsert;
        int m_startIndex;
        int m_count;
        QAbstract3DSeries *m_series;

        InsertRemoveRecord() :
            m_isInsert(false),
            m_startIndex(0),
            m_count(0),
            m_series(0)
        {}

        InsertRemoveRecord(bool isInsert, int startIndex, int count, QAbstract3DSeries *series) :
            m_isInsert(isInsert),
            m_startIndex(startIndex),
            m_count(count),
            m_series(series)
        {}
    };

    QList<InsertRemoveRecord> m_insertRemoveRecords;
    bool m_recordInsertsAndRemoves;

public:
    explicit Scatter3DController(QRect rect, Q3DScene *scene = 0);
    ~Scatter3DController();

    void initializeOpenGL() override;

    // Change selection mode
    void setSelectionMode(QAbstract3DGraph::SelectionFlags mode) override;

    inline QScatter3DSeries *selectedSeries() const { return m_selectedItemSeries; }

    void setSelectedItem(int index, QScatter3DSeries *series);
    static inline int invalidSelectionIndex() { return -1; }
    void clearSelection() override;

    void synchDataToRenderer() override;

    void addSeries(QAbstract3DSeries *series) override;
    void removeSeries(QAbstract3DSeries *series) override;
    virtual QList<QScatter3DSeries *> scatterSeriesList();

    void handleAxisAutoAdjustRangeChangedInOrientation(
            QAbstract3DAxis::AxisOrientation orientation, bool autoAdjust) override;
    void handleAxisRangeChangedBySender(QObject *sender) override;
    void handlePendingClick() override;
    void adjustAxisRanges() override;

public Q_SLOTS:
    void handleArrayReset();
    void handleItemsAdded(int startIndex, int count);
    void handleItemsChanged(int startIndex, int count);
    void handleItemsRemoved(int startIndex, int count);
    void handleItemsInserted(int startIndex, int count);

Q_SIGNALS:
    void selectedSeriesChanged(QScatter3DSeries *series);

protected:
    void startRecordingRemovesAndInserts() override;

private:

    Q_DISABLE_COPY(Scatter3DController)
};

QT_END_NAMESPACE

#endif
