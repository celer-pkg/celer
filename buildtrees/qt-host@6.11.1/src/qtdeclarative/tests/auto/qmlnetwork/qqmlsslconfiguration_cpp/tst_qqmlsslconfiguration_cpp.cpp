// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSsl>
#include <private/qqmlsslconfiguration_p.h>

class tst_QQmlSslConfiguration : public QObject
{
    Q_OBJECT

    enum class ConfType : quint8
    {
        DefaultSsl,
        DefaultDtls,
    };
private Q_SLOTS:
    void sslOptionsInSync_data();
    void sslOptionsInSync();

    void sslOptionFlags_data();
    void sslOptionFlags();

public:
    // Keep in sync with QSsl::SslOptions!
    static constexpr
    std::array<QSsl::SslOption, 8> AllOptions { QSsl::SslOptionDisableEmptyFragments,
                                                QSsl::SslOptionDisableSessionTickets,
                                                QSsl::SslOptionDisableCompression,
                                                QSsl::SslOptionDisableServerNameIndication,
                                                QSsl::SslOptionDisableLegacyRenegotiation,
                                                QSsl::SslOptionDisableSessionSharing,
                                                QSsl::SslOptionDisableSessionPersistence,
                                                QSsl::SslOptionDisableServerCipherPreference };
};

static QList<QSsl::SslOption> getSslOptionsFromConfig(const QSslConfiguration &c)
{
    QList<QSsl::SslOption> result;
    for (auto opt : tst_QQmlSslConfiguration::AllOptions) {
        if (c.testSslOption(opt))
            result.append(opt);
    }

    // return a sorted list
    std::sort(result.begin(), result.end());
    return result;
}

static QSsl::SslOptions getSslOptionsAsFlags(const QSslConfiguration &c)
{
    QSsl::SslOptions options{};
    for (auto opt : tst_QQmlSslConfiguration::AllOptions) {
        if (c.testSslOption(opt))
            options |= opt;
    }
    return options;
}

void tst_QQmlSslConfiguration::sslOptionsInSync_data()
{
    QTest::addColumn<ConfType>("type");

    QTest::newRow("DefaultSslConfiguration") << ConfType::DefaultSsl;
    QTest::newRow("DefaultDtlsConfiguration") << ConfType::DefaultDtls;
}

void tst_QQmlSslConfiguration::sslOptionsInSync()
{
#if QT_REMOVAL_QT7_DEPRECATED_SINCE(6, 11)
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
    QFETCH(ConfType, type);
    std::unique_ptr<QQmlSslConfiguration> conf;

    if (type == ConfType::DefaultSsl)
        conf.reset(new QQmlSslDefaultConfiguration);
    else
        conf.reset(new QQmlSslDefaultDtlsConfiguration);

    // Default config has some options, and originally they're in sync
    auto opts = conf->sslOptions();
    std::sort(opts.begin(), opts.end());
    QCOMPARE_EQ(opts, getSslOptionsFromConfig(conf->configuration()));

    // set new options
    QList<QSsl::SslOption> newOptions { QSsl::SslOptionDisableCompression,
                                        QSsl::SslOptionDisableServerCipherPreference };
    conf->setSslOptions(newOptions);

    std::sort(newOptions.begin(), newOptions.end());

    opts = conf->sslOptions();
    std::sort(opts.begin(), opts.end());
    // reading back the correct value
    QCOMPARE_EQ(opts, newOptions);

    // reading the values from the underlying configuration
    QEXPECT_FAIL("", "QTBUG-137900", Continue);
    QCOMPARE_EQ(getSslOptionsFromConfig(conf->configuration()), newOptions);
QT_WARNING_POP
#else
    QSKIP("This test should be removed in Qt 7!")
#endif // QT_REMOVAL_QT7_DEPRECATED_SINCE(6, 11)
}

void tst_QQmlSslConfiguration::sslOptionFlags_data()
{
    QTest::addColumn<ConfType>("type");

    QTest::newRow("DefaultSslConfiguration") << ConfType::DefaultSsl;
    QTest::newRow("DefaultDtlsConfiguration") << ConfType::DefaultDtls;
}

void tst_QQmlSslConfiguration::sslOptionFlags()
{
    QFETCH(const ConfType, type);

    std::unique_ptr<QQmlSslConfiguration> conf;
    QSsl::SslOptions defaultOptions;

    if (type == ConfType::DefaultSsl) {
        conf.reset(new QQmlSslDefaultConfiguration);
        defaultOptions = getSslOptionsAsFlags(QSslConfiguration::defaultConfiguration());
    } else {
        conf.reset(new QQmlSslDefaultDtlsConfiguration);
#if QT_CONFIG(dtls)
        defaultOptions = getSslOptionsAsFlags(QSslConfiguration::defaultDtlsConfiguration());
#else
        defaultOptions = getSslOptionsAsFlags(QSslConfiguration::defaultConfiguration());
#endif // QT_CONFIG(dtls)
    }

    // check default options
    QCOMPARE_EQ(conf->sslOptionFlags(), defaultOptions);
    // check the underlying configuration
    QCOMPARE_EQ(getSslOptionsAsFlags(conf->configuration()), defaultOptions);

    // update to some new value
    QSsl::SslOptions options = QSsl::SslOptionDisableCompression
                                | QSsl::SslOptionDisableServerCipherPreference;

    conf->setSslOptionFlags(options);
    QCOMPARE_EQ(conf->sslOptionFlags(), options);
    // check the underlying configuration
    QCOMPARE_EQ(getSslOptionsAsFlags(conf->configuration()), options);
}

QTEST_MAIN(tst_QQmlSslConfiguration)

#include "tst_qqmlsslconfiguration_cpp.moc"
