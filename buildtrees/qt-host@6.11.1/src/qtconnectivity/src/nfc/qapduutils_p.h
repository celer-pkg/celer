// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QAPDUUTILS_P_H
#define QAPDUUTILS_P_H

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

#include <QtCore/QByteArray>

QT_BEGIN_NAMESPACE

class QResponseApdu
{
public:
    static constexpr uint16_t Empty = 0x0000;
    static constexpr uint16_t Success = 0x9000;

    explicit QResponseApdu(const QByteArray &response = {});

    const QByteArray &data() const { return m_data; }
    uint16_t status() const { return m_status; }
    bool isOk() const { return m_status == Success; }

private:
    QByteArray m_data;
    uint16_t m_status;
};

namespace QCommandApdu {

// INS byte values for command APDUs
constexpr uint8_t Select = 0xA4;
constexpr uint8_t ReadBinary = 0xB0;
constexpr uint8_t GetData = 0xCA;
constexpr uint8_t UpdateBinary = 0xD6;

QByteArray build(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, QByteArrayView data,
                 uint16_t ne = 0);
};

QT_END_NAMESPACE

#endif // QAPDUUTILS_P_H
