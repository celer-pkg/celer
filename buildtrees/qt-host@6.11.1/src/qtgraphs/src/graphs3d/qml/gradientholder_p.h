// Copyright (C) 2024 The Qt Company Ltd.
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

#ifndef GRADIENTHOLDER_P_H
#define GRADIENTHOLDER_P_H

#include <QtGraphs/qgraphsglobal.h>

QT_BEGIN_NAMESPACE

class QQuickGradient;
struct GradientHolder{
    QQuickGradient *m_baseGradient = nullptr;
    QQuickGradient *m_singleHighlightGradient = nullptr;
    QQuickGradient *m_multiHighlightGradient = nullptr;
};

QT_END_NAMESPACE

#endif // GRADIENTHOLDER_P_H
