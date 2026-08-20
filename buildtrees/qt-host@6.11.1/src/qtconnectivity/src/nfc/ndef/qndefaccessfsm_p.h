// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNDEFACCESSFSM_P_H
#define QNDEFACCESSFSM_P_H

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

#include "qndefmessage.h"

QT_BEGIN_NAMESPACE

/*
    Base class for FSMs that can be used to exchange NDEF messages with cards.

    The user may call one of these methods to start a task:

    - detectNdefSupport()
    - readMessages()
    - writeMessages()

    The user is then expected to perform actions indicated by Action type.
*/
class QNdefAccessFsm
{
    Q_DISABLE_COPY_MOVE(QNdefAccessFsm)
public:
    QNdefAccessFsm() = default;
    virtual ~QNdefAccessFsm() = default;

    enum Action {
        // The requested task has successfully completed. New tasks can be started.
        Done,
        // The requested task has failed. New tasks can be started.
        Failed,
        // An NDEF message was successfully read. The user must call getMessage().
        GetMessage,
        // The user's call was unexpected. The FSM may be in an invalid state.
        Unexpected,
        // The user must call getCommand() and then send the returned command to the card.
        SendCommand,
        // The user must call provideResponse() with the result of the last sent command.
        ProvideResponse
    };

    /*
        Returns a command to send to the card.

        This method must be called if the FMS has requested SendCommand action.

        Next action will be ProvideResponse or Unexpected.
    */
    virtual QByteArray getCommand(Action &nextAction) = 0;

    /*
        This method must be called by the user to provide response for
        a completed command.

        An empty QByteArray can be provided to indicate that the command
        has failed.
    */
    virtual Action provideResponse(const QByteArray &response) = 0;

    /*
        Returns an NDEF message that was read from the card.

        This method must be called if the FSM has requested GetMessage action.
    */
    virtual QNdefMessage getMessage(Action &nextAction) = 0;

    /*
        Start NDEF support detection.
    */
    virtual Action detectNdefSupport() = 0;

    /*
        Start reading NDEF messages.

        This call also performs NDEF support detection if it was not performed
        earlier.
    */
    virtual Action readMessages() = 0;

    /*
        Start writing the given messages to the card.

        This call also performs NDEF detection if is was not performed earlier.
    */
    virtual Action writeMessages(const QList<QNdefMessage> &messages) = 0;
};

QT_END_NAMESPACE

#endif // QNDEFACCESSFSM_P_H
