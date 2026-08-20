// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qpcscmanager_p.h"
#include "qpcscslot_p.h"
#include "qpcsccard_p.h"
#include <QtCore/QLoggingCategory>
#include <QtCore/QThread>
#include <QtCore/QTimer>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_NFC_PCSC)

static constexpr auto PollIntervalEnvVar = "QT_NFC_POLL_INTERVAL_MS";
static constexpr int DefaultPollIntervalMs = 100;

QPcscManager::QPcscManager(QObject *parent) : QObject(parent)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    int pollInterval = DefaultPollIntervalMs;
    QByteArray intervalEnv = qgetenv(PollIntervalEnvVar);
    if (!intervalEnv.isEmpty()) {
        if (int intervalFromEnv = intervalEnv.toInt(); intervalFromEnv > 0)
            pollInterval = intervalFromEnv;
        else
            qCWarning(QT_NFC_PCSC) << PollIntervalEnvVar << "set to an invalid value";
    }

    m_stateUpdateTimer = new QTimer(this);
    m_stateUpdateTimer->setInterval(pollInterval);
    connect(m_stateUpdateTimer, &QTimer::timeout, this, &QPcscManager::onStateUpdate);
}

QPcscManager::~QPcscManager()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;
    if (m_hasContext) {
        // Destroy all card handles before destroying the PCSC context.
        for (auto slot : std::as_const(m_slots))
            slot->invalidateInsertedCard();
        SCardReleaseContext(m_context);
    }

    // Stop the worker thread.
    thread()->quit();
}

void QPcscManager::processSlotUpdates()
{
    for (auto &state : m_slotStates) {
        auto slot = static_cast<QPcscSlot *>(state.pvUserData);
        Q_ASSERT(slot != nullptr);

        if ((state.dwEventState & SCARD_STATE_UNKNOWN) != 0)
            continue;

        if (state.dwEventState == state.dwCurrentState)
            continue;

        qCDebug(QT_NFC_PCSC) << Qt::hex << state.dwCurrentState << "=>" << state.dwEventState << ":"
                             << slot->name();

        state.dwCurrentState = state.dwEventState;
        slot->processStateChange(state.dwEventState, m_targetDetectionRunning);
    }
}

/*
    Remove slots that no longer need to be tracked.
*/
void QPcscManager::removeSlots()
{
    for (auto &state : m_slotStates) {
        auto slot = static_cast<QPcscSlot *>(state.pvUserData);
        Q_ASSERT(slot != nullptr);

        // Remove slots that no longer exist, or all slots without cards if
        // target detection is stopped.
        if ((state.dwEventState & SCARD_STATE_UNKNOWN) != 0
            || !(m_targetDetectionRunning || slot->hasCard())) {
            qCDebug(QT_NFC_PCSC) << "Removing slot:" << slot;
            state.dwEventState = SCARD_STATE_UNKNOWN;
            slot->invalidateInsertedCard();
            m_slots.remove(slot->name());
            slot->deleteLater();
            state.pvUserData = nullptr;
        }
    }

    // Remove state tracking entries for slots that no longer exist.
    m_slotStates.removeIf(
            [](const auto &state) { return (state.dwEventState & SCARD_STATE_UNKNOWN) != 0; });
}

