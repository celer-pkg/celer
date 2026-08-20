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

#ifndef Q3DSCENE_P_H
#define Q3DSCENE_P_H

#include <QtCore/private/qobject_p.h>
#include <QtGraphs/q3dscene.h>
#include <private/qgraphsglobal_p.h>

QT_BEGIN_NAMESPACE

struct Q3DSceneChangeBitField
{
    bool viewportChanged : 1;
    bool primarySubViewportChanged : 1;
    bool secondarySubViewportChanged : 1;
    bool subViewportOrderChanged : 1;
    bool slicingActivatedChanged : 1;
    bool devicePixelRatioChanged : 1;
    bool selectionQueryPositionChanged : 1;
    bool graphPositionQueryPositionChanged : 1;
    bool windowSizeChanged : 1;

    Q3DSceneChangeBitField()
        : viewportChanged(true)
        , primarySubViewportChanged(true)
        , secondarySubViewportChanged(true)
        , subViewportOrderChanged(true)
        , slicingActivatedChanged(true)
        , devicePixelRatioChanged(true)
        , selectionQueryPositionChanged(false)
        , graphPositionQueryPositionChanged(false)
        , windowSizeChanged(true)
    {}
};

class Q_GRAPHS_EXPORT Q3DScenePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(Q3DScene)

public:
    Q3DScenePrivate();
    ~Q3DScenePrivate();

    void sync(Q3DScenePrivate &other);

    void setViewport(const QRect viewport);
    void setViewportSize(int width, int height);
    void setWindowSize(QSize size);
    QSize windowSize() const;
    void updateDefaultViewports();

    void markDirty();

    bool isInArea(const QRect area, int x, int y) const;

public:
    Q3DSceneChangeBitField m_changeTracker;

    QRect m_viewport;
    QRect m_primarySubViewport;
    QRect m_secondarySubViewport;
    bool m_isSecondarySubviewOnTop;
    qreal m_devicePixelRatio;
    bool m_isUnderSideCameraEnabled;
    bool m_isSlicingActive;
    QPoint m_selectionQueryPosition;
    QPoint m_graphPositionQueryPosition;
    QSize m_windowSize;
    bool m_sceneDirty;
    QRect m_defaultSmallViewport;
    QRect m_defaultLargeViewport;
};

QT_END_NAMESPACE

#endif
