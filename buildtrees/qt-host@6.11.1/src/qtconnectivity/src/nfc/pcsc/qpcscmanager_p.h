// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPCSCMANAGER_P_H
#define QPCSCMANAGER_P_H

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

#include "qpcsc_p.h"
#include "qnearfieldtarget.h"

QT_BEGIN_NAMESPACE

class QPcscSlot;
class QPcscCard;
class QTimer;

class QPcscManager : public QObject
{
    Q_OBJECT
public:
    explicit QPcscManager(QObject *parent = nullptr);
    ~QPcscManager() override;

    QPcscCard *connectToCard(QPcscSlot *slot);

private:
    QTimer *m_stateUpdateTimer;
    bool m_targetDetectionRunning = false;
    bool m_hasContext = false;
    SCARDCONTEXT m_context;
    QMap<QPcscSlotName, QPcscSlot *> m_slots;
    QList<SCARD_READERSTATE> m_slotStates;
    QNearFieldTarget::AccessMethod m_requestedMethod;

    [[nodiscard]] bool establishContext();
    void processSlotUpdates();
    void updateSlotList();
    void removeSlots();
    void retryCardDetection(const QPcscSlot *slot);

public Q_SLOTS:
    void onStartTargetDetectionRequest(QNearFieldTarget::AccessMethod accessMethod);
    void onStopTargetDetectionRequest();

private Q_SLOTS:
    void onStateUpdate();

Q_SIGNALS:
    void cardInserted(QPcscCard *card, const QByteArray &uid,
                      QNearFieldTarget::AccessMethods accessMethods, int maxInputLength);
};

QT_END_NAMESPACE

#endif // QPCSCMANAGER_P_H
