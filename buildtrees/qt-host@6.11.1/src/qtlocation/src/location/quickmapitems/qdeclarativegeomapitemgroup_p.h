// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEGEOMAPITEMGROUP_P_H
#define QDECLARATIVEGEOMAPITEMGROUP_P_H

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

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/private/qdeclarativegeomapitemtransitionmanager_p.h>
#include <QtQuick/QQuickItem>

QT_BEGIN_NAMESPACE

class QDeclarativeGeoMap;
class Q_LOCATION_EXPORT QDeclarativeGeoMapItemGroup : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MapItemGroup)
    QML_ADDED_IN_VERSION(5, 0)

public:
    explicit QDeclarativeGeoMapItemGroup(QQuickItem *parent = nullptr);
    virtual ~QDeclarativeGeoMapItemGroup();

    void setParentGroup(QDeclarativeGeoMapItemGroup &parentGroup);
    void setQuickMap(QDeclarativeGeoMap *quickMap);
    QDeclarativeGeoMap *quickMap() const;

    qreal mapItemOpacity() const;

Q_SIGNALS:
    void mapItemOpacityChanged();
    void addTransitionFinished();
    void removeTransitionFinished();

protected:
    // QQmlParserStatus interface
    void classBegin() override;
    void componentComplete() override;

protected slots:
    void onMapSizeChanged();

private:
    QDeclarativeGeoMap *m_quickMap = nullptr;
    QDeclarativeGeoMapItemGroup *m_parentGroup = nullptr;
    std::unique_ptr<QDeclarativeGeoMapItemTransitionManager> m_transitionManager;

    friend class QDeclarativeGeoMapItemView;
    friend class QDeclarativeGeoMapItemTransitionManager;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeGeoMapItemGroup)

#endif // QDECLARATIVEGEOMAPITEMGROUP_P_H
