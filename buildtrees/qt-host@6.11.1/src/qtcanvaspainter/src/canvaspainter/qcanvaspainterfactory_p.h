// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPAINTERFACTORY_P_H
#define QCANVASPAINTERFACTORY_P_H

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

#include "qcanvaspainterfactory.h"
#include "qcanvasrhipaintdriver.h"
#include "engine/qcpainterrhirenderer_p.h"

QT_BEGIN_NAMESPACE

class QCanvasPainterFactoryPrivate
{
public:
    static const QCanvasPainterFactoryPrivate *get(const QCanvasPainterFactory *obj) { return obj->d; }
    static QCanvasPainterFactoryPrivate *get(QCanvasPainterFactory *obj) { return obj->d; }

    std::unique_ptr<QCanvasRhiPaintDriver> paintDriver;
    std::unique_ptr<QCanvasPainter> painter;
    QCPainterRhiRenderer renderer;
};

QT_END_NAMESPACE

#endif // QCANVASPAINTERFACTORY_P_H
