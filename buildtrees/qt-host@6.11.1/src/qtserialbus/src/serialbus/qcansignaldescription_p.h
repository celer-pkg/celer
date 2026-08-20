// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANSIGNALDESCRIPTION_P_H
#define QCANSIGNALDESCRIPTION_P_H

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
#include "qcansignaldescription.h"

#include <QtCore/QHash>
#include <QtCore/QSharedData>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

class Q_SERIALBUS_EXPORT QCanSignalDescriptionPrivate : public QSharedData
{
public:
    QString name;
    QString unit;
    QString receiver;
    QString comment;
    QtCanBus::DataSource source = QtCanBus::DataSource::Payload;
    QSysInfo::Endian endian = QSysInfo::Endian::BigEndian;
    QtCanBus::DataFormat format = QtCanBus::DataFormat::SignedInteger;
    quint16 startBit = 0;
    quint16 dataLength = 0;
    // for conversion, possibly unused
    double factor = qQNaN();
    double offset = qQNaN();
    double scaling = qQNaN();
    // expected range, possibly unused
    double minimum = qQNaN();
    double maximum = qQNaN();
    // multiplexing state
    QtCanBus::MultiplexState muxState = QtCanBus::MultiplexState::None;
    // Multiplexed values. The key of the hash represents the multiplex switch
    // name, and the value represents the valid range(s) of the mux switch
    // values.
    QCanSignalDescription::MultiplexSignalValues muxSignals;

    bool muxValueInRange(const QVariant &value,
                         const QCanSignalDescription::MultiplexValues &ranges) const;

    inline bool isShared() const { return ref.loadRelaxed() != 1; }
    static QCanSignalDescriptionPrivate *get(const QCanSignalDescription &desc);
};

QT_END_NAMESPACE

#endif // QCANSIGNALDESCRIPTION_P_H
