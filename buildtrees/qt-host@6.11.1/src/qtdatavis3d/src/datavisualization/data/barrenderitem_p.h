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

#ifndef BARRENDERITEM_P_H
#define BARRENDERITEM_P_H

#include "abstractrenderitem_p.h"

QT_BEGIN_NAMESPACE

class BarRenderItem : public AbstractRenderItem
{
public:
    BarRenderItem();
    BarRenderItem(const BarRenderItem &other);
    BarRenderItem &operator=(const BarRenderItem &) = default;
    virtual ~BarRenderItem();

    // Position relative to data window (for bar label generation)
    inline void setPosition(const QPoint &pos) { m_position = pos; }
    inline const QPoint &position() const { return m_position; }

    // Actual cached data value of the bar (needed to trigger label reformats)
    inline void setValue(float value) { m_value = value; }
    inline float value() const { return m_value; }

    // Normalized bar height
    inline void setHeight(GLfloat height) { m_height = height; }
    inline GLfloat height() const { return m_height; }

protected:
    float m_value;
    QPoint m_position; // x = row, y = column
    GLfloat m_height;

    friend class QBarDataItem;
};

class BarRenderSliceItem : public BarRenderItem
{
public:
    BarRenderSliceItem();
    BarRenderSliceItem(const BarRenderSliceItem &other);
    BarRenderSliceItem &operator=(const BarRenderSliceItem &other) = default;
    virtual ~BarRenderSliceItem();

    void setItem(const BarRenderItem &renderItem);

    // Label item for formatted label
    LabelItem &sliceLabelItem();

    // Formatted label for item.
    void setSliceLabel(const QString &label);
    const QString &sliceLabel() const; // Formats label if not previously formatted

protected:
    QString m_sliceLabel;
    LabelItem *m_sliceLabelItem;
    bool m_isNull;
};

typedef QList<BarRenderItem> BarRenderItemRow;
typedef QList<BarRenderItemRow> BarRenderItemArray;

QT_END_NAMESPACE

#endif
