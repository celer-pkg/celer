// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASOFFSCREENCANVAS_P_H
#define QCANVASOFFSCREENCANVAS_P_H

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

#include <QtCore/qshareddata.h>
#include <QtCore/qsize.h>
#include "qcanvasoffscreencanvas.h"
#include "engine/qcpainterrhirenderer_p.h"

QT_BEGIN_NAMESPACE

class QCanvasOffscreenCanvasPrivate : public QSharedData
{
public:
    static QCanvasOffscreenCanvasPrivate *get(QCanvasOffscreenCanvas *canvas) { return canvas->d.get(); }
    static const QCanvasOffscreenCanvasPrivate *get(const QCanvasOffscreenCanvas *canvas) { return canvas->d.get(); }

    QCRhiCanvas rhiCanvas;
    QColor fillColor = {0, 0, 0, 0};
};

QT_END_NAMESPACE

#endif // QCANVASOFFSCREENCANVAS_P_H
