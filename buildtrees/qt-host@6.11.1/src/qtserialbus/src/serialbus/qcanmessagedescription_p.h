// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANMESSAGEDESCRIPTION_P_H
#define QCANMESSAGEDESCRIPTION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtserialbusexports.h"
#include "qcanmessagedescription.h"

QT_BEGIN_NAMESPACE

class Q_SERIALBUS_EXPORT QCanMessageDescriptionPrivate : public QSharedData
{
public:
    QString name;
    QString transmitter;
    QString comment;
    QtCanBus::UniqueId id{0};
    quint8 size = 0; // even CAN FD has max 64 bytes
    QHash<QString, QCanSignalDescription> messageSignals;

    inline bool isShared() const { return ref.loadRelaxed() != 1; }
    static QCanMessageDescriptionPrivate *get(const QCanMessageDescription &desc);
};

QT_END_NAMESPACE

#endif // QCANMESSAGEDESCRIPTION_P_H
