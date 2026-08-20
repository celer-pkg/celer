// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef GENSHADERS_H
#define GENSHADERS_H

#include "parser.h"

#include <QtCore/qlist.h>

#include <QtQuick3DRuntimeRender/private/qssgrendershaderkeys_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercodegenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderlibrarymanager_p.h>

QT_BEGIN_NAMESPACE
class QDir;
class QQuick3DSceneManager;
class QQuick3DWindowAttachment;
QT_END_NAMESPACE

struct GenShaders
{
    explicit GenShaders();
    ~GenShaders();
    bool process(const MaterialParser::SceneData &sceneData, QVector<QString> &qsbcFiles, const QDir &outDir,
                 bool generateMultipleLights, bool dryRun);

    QRhi *rhi = nullptr;
    std::shared_ptr<QSSGRenderContextInterface> renderContext;

    QQuick3DSceneManager *sceneManager = nullptr;
    QQuick3DWindowAttachment *wa = nullptr;
};

#endif
