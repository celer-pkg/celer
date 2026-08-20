// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DINFINITEGRID_H
#define QQUICK3DINFINITEGRID_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3DHelpers/qtquick3dhelpersexports.h>
#include <QQuick3DObject>

QT_BEGIN_NAMESPACE

class QQuick3DSceneEnvironment;

class Q_QUICK3DHELPERS_EXPORT QQuick3DInfiniteGrid : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    QML_NAMED_ELEMENT(InfiniteGrid)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(float gridInterval READ gridInterval WRITE setGridInterval NOTIFY gridIntervalChanged)
    Q_PROPERTY(bool gridAxes READ gridAxes WRITE setGridAxes NOTIFY gridAxesChanged)

public:
    QQuick3DInfiniteGrid();
    ~QQuick3DInfiniteGrid() override;
    bool visible() const;
    void setVisible(bool newVisible);
    float gridInterval() const;
    void setGridInterval(float newGridInterval);
    void componentComplete() override;
    void classBegin() override;

    bool gridAxes() const;
    void setGridAxes(bool newGridAxes);

signals:
    void visibleChanged();
    void gridIntervalChanged();

    void gridAxesChanged();

private:
    void updateGridFlags();
    bool m_visible = true;
    float m_gridInterval = 1.0f;
    QQuick3DSceneEnvironment *m_sceneEnv = nullptr;
    bool m_componentComplete = false;
    bool m_gridAxes = true;
};

QT_END_NAMESPACE

#endif // QQUICK3DINFINITEGRID_H
