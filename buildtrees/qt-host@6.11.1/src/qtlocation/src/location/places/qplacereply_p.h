// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACEREPLY_P_H
#define QPLACEREPLY_P_H

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

#include "qplacereply.h"

QT_BEGIN_NAMESPACE

class QPlaceReplyPrivate
{
public:
    virtual ~QPlaceReplyPrivate(){}
    bool isFinished = false;
    QPlaceReply::Error error = QPlaceReply::NoError;
    QString errorString;
};

QT_END_NAMESPACE

#endif
