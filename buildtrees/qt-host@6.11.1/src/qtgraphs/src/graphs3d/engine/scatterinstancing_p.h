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

#ifndef SCATTERINSTANCING_H
#define SCATTERINSTANCING_H

#include <QtGraphs/qgraphsglobal.h>
#include <private/qquick3dinstancing_p.h>

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

struct DataItemHolder
{
    QVector3D position;
    QQuaternion rotation;
    QVector3D scale;
    bool hide = false;
};

class Q_GRAPHS_EXPORT ScatterInstancing : public QQuick3DInstancing
{
    Q_OBJECT
public:
    ScatterInstancing();

    const QList<DataItemHolder> &dataArray() const;
    void setDataArray(const QList<DataItemHolder> &newDataArray);
    void hideDataItem(qsizetype index);
    void unhidePreviousDataItem();
    void resetVisibilty();

    const QList<float> &customData() const;
    void setCustomData(const QList<float> &newCustomData);

    void markDataDirty();
    bool rangeGradient() const;
    void setRangeGradient(bool newRangeGradient);

    void setTransparency(bool transparency);

    bool isDirty() const { return m_dirty; }

    // QQuick3DInstancing interface

protected:
    QByteArray getInstanceBuffer(int *instanceCount) override;

private:
    QByteArray m_instanceData;
    QList<DataItemHolder> m_dataArray;
    QList<float> m_customData;
    int m_instanceCount = 0;
    bool m_dirty = true;
    bool m_rangeGradient = false;
    qsizetype m_previousHideIndex = -1;
};

QT_END_NAMESPACE

#endif // SCATTERINSTANCING_H
