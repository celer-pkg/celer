// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QSCATTER3DSERIES_P_H
#define QSCATTER3DSERIES_P_H

#include "qabstract3dseries_p.h"
#include "qscatter3dseries.h"

QT_BEGIN_NAMESPACE

class QScatter3DSeriesPrivate : public QAbstract3DSeriesPrivate
{
    Q_DECLARE_PUBLIC(QScatter3DSeries)

public:
    QScatter3DSeriesPrivate();
    ~QScatter3DSeriesPrivate() override;

    void setDataProxy(QAbstractDataProxy *proxy) override;
    void connectGraphAndProxy(QQuickGraphsItem *newGraph) override;
    void createItemLabel() override;

    void setSelectedItem(qsizetype index);
    void setItemSize(float size);

    void setDataArray(const QScatterDataArray &newDataArray);
    void clearArray();

    void setScaleArray(const QList<QVector3D> &newScaleArray);
    void clearScaleArray();

    void setAxisX(QValue3DAxis *axis);
    void setAxisY(QValue3DAxis *axis);
    void setAxisZ(QValue3DAxis *axis);
    void resetAxisX();
    void resetAxisY();
    void resetAxisZ();

private:
    qsizetype m_selectedItem;
    float m_itemSize;
    QScatterDataArray m_dataArray;
    QList<QVector3D> m_scaleArray;

    QValue3DAxis *m_axisX = nullptr;
    QValue3DAxis *m_axisY = nullptr;
    QValue3DAxis *m_axisZ = nullptr;

    friend class QQuickGraphsScatter;
};

QT_END_NAMESPACE

#endif
