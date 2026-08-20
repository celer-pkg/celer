// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>
#include <QCoreApplication>

#include <QtCoap/qcoapmessage.h>

class tst_QCoapMessage : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void copyAndDetach();
    void setType_data();
    void setType();
    void addOption();
    void addOption_string_data();
    void addOption_string();
    void addOption_uint_data();
    void addOption_uint();
    void removeOption_data();
    void removeOption();
    void removeOptionByName_data();
    void removeOptionByName();
    void removeAll();
};

void tst_QCoapMessage::copyAndDetach()
{
    QCoapMessage a;
    a.setMessageId(3);
    a.setPayload("payload");
    a.setToken("token");
    a.setType(QCoapMessage::Type::Acknowledgment);
    a.setVersion(5);

    // Test the copy
    QCoapMessage b(a);
    QVERIFY2(b.messageId() == 3, "Message not copied correctly");
    QVERIFY2(b.payload() == "payload", "Message not copied correctly");
    QVERIFY2(b.token() == "token", "Message not copied correctly");
    QVERIFY2(b.type() == QCoapMessage::Type::Acknowledgment, "Message not copied correctly");
    QVERIFY2(b.version() == 5, "Message not copied correctly");

    // Detach
    b.setMessageId(9);
    QCOMPARE(b.messageId(), 9);
    QCOMPARE(a.messageId(), 3);
}

void tst_QCoapMessage::setType_data()
{
    QTest::addColumn<QCoapMessage::Type>("type");

    QTest::newRow("acknowledgment") << QCoapMessage::Type::Acknowledgment;
    QTest::newRow("confirmable") << QCoapMessage::Type::Confirmable;
    QTest::newRow("non-confirmable") << QCoapMessage::Type::NonConfirmable;
    QTest::newRow("reset") << QCoapMessage::Type::Reset;
}

void tst_QCoapMessage::setType()
{
    QFETCH(QCoapMessage::Type, type);
    QCoapMessage message;
    message.setType(type);
    QCOMPARE(message.type(), type);
}

void tst_QCoapMessage::addOption()
{
    QCoapMessage message;

    QList<QCoapOption::OptionName> optionNames = {
        QCoapOption::ProxyUri,
        QCoapOption::UriHost,
        QCoapOption::LocationQuery,
        QCoapOption::ProxyUri,
        QCoapOption::UriHost,
        QCoapOption::UriQuery
    };
    QVERIFY(!std::is_sorted(optionNames.cbegin(), optionNames.cend()));

    const QByteArray value("\xAF\x01\xC2");
    for (const auto& name : optionNames)
        message.addOption(name, value);

    QCOMPARE(message.optionCount(), optionNames.size());
    QVERIFY(std::is_sorted(message.options().cbegin(), message.options().cend(),
                           [](const QCoapOption &a, const QCoapOption &b) -> bool {
                               return a.name() < b.name();
           }));

    for (const auto& name : optionNames)
        QVERIFY2(message.hasOption(name), qPrintable(QString("Missing option %1").arg(name)));

    QVERIFY(std::all_of(message.options().cbegin(), message.options().cend(),
                        [value](const QCoapOption opt) -> bool {
                            return opt.opaqueValue() == value;
           }));
}

void tst_QCoapMessage::addOption_string_data()
{
    QTest::addColumn<QList<QCoapOption>>("options");

    QList<QCoapOption> single_string_option = { { QCoapOption::LocationPath, QString("path1") } };
    QList<QCoapOption> single_ba_option = {
        { QCoapOption::LocationPath, QByteArray("\xAF\x01\xC2") }
    };
    QList<QCoapOption> multiple_string_options = {
        { QCoapOption::LocationPath, QString("str_path2") },
        { QCoapOption::LocationPath, QString("str_path3") }
    };

    QTest::newRow("single_char_option") << single_string_option;
    QTest::newRow("single_ba_option") << single_ba_option;
    QTest::newRow("multiple_string_options") << multiple_string_options;
}

