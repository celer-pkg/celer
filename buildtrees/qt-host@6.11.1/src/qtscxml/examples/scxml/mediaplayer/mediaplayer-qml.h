// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MEDIAPLAYER_QML
#define MEDIAPLAYER_QML

#include "mediaplayer.h"

#include <QtQml/qqml.h>
#include <QtCore/qobject.h>

struct MediaPlayerStateMachineRegistration
{
    Q_GADGET
    QML_FOREIGN(MediaPlayerStateMachine)
    QML_NAMED_ELEMENT(MediaPlayerStateMachine)
    QML_ADDED_IN_VERSION(1, 0)
};

#endif // MEDIAPLAYER_QML
