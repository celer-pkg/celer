// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QGRAPHSGLOBAL_H
#define QTGRAPHS_QGRAPHSGLOBAL_H

#include <QtCore/qglobal.h>
#include <QtGraphs/qtgraphsexports.h>

#ifdef QTDATAVIS3D_HEADERS
#error Mixing QtDataVisualization and QtGraphs in the same TU is unsupported since they use\
 the same class names.
#endif

#ifdef QTCHARTS_HEADERS
#error Mixing QtCharts and QtGraphs in the same TU is unsupported since they use\
 the same class names.
#endif

#define QTGRAPHS_HEADERS

#endif
