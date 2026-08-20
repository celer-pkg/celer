// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_Q3DSCENE_H
#define QTGRAPHS_Q3DSCENE_H

#include <QtCore/qobject.h>
#include <QtCore/qrect.h>
#include <QtGraphs/qgraphsglobal.h>
#include <QtQmlIntegration/qqmlintegration.h>

QT_BEGIN_NAMESPACE

class Q3DScenePrivate;

class Q_GRAPHS_EXPORT Q3DScene : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Q3DScene)
    Q_PROPERTY(QRect viewport READ viewport NOTIFY viewportChanged)
    Q_PROPERTY(QRect primarySubViewport READ primarySubViewport WRITE setPrimarySubViewport NOTIFY
                   primarySubViewportChanged)
    Q_PROPERTY(QRect secondarySubViewport READ secondarySubViewport WRITE setSecondarySubViewport
                   NOTIFY secondarySubViewportChanged)
    Q_PROPERTY(QPoint selectionQueryPosition READ selectionQueryPosition WRITE
                   setSelectionQueryPosition NOTIFY selectionQueryPositionChanged)
    Q_PROPERTY(bool secondarySubviewOnTop READ isSecondarySubviewOnTop WRITE
                   setSecondarySubviewOnTop NOTIFY secondarySubviewOnTopChanged)
    Q_PROPERTY(
        bool slicingActive READ isSlicingActive WRITE setSlicingActive NOTIFY slicingActiveChanged)
    Q_PROPERTY(qreal devicePixelRatio READ devicePixelRatio WRITE setDevicePixelRatio NOTIFY
                   devicePixelRatioChanged)
    Q_PROPERTY(QPoint graphPositionQuery READ graphPositionQuery WRITE setGraphPositionQuery NOTIFY
                   graphPositionQueryChanged)
    Q_PROPERTY(QPoint invalidSelectionPoint READ invalidSelectionPoint CONSTANT)

    QML_NAMED_ELEMENT(Scene3D)
    QML_UNCREATABLE("")

public:
    explicit Q3DScene(QObject *parent = nullptr);
    ~Q3DScene() override;

    QRect viewport() const;

    QRect primarySubViewport() const;
    void setPrimarySubViewport(QRect primarySubViewport);
    bool isPointInPrimarySubView(QPoint point);

    QRect secondarySubViewport() const;
    void setSecondarySubViewport(QRect secondarySubViewport);
    bool isPointInSecondarySubView(QPoint point);

    void setSelectionQueryPosition(QPoint point);
    QPoint selectionQueryPosition() const;

    void setGraphPositionQuery(QPoint point);
    QPoint graphPositionQuery() const;

    void setSlicingActive(bool isSlicing);
    bool isSlicingActive() const;

    void setSecondarySubviewOnTop(bool isSecondaryOnTop);
    bool isSecondarySubviewOnTop() const;

    qreal devicePixelRatio() const;
    void setDevicePixelRatio(qreal pixelRatio);

    QPoint invalidSelectionPoint() const;

Q_SIGNALS:
    void viewportChanged(QRect viewport);
    void primarySubViewportChanged(QRect subViewport);
    void secondarySubViewportChanged(QRect subViewport);
    void secondarySubviewOnTopChanged(bool isSecondaryOnTop);
    void slicingActiveChanged(bool isSlicingActive);
    void devicePixelRatioChanged(qreal pixelRatio);
    void selectionQueryPositionChanged(QPoint position);
    void graphPositionQueryChanged(QPoint position);
    void needRender();

private:
    Q_DISABLE_COPY(Q3DScene)

    friend class Q3DGraphsWidgetItem;
    friend class QQuickGraphsItem;
};

QT_END_NAMESPACE

#endif
