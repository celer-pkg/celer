// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANUNIQUEIDDESCRIPTION_P_H
#define QCANUNIQUEIDDESCRIPTION_P_H

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
#include "qcanuniqueiddescription.h"

QT_BEGIN_NAMESPACE

class Q_SERIALBUS_EXPORT QCanUniqueIdDescriptionPrivate : public QSharedData
{
public:
    QtCanBus::DataSource source = QtCanBus::DataSource::FrameId;
    QSysInfo::Endian endian = QSysInfo::Endian::LittleEndian;
    quint16 startBit = 0;
    quint8 bitLength = 0;

    inline bool isShared() const { return ref.loadRelaxed() != 1; }
    static QCanUniqueIdDescriptionPrivate *get(const QCanUniqueIdDescription &desc);
};

QT_END_NAMESPACE

#endif // QCANUNIQUEIDDESCRIPTION_P_H
