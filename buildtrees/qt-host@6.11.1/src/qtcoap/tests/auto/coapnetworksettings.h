// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>
#include <QtCore/qstring.h>
#include <QtNetwork/qhostinfo.h>
#include <QtCoap/qcoapclient.h>
#include <QtCoap/qcoapsecurityconfiguration.h>

/*!
    \internal

    This namespace provides URL and settings used in QtCoap tests.

    Tests require a Californium plugtest server, accessible with
    "coap-plugtest-server" host name. You create such server with Docker and
    the following command line:
    \code
        docker run -d --rm -p 5683:5683/udp aleravat/coap-test-server:latest
    \endcode

    For more details, see
    \l{https://github.com/Pixep/coap-testserver-docker}{https://github.com/Pixep/coap-testserver-docker}.
*/
namespace QtCoapNetworkSettings
{

#if defined(COAP_TEST_SERVER_IP) || defined(QT_TEST_SERVER)
#define CHECK_FOR_COAP_SERVER
#else
#define CHECK_FOR_COAP_SERVER \
    QSKIP("CoAP server is not setup, skipping the test..."); \
    return;
#endif

#if defined(QT_TEST_SERVER) && !defined(COAP_TEST_SERVER_IP)
static QString tryToResolveHostName(const QString &hostName)
{
    const auto hostInfo = QHostInfo::fromName(hostName);
    if (!hostInfo.addresses().empty())
        return hostInfo.addresses().first().toString();

    qWarning() << "Could not resolve the hostname"<< hostName;
    return hostName;
}
#endif

static QString getHostAddress(const QString &serverName)
{
#if defined(COAP_TEST_SERVER_IP)
    Q_UNUSED(serverName);
    return QStringLiteral(COAP_TEST_SERVER_IP);
#elif defined(QT_TEST_SERVER_NAME)
    QString hostname = serverName % "." % QString(QT_TEST_SERVER_DOMAIN);
    return tryToResolveHostName(hostname);
#elif defined(QT_TEST_SERVER)
    Q_UNUSED(serverName);
    QString hostname = "qt-test-server." % QString(QT_TEST_SERVER_DOMAIN);
    return tryToResolveHostName(hostname);
#else
    Q_UNUSED(serverName);
    qWarning("This test will fail, "
             "please set the COAP_TEST_SERVER_IP variable to specify the CoAP server.");
    return "";
#endif
}

QString testServerHost()
{
    static QString testServerHostAddress = getHostAddress("californium");
    return testServerHostAddress;
}

QString timeServerUrl()
{
    static QString timeServerHostAddress = getHostAddress("freecoap");
    return QStringLiteral("coaps://") + timeServerHostAddress + QStringLiteral(":5685/time");
}

QString testServerUrl()
{
    return QStringLiteral("coap://") + testServerHost() + QStringLiteral(":")
            + QString::number(QtCoap::DefaultPort);
}

QString testServerResource()
{
    return testServerHost() + QStringLiteral("/test");
}

QCoapSecurityConfiguration createConfiguration(QtCoap::SecurityMode securityMode)
{
    QCoapSecurityConfiguration configuration;

    if (securityMode == QtCoap::SecurityMode::PreSharedKey) {
        configuration.setPreSharedKeyIdentity("Client_identity");
        configuration.setPreSharedKey("secretPSK");
    } else if (securityMode == QtCoap::SecurityMode::Certificate) {
        const QString directory = QFINDTESTDATA("testdata");
        if (directory.isEmpty()) {
            qWarning() << "Found no testdata/, cannot load certificates.";
            return configuration;
        }

        const auto localCertPath = directory + QDir::separator() +"local_cert.pem";
        const auto localCerts = QSslCertificate::fromPath(
                localCertPath, QSsl::Pem, QSslCertificate::PatternSyntax::FixedString);
        if (localCerts.isEmpty()) {
            qWarning() << "Failed to load local certificates, the"
                       << localCertPath
                       << "file was not found or it is not valid.";
        } else {
            configuration.setLocalCertificateChain(localCerts.toVector());
        }

        const auto caCertPath = directory + QDir::separator() + "ca_cert.pem";
        const auto caCerts = QSslCertificate::fromPath(caCertPath, QSsl::Pem,
                                                       QSslCertificate::PatternSyntax::FixedString);
        if (caCerts.isEmpty()) {
            qWarning() << "Failed to load CA certificates, the"
                       << caCertPath
                       << "file was not found or it is not valid.";
        } else {
            configuration.setCaCertificates(caCerts.toVector());
        }

        const auto privateKeyPath = directory + QDir::separator() + "privkey.pem";
        QFile privateKey(privateKeyPath);
        if (privateKey.open(QIODevice::ReadOnly)) {
            QCoapPrivateKey key(privateKey.readAll(), QSsl::Ec);
            if (key.isNull()) {
                qWarning() << "Failed to set a private key, the key" << privateKeyPath
                           << "is not valid.";
            } else {
                configuration.setPrivateKey(key);
            }
        } else {
            qWarning() << "Failed to read the private key" << privateKeyPath;
        }
    }

    return configuration;
}

bool waitForHost(const QUrl &url, QtCoap::SecurityMode security = QtCoap::SecurityMode::NoSecurity,
                 quint8 retries = 10)
{
    while (retries-- > 0) {
        QCoapClient client(security);
        if (security != QtCoap::SecurityMode::NoSecurity)
            client.setSecurityConfiguration(createConfiguration(security));

        QSignalSpy spyClientFinished(&client, SIGNAL(finished(QCoapReply*)));
        client.get(url);

        spyClientFinished.wait(1000);

        if (spyClientFinished.size() == 1)
            return true;
    }
    return false;
}

}
