// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQUICKTIMELINEANIMATION_H
#define QQUICKTIMELINEANIMATION_H

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

#include "qtquicktimelineglobal_p.h"

#include <QtQuick/private/qquickanimation_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICKTIMELINE_EXPORT QQuickTimelineAnimation : public QQuickNumberAnimation
{
    Q_OBJECT

    Q_PROPERTY(bool pingPong READ pingPong WRITE setPingPong NOTIFY pingPongChanged)
    QML_NAMED_ELEMENT(TimelineAnimation)
    QML_ADDED_IN_VERSION(1, 0)

public:
    explicit QQuickTimelineAnimation(QObject *parent = nullptr);

    void setPingPong(bool b);
    bool pingPong() const;

Q_SIGNALS:
    void pingPongChanged();
    void finished();

private:
    void handleStarted();
    void handleStopped();

    bool m_pinpong = false;
    bool m_reversed = false;
    bool m_originalStart = true;
    int m_currentLoop = 0;
    int m_originalLoop = 0;
};

QT_END_NAMESPACE

#endif // QQUICKTIMELINEANIMATION_H
