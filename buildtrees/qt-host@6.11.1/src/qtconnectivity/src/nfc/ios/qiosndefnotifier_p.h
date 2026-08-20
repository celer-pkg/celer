// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QIOSNDEFNOTIFIER_P_H
#define QIOSNDEFNOTIFIER_P_H

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

#include <QtCore/qglobal.h>
#include <QtCore/qobject.h>

#include <QtCore/qloggingcategory.h>

#include "qnearfieldmanager.h"
#include "qnearfieldtarget.h"

QT_BEGIN_NAMESPACE

class QNdefMessage;
class QString;

class QNfcNdefNotifier : public QObject
{
    Q_OBJECT

public:
    QNfcNdefNotifier() = default;

Q_SIGNALS:
    void tagDetected(void *tag);
    void invalidateWithError(bool restart);
    void tagLost(void *tag);

    void tagError(QNearFieldTarget::Error code, QNearFieldTarget::RequestId request);

    void ndefMessageWritten(QNearFieldTarget::RequestId request);
    void ndefMessageRead(const QNdefMessage &message, QNearFieldTarget::RequestId request);

private:
    Q_DISABLE_COPY_MOVE(QNfcNdefNotifier);
};

Q_DECLARE_LOGGING_CATEGORY(QT_IOS_NFC)

QT_END_NAMESPACE

#endif //QIOSNDEFNOTIFIER_P_H
