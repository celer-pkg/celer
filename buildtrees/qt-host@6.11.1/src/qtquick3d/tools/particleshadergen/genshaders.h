// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef GENSHADERS_H
#define GENSHADERS_H

#include <QtCore/qlist.h>

#include <QtQuick3DRuntimeRender/private/qssgrendershaderkeys_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercodegenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderlibrarymanager_p.h>

QT_BEGIN_NAMESPACE
class QDir;
class QQuick3DSceneManager;
QT_END_NAMESPACE

struct GenShaders
{
    explicit GenShaders();
    ~GenShaders();
    bool process(QVector<QString> &qsbcFiles, const QDir &outDir, bool verbose,
                 bool dryRun);

    QScopedPointer<QRhi> rhi;
    std::shared_ptr<QSSGRenderContextInterface> renderContext;

    QScopedPointer<QQuick3DSceneManager> sceneManager;
};

#endif
