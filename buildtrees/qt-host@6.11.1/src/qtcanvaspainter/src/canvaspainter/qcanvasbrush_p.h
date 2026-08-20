// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASBRUSH_P_H
#define QCANVASBRUSH_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qcanvasbrush.h"

QT_BEGIN_NAMESPACE

class QCanvasBrushPrivate : public QSharedData
{
public:
    QCanvasBrushPrivate(QCanvasBrush::BrushType t) : type(t) {}
    virtual ~QCanvasBrushPrivate() = default;
    QCanvasBrush::BrushType type;
    virtual QCanvasBrushPrivate *clone() = 0;
    virtual QCPaint createPaint(QCanvasPainter *painter) const = 0;
};

template <>
inline QCanvasBrushPrivate *QExplicitlySharedDataPointer<QCanvasBrushPrivate>::clone()
{
    return d ? d->clone() : nullptr;
}

QT_END_NAMESPACE

#endif // QCANVASBRUSH_P_H
