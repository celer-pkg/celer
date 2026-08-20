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

#ifndef BARINSTANCING_H
#define BARINSTANCING_H
#include <QtGraphs/qgraphsglobal.h>
#include <private/qquick3dinstancing_p.h>

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

struct BarItemHolder
{
    QVector3D position = {.0f, .0f, .0f};
    QQuaternion rotation = {1.f, .0f, .0f, .0f};
    QVector3D scale = {.0f, .0f, .0f};
    QPoint coord;
    float heightValue = .0f;
    bool selectedBar = false;
    QColor color = {0, 0, 0};
};

class Q_GRAPHS_EXPORT BarInstancing : public QQuick3DInstancing
{
    Q_OBJECT
public:
    BarInstancing();
    ~BarInstancing();

    QList<BarItemHolder *> dataArray() const;
    void setDataArray(const QList<BarItemHolder *> &newDataArray);

    void markDataDirty();
    bool transparency() const;
    void setTransparency(bool newTransparencyValue);

    void clearDataArray();

protected:
    QByteArray getInstanceBuffer(int *instanceCount) override;

private:
    QByteArray m_instanceData;
    QList<BarItemHolder *> m_dataArray;
    int m_instanceCount = 0;
    bool m_dirty = true;
    bool m_transparency = false;
};

QT_END_NAMESPACE

#endif // BARINSTANCING_H