/*
    Reads the system slot lists and marks slots that no longer exists, also
    creates new slot entries if target detection is currently running.
*/
void QPcscManager::updateSlotList()
{
    Q_ASSERT(m_hasContext);

#ifndef SCARD_AUTOALLOCATE
    // macOS does not support automatic allocation. Try using a fixed-size
    // buffer first, extending it if it is not sufficient.
#define LIST_READER_BUFFER_EXTRA 1024
    QPcscSlotName buf(nullptr);
    DWORD listSize = LIST_READER_BUFFER_EXTRA;
    buf.resize(listSize);
    QPcscSlotName::Ptr list = buf.ptr();

    auto ret = SCardListReaders(m_context, nullptr, list, &listSize);
#else
    QPcscSlotName::Ptr list;
    DWORD listSize = SCARD_AUTOALLOCATE;
    auto ret = SCardListReaders(m_context, nullptr, reinterpret_cast<QPcscSlotName::Ptr>(&list),
                                &listSize);
#endif

    if (ret == LONG(SCARD_E_NO_READERS_AVAILABLE)) {
        list = nullptr;
        ret = SCARD_S_SUCCESS;
    }
#ifndef SCARD_AUTOALLOCATE
    else if (ret == LONG(SCARD_E_INSUFFICIENT_BUFFER)) {
        // SCardListReaders() has set listSize to the required size. We add
        // extra space to reduce possibility of failure if the reader list has
        // changed since the last call.
        listSize += LIST_READER_BUFFER_EXTRA;
        buf.resize(listSize);
        list = buf.ptr();

        ret = SCardListReaders(m_context, nullptr, list, &listSize);
        if (ret == LONG(SCARD_E_NO_READERS_AVAILABLE)) {
            list = nullptr;
            ret = SCARD_S_SUCCESS;
        }
    }
#undef LIST_READER_BUFFER_EXTRA
#endif

    if (ret != SCARD_S_SUCCESS) {
        qCDebug(QT_NFC_PCSC) << "Failed to list readers:" << QPcsc::errorMessage(ret);
        return;
    }

#ifdef SCARD_AUTOALLOCATE
    auto freeList = qScopeGuard([this, list] {
        if (list) {
            Q_ASSERT(m_hasContext);
            SCardFreeMemory(m_context, list);
        }
    });
#endif

    QSet<QPcscSlotName> presentSlots;

    if (list != nullptr) {
        for (const auto *p = list; *p; p += QPcscSlotName::nameSize(p) + 1)
            presentSlots.insert(QPcscSlotName(p));
    }

    // Check current state list and mark slots that are not present anymore to
    // be removed later.
    for (auto &state : m_slotStates) {
        auto slot = static_cast<QPcscSlot *>(state.pvUserData);
        Q_ASSERT(slot != nullptr);

        if (presentSlots.contains(slot->name()))
            presentSlots.remove(slot->name());
        else
            state.dwEventState = SCARD_STATE_UNKNOWN;
    }

    if (!m_targetDetectionRunning)
        return;

    // Add new slots
    for (auto &&slotName : std::as_const(presentSlots)) {
        QPcscSlot *slot = new QPcscSlot(slotName, this);
        qCDebug(QT_NFC_PCSC) << "New slot:" << slot;

        m_slots[slotName] = slot;

        SCARD_READERSTATE state {};
        state.pvUserData = slot;
        state.szReader = slot->name().ptr();
        state.dwCurrentState = SCARD_STATE_UNAWARE;

        m_slotStates.append(state);
    }
}

bool QPcscManager::establishContext()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    Q_ASSERT(!m_hasContext);

    LONG ret = SCardEstablishContext(SCARD_SCOPE_USER, nullptr, nullptr, &m_context);
    if (ret != SCARD_S_SUCCESS) {
        qCWarning(QT_NFC_PCSC) << "Failed to establish context:" << QPcsc::errorMessage(ret);
        return false;
    }
    m_hasContext = true;

    return true;
}

