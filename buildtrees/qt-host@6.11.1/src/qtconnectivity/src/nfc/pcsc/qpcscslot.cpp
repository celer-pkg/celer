// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qpcscslot_p.h"
#include "qpcscmanager_p.h"
#include "qpcsccard_p.h"
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_NFC_PCSC)

QPcscSlot::QPcscSlot(const QPcscSlotName &name, QPcscManager *manager)
    : QObject(manager), m_name(name)
{
}

QPcscSlot::~QPcscSlot()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO << this;
    if (m_insertedCard)
        m_insertedCard->invalidate();
}

void QPcscSlot::processStateChange(DWORD eventId, bool createCards)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    // Check if the currently inserted card is still valid
    if (!m_insertedCard.isNull()) {
        if (m_insertedCard->checkCardPresent())
            return;
        qCDebug(QT_NFC_PCSC) << "Removing card from slot" << m_name;
        m_insertedCard->invalidate();
        m_insertedCard.clear();
    }

    auto manager = qobject_cast<QPcscManager *>(parent());

    if (manager != nullptr && createCards
        && (eventId
            & (SCARD_STATE_PRESENT | SCARD_STATE_MUTE | SCARD_STATE_UNPOWERED
               | SCARD_STATE_EXCLUSIVE))
                == SCARD_STATE_PRESENT) {
        qCDebug(QT_NFC_PCSC) << "New card in slot" << m_name;

        m_insertedCard = manager->connectToCard(this);
    }
}

void QPcscSlot::invalidateInsertedCard()
{
    if (m_insertedCard)
        m_insertedCard->invalidate();
}

QT_END_NAMESPACE
