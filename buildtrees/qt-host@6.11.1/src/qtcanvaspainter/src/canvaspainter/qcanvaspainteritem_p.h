// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPAINTERITEM_P_H
#define QCANVASPAINTERITEM_P_H

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

#include "qcanvaspainteritem.h"
#include <QTimer>
#include <private/qquickrhiitem_p.h>

QT_BEGIN_NAMESPACE

struct QCDebugCounters;

class QCanvasPainterItemPrivate : public QQuickRhiItemPrivate
{
    Q_DECLARE_PUBLIC(QCanvasPainterItem)
public:
    void init();
    void updateDebugData(const QCDebugCounters &debugCounters);
    void updateDebug();

    QCanvasPainterItemRenderer *m_renderer = nullptr;
    QColor m_fillColor = Qt::black;
    QVariantMap m_debugData;
    QTimer m_debugUpdateTimer;
    bool m_debugDataChanged = false;
};

QT_END_NAMESPACE

#endif // QCANVASPAINTERITEM_P_H
