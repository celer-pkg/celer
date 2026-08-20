// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QBARDATAITEM_H
#define QTGRAPHS_QBARDATAITEM_H

#include <QtGraphs/qgraphsglobal.h>

QT_BEGIN_NAMESPACE

class QBarDataItem
{
public:
    constexpr QBarDataItem() noexcept = default;
    explicit constexpr QBarDataItem(float value) noexcept
        : m_value(value)
    {}
    explicit constexpr QBarDataItem(float value, float angle) noexcept
        : m_value(value)
        , m_angle(angle)
    {}

    constexpr void setValue(float val) noexcept { m_value = val; }
    constexpr float value() const noexcept { return m_value; }
    constexpr void setRotation(float angle) noexcept { m_angle = angle; }
    constexpr float rotation() const noexcept { return m_angle; }

private:
    float m_value = 0.f;
    float m_angle = 0.f;
};

QT_END_NAMESPACE

#endif
