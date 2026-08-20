// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPCSCSLOT_P_H
#define QPCSCSLOT_P_H

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
#include <QtCore/QObject>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QPcscManager;
class QPcscCard;

class QPcscSlot : public QObject
{
    Q_OBJECT
public:
    QPcscSlot(const QPcscSlotName &name, QPcscManager *manager);
    ~QPcscSlot() override;

    const QPcscSlotName &name() const { return m_name; }
    void processStateChange(DWORD eventId, bool createCards);
    bool hasCard() const { return !m_insertedCard.isNull(); }
    void invalidateInsertedCard();

private:
    const QPcscSlotName m_name;
    QPointer<QPcscCard> m_insertedCard;
};

QT_END_NAMESPACE

#endif // QPCSCSLOT_P_H
