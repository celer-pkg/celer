// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QGRAPHSGLOBAL_P_H
#define QGRAPHSGLOBAL_P_H

#include <QtCore/qstringliteral.h>
#include <QtGraphs/qgraphsglobal.h>
#include <QtGraphs/qtgraphsexports.h>
#include <QtGui/qvector3d.h>

QT_BEGIN_NAMESPACE

// Constants used in several files
// Distance from camera to origin
static const float cameraDistance = 6.0f;
// Size of font to be used in label texture rendering. Doesn't affect the actual font size.
static const int textureFontSize = 50;
static const QVector3D zeroVector = QVector3D(0.0f, 0.0f, 0.0f);
static const QVector3D upVector = QVector3D(0.0f, 1.0f, 0.0f);
static const float itemAlpha = 0.0f;
static const qreal gradientTextureHeight = 1.;
// Default to 4096 just in case we don't get real max from rhi
static const qreal gradientTextureWidth = 4096.;
// Tag to be used to hide a log axis label when edgeLabelsVisible is set to false
// or when an item selection label should not be shown
static constexpr auto hiddenLabelTag = u"õ";

QT_END_NAMESPACE

#endif