void tst_QCoapMessage::addOption_string()
{
    QFETCH(QList<QCoapOption>, options);

    QCoapMessage message;
    for (const auto& option : options)
        message.addOption(option);

    QCOMPARE(message.optionCount(), options.size());
    for (const auto& option : options)
    {
        const auto it = std::find(message.options().cbegin(), message.options().cend(), option);
        QVERIFY(it != message.options().cend());
    }
}

void tst_QCoapMessage::addOption_uint_data()
{
    QTest::addColumn<quint32>("value");
    QTest::addColumn<int>("size");

    QTest::newRow("4 bytes") << static_cast<quint32>(0xF0aF0010) << 4;
    QTest::newRow("3 bytes") << static_cast<quint32>(0x300010) << 3;
    QTest::newRow("2 bytes") << static_cast<quint32>(0x5010) << 2;
    QTest::newRow("1 byte")  << static_cast<quint32>(0x80) << 1;
}

void tst_QCoapMessage::addOption_uint()
{
    QFETCH(quint32, value);
    QFETCH(int, size);

    const auto name = QCoapOption::Block1;
    const QCoapOption option(name, value);

    QCoapMessage message;
    message.addOption(option);

    QCOMPARE(message.options(name).size(), 1);
    QCOMPARE(message.option(name).uintValue(), value);
    QCOMPARE(option.opaqueValue().size(), size);
}

void tst_QCoapMessage::removeOption_data()
{
    QTest::addColumn<QList<QCoapOption>>("options");

    QList<QCoapOption> single_option = { { QCoapOption::LocationPath, QByteArray("path1") } };
    QList<QCoapOption> multiple_options = {
        { QCoapOption::LocationPath, QByteArray("path2") },
        { QCoapOption::LocationPath, QByteArray("path3") }
    };

    QTest::newRow("single_option") << single_option;
    QTest::newRow("multiple_options") << multiple_options;
}

void tst_QCoapMessage::removeOption()
{
    QFETCH(QList<QCoapOption>, options);

    QCoapMessage message;
    for (const auto& option : options)
        message.addOption(option);

    for (const auto& option : options)
    {
        // Make sure option is present before removal
        auto it = std::find(message.options().cbegin(), message.options().cend(), option);
        QVERIFY(it != message.options().end());

        message.removeOption(option);
        it = std::find(message.options().cbegin(), message.options().cend(), option);
        QVERIFY(it == message.options().end());
    }
}

void tst_QCoapMessage::removeOptionByName_data()
{
    QTest::addColumn<QList<QCoapOption>>("options");
    QTest::addColumn<QCoapOption::OptionName>("name");

    QList<QCoapOption> single_option = { { QCoapOption::LocationPath, QByteArray("path1") } };
    QList<QCoapOption> multiple_options = {
        { QCoapOption::LocationPath, QByteArray("path2") },
        { QCoapOption::LocationPath, QByteArray("path3") }
    };

    QTest::newRow("remove_single_option") << single_option << single_option.back().name();
    QTest::newRow("remove_multiple_options") << multiple_options << multiple_options.back().name();
}

void tst_QCoapMessage::removeOptionByName()
{
    QFETCH(QList<QCoapOption>, options);
    QFETCH(QCoapOption::OptionName, name);

    QCoapMessage message;
    for (const auto& option : options)
        message.addOption(option);

    message.removeOption(name);
    QVERIFY(!message.hasOption(name));
}

void tst_QCoapMessage::removeAll()
{
    QCoapMessage message;
    message.addOption(QCoapOption::LocationPath, "path");
    message.addOption(QCoapOption::ProxyUri, "proxy1");
    message.addOption(QCoapOption::ProxyUri, "proxy2");
    message.clearOptions();

    QVERIFY(message.options().isEmpty());
}

QTEST_APPLESS_MAIN(tst_QCoapMessage)

#include "tst_qcoapmessage.moc"
