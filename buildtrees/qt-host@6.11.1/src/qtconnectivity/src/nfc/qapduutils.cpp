// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qapduutils_p.h"
#include <QtCore/QtEndian>

QT_BEGIN_NAMESPACE

/*
    Utilities for handling smart card application protocol data units (APDU).

    The structure of APDUs is defined by ISO/IEC 7816-4 Organization, security
    and commands for interchange. The summary can be found on Wikipedia:

        https://en.wikipedia.org/wiki/Smart_card_application_protocol_data_unit
*/

/*
    Parses a response APDU from the raw data.

    If the data is too short to contain SW bytes, the returned responses SW
    is set to QResponseApdu::Empty.
*/
QResponseApdu::QResponseApdu(const QByteArray &response)
{
    const auto view = qToByteArrayViewIgnoringNull(response);
    if (view.size() < 2) {
        m_status = Empty;
        m_data = response;
    } else {
        const auto dataSize = view.size() - 2;
        m_status = qFromBigEndian(qFromUnaligned<uint16_t>(view.data() + dataSize));
        m_data = response.left(dataSize);
    }
}

/*
    Builds a command APDU from components according to ISO/IEC 7816.
*/
QByteArray QCommandApdu::build(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2,
                               QByteArrayView data, uint16_t ne)
{
    Q_ASSERT(data.size() <= 0xFFFF);

    QByteArray apdu;
    apdu.append(static_cast<char>(cla));
    apdu.append(static_cast<char>(ins));
    apdu.append(static_cast<char>(p1));
    apdu.append(static_cast<char>(p2));

    bool extendedLc = false;
    uint16_t nc = data.size();

    if (nc > 0) {
        if (nc < 256) {
            apdu.append(static_cast<char>(nc));
        } else {
            extendedLc = true;
            apdu.append('\0');
            apdu.append(static_cast<char>(nc >> 8));
            apdu.append(static_cast<char>(nc & 0xFF));
        }
        apdu.append(data);
    }

    if (ne) {
        if (ne < 256) {
            apdu.append(static_cast<char>(ne));
        } else if (ne == 256) {
            apdu.append(static_cast<char>('\0'));
        } else {
            if (!extendedLc)
                apdu.append('\0');
            apdu.append(static_cast<char>(ne >> 8));
            apdu.append(static_cast<char>(ne & 0xFF));
        }
    }

    return apdu;
}

QT_END_NAMESPACE
