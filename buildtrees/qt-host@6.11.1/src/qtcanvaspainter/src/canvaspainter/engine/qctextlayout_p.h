// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCTEXTLAYOUT_P_H
#define QCTEXTLAYOUT_P_H

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

#include "engine/qcpainterengineutils_p.h"
#include <QFontMetrics>

QT_BEGIN_NAMESPACE

class QCTextLayout
{
public:
    static QTextOption::WrapMode convertToQtWrapMode(QCanvasPainter::WrapMode mode);
    static Qt::Alignment convertToQtAlignment(QCanvasPainter::TextAlign alignment);
    static float calculateVerticalAlignment(QCanvasPainter::TextBaseline baseline, const QRectF &rect,
                                            const QFontMetrics &metrics, const QRectF &layoutRect);
    static Qt::LayoutDirection convertToQtDirection(QCanvasPainter::TextDirection direction);
};

QT_END_NAMESPACE

#endif