void QPcscManager::onStateUpdate()
{
    if (!m_hasContext) {
        if (!m_targetDetectionRunning) {
            m_stateUpdateTimer->stop();
            return;
        }

        if (!establishContext())
            return;
    }

    updateSlotList();
    removeSlots();

    if (m_slotStates.isEmpty()) {
        if (!m_targetDetectionRunning) {
            // Free the context if it is no longer needed to card tracking.
            SCardReleaseContext(m_context);
            m_hasContext = false;

            m_stateUpdateTimer->stop();
        }
        return;
    }

    // Both Windows and PCSCLite support interruptable continuos state detection
    // where SCardCancel() can be used to abort it from another thread.
    // But that introduces a problem of reliable cancelling of the status change
    // call and no other. Reliable use of SCardCancel() would probably require
    // some form of synchronization and polling, and would make the code much
    // more complicated.
    // Alternatively, the state detection code could run in a yet another thread
    // that will not need to be cancelled too often.
    // For simplicity, the current code just checks for status changes every
    // second.
    LONG ret = SCardGetStatusChange(m_context, 0, m_slotStates.data(), m_slotStates.size());

    if (ret == SCARD_S_SUCCESS || ret == LONG(SCARD_E_UNKNOWN_READER)) {
        processSlotUpdates();
        removeSlots();
    } else if (ret == LONG(SCARD_E_CANCELLED) || ret == LONG(SCARD_E_UNKNOWN_READER)
               || ret == LONG(SCARD_E_TIMEOUT)) {
        /* ignore */
    } else {
        qCWarning(QT_NFC_PCSC) << "SCardGetStatusChange failed:" << QPcsc::errorMessage(ret);

        // Unknown failure. It is likely that the current context will not
        // recover from it, so destroy it and try to create a new context at the
        // next iteration.
        Q_ASSERT(m_hasContext);
        m_hasContext = false;
        for (auto slot : std::as_const(m_slots)) {
            slot->invalidateInsertedCard();
            slot->deleteLater();
        }
        SCardReleaseContext(m_context);
        m_slots.clear();
        m_slotStates.clear();
    }
}

void QPcscManager::onStartTargetDetectionRequest(QNearFieldTarget::AccessMethod accessMethod)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    m_requestedMethod = accessMethod;

    if (m_targetDetectionRunning)
        return;

    m_targetDetectionRunning = true;
    m_stateUpdateTimer->start();
}

void QPcscManager::onStopTargetDetectionRequest()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;
    m_targetDetectionRunning = false;
}

QPcscCard *QPcscManager::connectToCard(QPcscSlot *slot)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;
    Q_ASSERT(slot != nullptr);
    Q_ASSERT(m_hasContext);

    SCARDHANDLE cardHandle;
    DWORD activeProtocol;

    LONG ret = SCardConnect(m_context, slot->name().ptr(), SCARD_SHARE_SHARED,
                            SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &cardHandle, &activeProtocol);
    if (ret != SCARD_S_SUCCESS) {
        qCDebug(QT_NFC_PCSC) << "Failed to connect to card:" << QPcsc::errorMessage(ret);
        retryCardDetection(slot);
        return nullptr;
    }

    auto card = new QPcscCard(cardHandle, activeProtocol, this);
    auto uid = card->readUid();
    auto maxInputLength = card->readMaxInputLength();

    QNearFieldTarget::AccessMethods accessMethods = QNearFieldTarget::TagTypeSpecificAccess;
    if (card->supportsNdef())
        accessMethods |= QNearFieldTarget::NdefAccess;

    if (m_requestedMethod != QNearFieldTarget::UnknownAccess
        && (accessMethods & m_requestedMethod) == 0) {
        qCDebug(QT_NFC_PCSC) << "Dropping card without required access support";
        card->deleteLater();
        return nullptr;
    }

    if (!card->isValid()) {
        qCDebug(QT_NFC_PCSC) << "Card became invalid";
        card->deleteLater();

        retryCardDetection(slot);

        return nullptr;
    }

    Q_EMIT cardInserted(card, uid, accessMethods, maxInputLength);

    return card;
}

/*
    Setup states list so that the card detection for the given slot will
    be retried on the next iteration.

    This is useful to try to get cards working after reset.
*/
void QPcscManager::retryCardDetection(const QPcscSlot *slot)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    for (auto &state : m_slotStates) {
        if (state.pvUserData == slot) {
            state.dwCurrentState = SCARD_STATE_UNAWARE;
            break;
        }
    }
}

QT_END_NAMESPACE
