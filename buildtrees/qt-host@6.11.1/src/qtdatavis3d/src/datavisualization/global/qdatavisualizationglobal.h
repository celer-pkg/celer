// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QDATAVISUALIZATIONGLOBAL_H
#define QDATAVISUALIZATIONGLOBAL_H

#include <QtCore/qglobal.h>
#include <QtDataVisualization/qtdatavisualizationexports.h>

QT_BEGIN_NAMESPACE

#define QT_DATAVISUALIZATION_VERSION_STR QT_VERSION_STR
/*
   QT_DATAVISUALIZATION_VERSION is (major << 16) + (minor << 8) + patch.
*/
#define QT_DATAVISUALIZATION_VERSION QT_VERSION
/*
   can be used like #if (QT_DATAVISUALIZATION_VERSION >= QT_DATAVISUALIZATION_VERSION_CHECK(1, 0, 0))
*/
#define QT_DATAVISUALIZATION_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))

#ifdef QTGRAPHS_HEADERS
#error Mixing QtDataVisualization and QtGraphs in the same TU is unsupported since they use\
 the same class names.
#endif

#define QTDATAVIS3D_HEADERS

QT_END_NAMESPACE

#endif
