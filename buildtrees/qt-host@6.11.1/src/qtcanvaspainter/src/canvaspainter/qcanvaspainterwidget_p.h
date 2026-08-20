// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPAINTERWIDGET_P_H
#define QCANVASPAINTERWIDGET_P_H

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

#include <QtWidgets/private/qrhiwidget_p.h>
#include "qcanvaspainterwidget.h"
#include "engine/qcpainterengineutils_p.h"
#include "qcanvasdebugcountervisualizer_p.h"
#include <QAtomicInt>

QT_BEGIN_NAMESPACE

class QCanvasPainterFactory;

class QCanvasPainterWidgetPrivate : public QRhiWidgetPrivate
{
    Q_DECLARE_PUBLIC(QCanvasPainterWidget)

public:
    bool m_sharedPainter = true;
    QCanvasPainterFactory *m_factory = nullptr;
    QColor m_fillColor = Qt::black;
    QCanvasDebugCounterVisualizer m_debugVis;
    QCDebugCounters m_debugCounters;
    QRhiCommandBuffer *m_currentCb = nullptr;
    static QAtomicInt m_rendered;
};

QT_END_NAMESPACE

#endif // QCANVASPAINTERWIDGET_P_H
