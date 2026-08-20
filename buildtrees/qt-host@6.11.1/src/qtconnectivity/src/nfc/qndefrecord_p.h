// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNDEFRECORD_P_H
#define QNDEFRECORD_P_H

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

#include "qtnfcglobal.h"

#include <QtCore/QSharedData>
#include <QtCore/QByteArray>

QT_BEGIN_NAMESPACE

class QNdefRecordPrivate : public QSharedData
{
public:
    QNdefRecordPrivate() : QSharedData()
    {
        typeNameFormat = 0; //TypeNameFormat::Empty
    }

    unsigned int typeNameFormat : 3;

    QByteArray type;
    QByteArray id;
    QByteArray payload;
};

QT_END_NAMESPACE

#endif // QNDEFRECORD_P_H
