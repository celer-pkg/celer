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

#ifndef SCATTERRENDERITEM_P_H
#define SCATTERRENDERITEM_P_H

#include "abstractrenderitem_p.h"

QT_BEGIN_NAMESPACE

class ScatterRenderItem : public AbstractRenderItem
{
public:
    ScatterRenderItem();
    ScatterRenderItem(const ScatterRenderItem &other);
    ScatterRenderItem &operator=(const ScatterRenderItem &) = default;
    virtual ~ScatterRenderItem();

    inline const QVector3D &position() const { return m_position; }
    inline void setPosition(const QVector3D &pos)
    {
        if (m_position != pos)
            m_position = pos;
    }

    inline bool isVisible() const { return m_visible; }
    inline void setVisible(bool visible) { m_visible = visible; }

protected:
    QVector3D m_position;
    bool m_visible;
};
typedef QList<ScatterRenderItem> ScatterRenderItemArray;

QT_END_NAMESPACE

#endif
