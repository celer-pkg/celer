// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNFCTAGTYPE4NDEFFSM_P_H
#define QNFCTAGTYPE4NDEFFSM_P_H

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

#include "qndefaccessfsm_p.h"
#include "qapduutils_p.h"

QT_BEGIN_NAMESPACE

class QNfcTagType4NdefFsm : public QNdefAccessFsm
{
public:
    QByteArray getCommand(Action &nextAction) override;
    QNdefMessage getMessage(Action &nextAction) override;
    Action provideResponse(const QByteArray &response) override;

    Action detectNdefSupport() override;
    Action readMessages() override;
    Action writeMessages(const QList<QNdefMessage> &messages) override;

private:
    enum State {
        SelectApplicationForProbe,
        SelectCCFile,
        ReadCCFile,
        NdefSupportDetected,
        NdefNotSupported,

        SelectApplicationForRead,
        SelectNdefFileForRead,
        ReadNdefMessageLength,
        ReadNdefMessage,
        NdefMessageRead,

        SelectApplicationForWrite,
        SelectNdefFileForWrite,
        ClearNdefLength,
        WriteNdefFile,
        WriteNdefLength,
        NdefMessageWritten // Only for target state, it is never actually reached
    };

    State m_currentState = SelectApplicationForProbe;
    State m_targetState = SelectApplicationForProbe;

    // Initialized during the detection phase
    uint16_t m_maxReadSize;
    uint16_t m_maxUpdateSize;
    QByteArray m_ndefFileId;
    uint16_t m_maxNdefSize = 0xFFFF;
    bool m_writable;

    // Used during the read and write operations
    uint16_t m_fileSize;
    uint16_t m_fileOffset;
    QByteArray m_ndefData;

    Action handleSimpleResponse(const QResponseApdu &response, State okState, State failedState,
                                Action okAction = SendCommand);

    Action handleReadCCResponse(const QResponseApdu &response);
    Action handleReadFileLengthResponse(const QResponseApdu &response);
    Action handleReadFileResponse(const QResponseApdu &response);
    Action handleWriteNdefFileResponse(const QResponseApdu &response);
};

QT_END_NAMESPACE

#endif // QNFCTAGTYPE4NDEFFSM_P_H
