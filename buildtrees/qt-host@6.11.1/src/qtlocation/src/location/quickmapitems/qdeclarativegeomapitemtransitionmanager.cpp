// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativegeomapitemtransitionmanager_p.h"
#include "qdeclarativegeomapitemview_p.h"

QT_BEGIN_NAMESPACE

QDeclarativeGeoMapItemTransitionManager::QDeclarativeGeoMapItemTransitionManager(QObject *mapItem)
    : QQuickTransitionManager(), m_mapItem(mapItem)
{
}

void QDeclarativeGeoMapItemTransitionManager::transitionEnter()
{
    if (m_transitionState == ExitTransition)
        cancel();

    if (!prepareEnterTransition())
        return;

    if (m_view && m_view->m_enter)
        transition(enterActions, m_view->m_enter, m_mapItem);
    else
        finished();
}

void QDeclarativeGeoMapItemTransitionManager::transitionExit()
{
    if (!prepareExitTransition())
        return;

    if (m_view && m_view->m_exit)
        transition(exitActions, m_view->m_exit, m_mapItem);
    else
        finished();
}

void QDeclarativeGeoMapItemTransitionManager::finished()
{
    if (m_transitionState == EnterTransition)
        finalizeEnterTransition();
    else if (m_transitionState == ExitTransition)
        finalizeExitTransition();
}


QT_END_NAMESPACE
