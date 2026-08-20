// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEGEOMAPITEMTRANSITIONMANAGER_H
#define QDECLARATIVEGEOMAPITEMTRANSITIONMANAGER_H

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
#include <QtQuick/QQuickItem>
#include <QtQuick/private/qquicktransitionmanager_p_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeGeoMapItemView;

class QDeclarativeGeoMapItemTransitionManager : public QQuickTransitionManager
{
public:
    enum TransitionState {
        NoTransition, EnterTransition, ExitTransition
    };

    QDeclarativeGeoMapItemTransitionManager(QObject *mapItem);

    void transitionEnter();
    void transitionExit();

    bool prepareEnterTransition()
    {
        if (m_transitionState == QDeclarativeGeoMapItemTransitionManager::EnterTransition
                && isRunning())
            return false;

        if (m_transitionState != QDeclarativeGeoMapItemTransitionManager::EnterTransition) {
    //        setVisible(true);
            m_transitionState = QDeclarativeGeoMapItemTransitionManager::EnterTransition;
        }
        return true;
    }
    bool prepareExitTransition()
    {
        if (m_transitionState == QDeclarativeGeoMapItemTransitionManager::ExitTransition
                && isRunning())
            return false;

        if (m_transitionState != QDeclarativeGeoMapItemTransitionManager::ExitTransition) {
            m_transitionState = QDeclarativeGeoMapItemTransitionManager::ExitTransition;
        }
        return true;
    }
    void finalizeEnterTransition()
    {
        m_transitionState = QDeclarativeGeoMapItemTransitionManager::NoTransition;
        // use invokeMethod since this is used on both QDeclarativeGeoMapItemBase and QDeclarativeGeoMapItemGroup
        QMetaObject::invokeMethod(m_mapItem, QByteArrayLiteral("addTransitionFinished").constData(), Qt::DirectConnection);
    }
    void finalizeExitTransition()
    {
//        setVisible(false);
        m_transitionState = QDeclarativeGeoMapItemTransitionManager::NoTransition;
        QMetaObject::invokeMethod(m_mapItem, QByteArrayLiteral("removeTransitionFinished").constData(), Qt::DirectConnection);
    }

protected:
    void finished() override;

public:
    QObject *m_mapItem;
    QDeclarativeGeoMapItemView *m_view = nullptr;
    QList<QQuickStateAction> enterActions;
    QList<QQuickStateAction> exitActions;
    TransitionState m_transitionState = NoTransition;
};

QT_END_NAMESPACE

#endif
