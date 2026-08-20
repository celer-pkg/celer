// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrenderinstancetable_p.h"

QMatrix4x4 QSSGRenderInstanceTable::getTransform(int index) const
{
    Q_ASSERT(index < instanceCount);
    // NOTE: table size can be bigger than instanceCount due to QQuick3DInstancing::instanceCountOverride
    Q_ASSERT(table.size() >= instanceStride * (index + 1));
    auto *entry = reinterpret_cast<const QSSGRenderInstanceTableEntry*>(table.constData() + index * instanceStride);

    QMatrix4x4 res;
    res.setRow(0, entry->row0);
    res.setRow(1, entry->row1);
    res.setRow(2, entry->row2);
    res.setRow(3, { 0, 0, 0, 1 });
    return res;
}
