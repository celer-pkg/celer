// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:uses-kernel-cryptography
#include "lecmaccalculator_p.h"

#include "bluez/bluez_data_p.h"

#include <QtCore/qbytearray.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/private/qcore_unix_p.h>

#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef CONFIG_LINUX_CRYPTO_API
#include <linux/if_alg.h>
#endif

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

LeCmacCalculator::LeCmacCalculator()
{
#ifdef CONFIG_LINUX_CRYPTO_API
    m_baseSocket = socket(AF_ALG, SOCK_SEQPACKET, 0);
    if (m_baseSocket == -1) {
        qCWarning(QT_BT_BLUEZ) << "failed to create first level crypto socket:"
                               << strerror(errno);
        return;
    }
    sockaddr_alg sa;
    using namespace std;
    memset(&sa, 0, sizeof sa);
    sa.salg_family = AF_ALG;
    strcpy(reinterpret_cast<char *>(sa.salg_type), "hash");
    strcpy(reinterpret_cast<char *>(sa.salg_name), "cmac(aes)");
    if (::bind(m_baseSocket, reinterpret_cast<sockaddr *>(&sa), sizeof sa) == -1) {
        qCWarning(QT_BT_BLUEZ) << "bind() failed for crypto socket:" << strerror(errno);
        return;
    }
#else // CONFIG_LINUX_CRYPTO_API
    qCWarning(QT_BT_BLUEZ) << "Linux crypto API not present, CMAC verification will fail.";
#endif
}

LeCmacCalculator::~LeCmacCalculator()
{
    if (m_baseSocket != -1)
        close(m_baseSocket);
}

QByteArray LeCmacCalculator::createFullMessage(const QByteArray &message, quint32 signCounter)
{
    // Spec v4.2, Vol 3, Part H, 2.4.5
    QByteArray fullMessage = message;
    fullMessage.resize(fullMessage.size() + sizeof signCounter);
    putBtData(signCounter, fullMessage.data() + message.size());
    return fullMessage;
}

quint64 LeCmacCalculator::calculateMac(const QByteArray &message, QUuid::Id128Bytes csrk) const
{
#ifdef CONFIG_LINUX_CRYPTO_API
    if (m_baseSocket == -1)
        return false;
    QUuid::Id128Bytes csrkMsb;
    std::reverse_copy(std::begin(csrk.data), std::end(csrk.data), std::begin(csrkMsb.data));
    if (setsockopt(m_baseSocket, 279 /* SOL_ALG */, ALG_SET_KEY, csrkMsb.data, sizeof csrkMsb) == -1) {
        qCWarning(QT_BT_BLUEZ) << "setsockopt() failed for crypto socket:" << strerror(errno);
        return 0;
    }

    class SocketWrapper
    {
    public:
        SocketWrapper(int socket) : m_socket(socket) {}
        ~SocketWrapper() {
            if (m_socket != -1)
                close(m_socket);
        }

        int value() const { return m_socket; }
    private:
        int m_socket;
    };
    SocketWrapper cryptoSocket(accept(m_baseSocket, nullptr, nullptr));
    if (cryptoSocket.value() == -1) {
        qCWarning(QT_BT_BLUEZ) << "accept() failed for crypto socket:" << strerror(errno);
        return 0;
    }

    QByteArray messageSwapped(message.size(), Qt::Uninitialized);
    std::reverse_copy(message.begin(), message.end(), messageSwapped.begin());
    qint64 totalBytesWritten = 0;
    do {
        const qint64 bytesWritten = qt_safe_write(cryptoSocket.value(),
                                                  messageSwapped.constData() + totalBytesWritten,
                                                  messageSwapped.size() - totalBytesWritten);
        if (bytesWritten == -1) {
            qCWarning(QT_BT_BLUEZ) << "writing to crypto socket failed:" << strerror(errno);
            return 0;
        }
        totalBytesWritten += bytesWritten;
    } while (totalBytesWritten < messageSwapped.size());
    quint64 mac;
    quint8 * const macPtr = reinterpret_cast<quint8 *>(&mac);
    qint64 totalBytesRead = 0;
    do {
        const qint64 bytesRead = qt_safe_read(cryptoSocket.value(), macPtr + totalBytesRead,
                                              sizeof mac - totalBytesRead);
        if (bytesRead == -1) {
            qCWarning(QT_BT_BLUEZ) << "reading from crypto socket failed:" << strerror(errno);
            return 0;
        }
        totalBytesRead += bytesRead;
    } while (totalBytesRead < qint64(sizeof mac));
    return qFromBigEndian(mac);
#else // CONFIG_LINUX_CRYPTO_API
    Q_UNUSED(message);
    Q_UNUSED(csrk);
    qCWarning(QT_BT_BLUEZ) << "CMAC calculation failed due to missing Linux crypto API.";
    return 0;
#endif
}

bool LeCmacCalculator::verify(const QByteArray &message, QUuid::Id128Bytes csrk,
                           quint64 expectedMac) const
{
#ifdef CONFIG_LINUX_CRYPTO_API
    const quint64 actualMac = calculateMac(message, csrk);
    if (actualMac != expectedMac) {
        qCWarning(QT_BT_BLUEZ) << "signature verification failed: mac mismatch";
        return false;
    }
    return true;
#else // CONFIG_LINUX_CRYPTO_API
    Q_UNUSED(message);
    Q_UNUSED(csrk);
    Q_UNUSED(expectedMac);
    qCWarning(QT_BT_BLUEZ) << "CMAC verification failed due to missing Linux crypto API.";
    return false;
#endif
}

QT_END_NAMESPACE
