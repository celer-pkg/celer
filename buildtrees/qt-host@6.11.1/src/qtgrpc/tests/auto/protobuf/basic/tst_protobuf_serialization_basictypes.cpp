// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "basicmessages.qpb.h"
#include "fieldindexrange.qpb.h"
#include "qtprotobufserializertestdata.h"

#include <QProtobufSerializer>
#include <QTest>

#include <limits>

using namespace Qt::Literals::StringLiterals;

class QtProtobufTypesSerializationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() { m_serializer.reset(new QProtobufSerializer); }

    void boolMessageSerializeTest_data();
    void boolMessageSerializeTest();
    void intMessageSerializeTest_data();
    void intMessageSerializeTest();
    void uIntMessageSerializeTest_data();
    void uIntMessageSerializeTest();
    void sIntMessageSerializeTest_data();
    void sIntMessageSerializeTest();
    void int64MessageSerializeTest_data();
    void int64MessageSerializeTest();
    void uInt64MessageSerializeTest_data();
    void uInt64MessageSerializeTest();
    void sInt64MessageSerializeTest_data();
    void sInt64MessageSerializeTest();
    void fixedInt32MessageSerializeTest_data();
    void fixedInt32MessageSerializeTest();
    void fixedInt64MessageSerializeTest_data();
    void fixedInt64MessageSerializeTest();
    void sFixedInt32MessageSerializeTest_data();
    void sFixedInt32MessageSerializeTest();
    void sFixedInt64MessageSerializeTest_data();
    void sFixedInt64MessageSerializeTest();
    void floatMessageSerializeTest_data();
    void floatMessageSerializeTest();
    void doubleMessageSerializeTest_data();
    void doubleMessageSerializeTest();
    void stringMessageSerializeTest();
    void complexTypeSerializeTest_data();
    void complexTypeSerializeTest();
    void resetComplexTypeSerializeTest_data();
    void resetComplexTypeSerializeTest();
    void defaultConstructedComplexTypeSerializeTest();
    void emptyBytesMessageTest();
    void emptyStringMessageTest();
    void fieldIndexRangeTest();
    void oneofMessageEmptyTest();
    void oneofMessageClearTest();
    void oneofMessageIntTest();
    void oneofMessageComplexTest();

private:
    std::unique_ptr<QProtobufSerializer> m_serializer;
};

using namespace qtprotobufnamespace::tests;

template <typename T, IntTypes type>
void generateTestData()
{
    QTest::addColumn<T>("value");
    QTest::addColumn<qsizetype>("expectedSize");
    QTest::addColumn<QByteArray>("expectedData");

    const auto data = SerializeDataGenerator<T, type>::getSerializeData();
    for (const auto &test : data) {
        auto testName = QString("value_%1")
                                .arg(test.name ? *(test.name) : QString::number(test.value));
        QTest::newRow(testName.toStdString().c_str()) << test.value << test.size << test.hexData;
    }
}

void QtProtobufTypesSerializationTest::boolMessageSerializeTest_data()
{
    QTest::addColumn<bool>("value");
    QTest::addColumn<qsizetype>("expectedSize");
    QTest::addColumn<QByteArray>("expectedData");

    QTest::newRow("True") << true << qsizetype(2) << "0801"_ba;
    QTest::newRow("False") << false << qsizetype(0) << ""_ba;
}

void QtProtobufTypesSerializationTest::boolMessageSerializeTest()
{
    QFETCH(const bool, value);
    QFETCH(const qsizetype, expectedSize);
    QFETCH(const QByteArray, expectedData);

    SimpleBoolMessage test;
    test.setTestFieldBool(value);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.size(), expectedSize);
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::intMessageSerializeTest_data()
{
    generateTestData<int32_t, IntTypes::Int>();
}

void QtProtobufTypesSerializationTest::intMessageSerializeTest()
{
    QFETCH(const int32_t, value);
    QFETCH(const qsizetype, expectedSize);
    QFETCH(const QByteArray, expectedData);

    SimpleIntMessage test;
    test.setTestFieldInt(value);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.size(), expectedSize);
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::uIntMessageSerializeTest_data()
{
    generateTestData<uint32_t, IntTypes::UInt>();
}

void QtProtobufTypesSerializationTest::uIntMessageSerializeTest()
{
    QFETCH(const uint32_t, value);
    QFETCH(const qsizetype, expectedSize);
    QFETCH(const QByteArray, expectedData);

    SimpleUIntMessage test;
    test.setTestFieldInt(value);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.size(), expectedSize);
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::sIntMessageSerializeTest_data()
{
    generateTestData<int32_t, IntTypes::SInt>();
}

void QtProtobufTypesSerializationTest::sIntMessageSerializeTest()
{
    QFETCH(const int32_t, value);
    QFETCH(const qsizetype, expectedSize);
    QFETCH(const QByteArray, expectedData);

    SimpleSIntMessage test;
    test.setTestFieldInt(value);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.size(), expectedSize);
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::int64MessageSerializeTest_data()
{
    generateTestData<int64_t, IntTypes::Int>();
}
void QtProtobufTypesSerializationTest::int64MessageSerializeTest()
{
    QFETCH(const int64_t, value);
    QFETCH(const qsizetype, expectedSize);
    QFETCH(const QByteArray, expectedData);

    SimpleInt64Message test;
    test.setTestFieldInt(value);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.size(), expectedSize);
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::uInt64MessageSerializeTest_data()
{
    generateTestData<uint64_t, IntTypes::UInt>();
}
void QtProtobufTypesSerializationTest::uInt64MessageSerializeTest()
{
    QFETCH(const QtProtobuf::uint64, value);
    QFETCH(const qsizetype, expectedSize);
    QFETCH(const QByteArray, expectedData);

    SimpleUInt64Message test;
    test.setTestFieldInt(value);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.size(), expectedSize);
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::sInt64MessageSerializeTest_data()
{
    generateTestData<int64_t, IntTypes::SInt>();
}
void QtProtobufTypesSerializationTest::sInt64MessageSerializeTest()
{
    QFETCH(const int64_t, value);
    QFETCH(const qsizetype, expectedSize);
    QFETCH(const QByteArray, expectedData);

    SimpleSInt64Message test;
    test.setTestFieldInt(value);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.size(), expectedSize);
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::fixedInt32MessageSerializeTest_data()
{
    generateTestData<uint32_t, IntTypes::Fixed>();
}

void QtProtobufTypesSerializationTest::fixedInt32MessageSerializeTest()
{
    QFETCH(const uint32_t, value);
    QFETCH(const qsizetype, expectedSize);
    QFETCH(const QByteArray, expectedData);

    SimpleFixedInt32Message test;
    test.setTestFieldFixedInt32(value);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.size(), expectedSize);
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::fixedInt64MessageSerializeTest_data()
{
    generateTestData<uint64_t, IntTypes::Fixed>();
}

void QtProtobufTypesSerializationTest::fixedInt64MessageSerializeTest()
{
    QFETCH(const uint64_t, value);
    QFETCH(const qsizetype, expectedSize);
    QFETCH(const QByteArray, expectedData);

    SimpleFixedInt64Message test;
    test.setTestFieldFixedInt64(value);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.size(), expectedSize);
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::sFixedInt32MessageSerializeTest_data()
{
    generateTestData<int32_t, IntTypes::SFixed>();
}
void QtProtobufTypesSerializationTest::sFixedInt32MessageSerializeTest()
{
    QFETCH(const int32_t, value);
    QFETCH(const qsizetype, expectedSize);
    QFETCH(const QByteArray, expectedData);

    SimpleSFixedInt32Message test;
    test.setTestFieldFixedInt32(value);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.size(), expectedSize);
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::sFixedInt64MessageSerializeTest_data()
{
    generateTestData<int64_t, IntTypes::SFixed>();
}

void QtProtobufTypesSerializationTest::sFixedInt64MessageSerializeTest()
{
    QFETCH(const int64_t, value);
    QFETCH(const qsizetype, expectedSize);
    QFETCH(const QByteArray, expectedData);

    SimpleSFixedInt64Message test;
    test.setTestFieldFixedInt64(value);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.size(), expectedSize);
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::floatMessageSerializeTest_data()
{
    QTest::addColumn<float>("value");
    QTest::addColumn<qsizetype>("expectedSize");
    QTest::addColumn<QByteArray>("expectedData");
    QTest::addColumn<std::optional<int>>("optionalAt0");

    constexpr qsizetype FloatMessageSize = 5;

    QTest::newRow("float_value_0_1")
            << 0.1f << FloatMessageSize << "3dcdcccc3d"_ba << std::optional<int>();
    QTest::newRow("float_value_min") << std::numeric_limits<float>::min() << FloatMessageSize
                                     << "3d00008000"_ba << std::optional<int>();
    QTest::newRow("float_value_max") << std::numeric_limits<float>::max() << FloatMessageSize
                                     << "3dffff7f7f"_ba << std::make_optional(0x3d);
    QTest::newRow("float_neg_value_4_2")
            << -4.2f << FloatMessageSize << "3d666686c0"_ba << std::make_optional(0x3d);
    QTest::newRow("float_neg_value_0_0") << -0.0f << qsizetype(0) << ""_ba << std::optional<int>();
}

void QtProtobufTypesSerializationTest::floatMessageSerializeTest()
{
    QFETCH(const float, value);
    QFETCH(const qsizetype, expectedSize);
    QFETCH(const QByteArray, expectedData);
    QFETCH(const std::optional<int>, optionalAt0);

    SimpleFloatMessage test;
    test.setTestFieldFloat(value);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.size(), expectedSize);
    if (optionalAt0)
        QCOMPARE(result.at(0), *optionalAt0);
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::doubleMessageSerializeTest_data()
{
    QTest::addColumn<double>("value");
    QTest::addColumn<qsizetype>("expectedSize");
    QTest::addColumn<QByteArray>("expectedData");

    constexpr qsizetype DoubleMessageSize = 9;

    QTest::newRow("double_value_0_1") << 0.1 << DoubleMessageSize << "419a9999999999b93f"_ba;
    QTest::newRow("double_value_min")
            << std::numeric_limits<double>::min() << DoubleMessageSize << "410000000000001000"_ba;
    QTest::newRow("double_value_max")
            << std::numeric_limits<double>::max() << DoubleMessageSize << "41ffffffffffffef7f"_ba;
    QTest::newRow("double_neg_value_4_2") << -4.2 << DoubleMessageSize << "41cdcccccccccc10c0"_ba;
    QTest::newRow("double_value_0_0") << 0.0 << qsizetype(0) << ""_ba;
}

void QtProtobufTypesSerializationTest::doubleMessageSerializeTest()
{
    QFETCH(const double, value);
    QFETCH(const qsizetype, expectedSize);
    QFETCH(const QByteArray, expectedData);

    SimpleDoubleMessage test;
    test.setTestFieldDouble(value);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.size(), expectedSize);
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::stringMessageSerializeTest()
{
    SimpleStringMessage test;
    test.setTestFieldString("qwerty");
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), "3206717765727479"_ba);

    test.setTestFieldString("oepSNLIVG08UJpk2W7JtTkkBxyK06X0lQ6ML7IMd55K8XC1Tps"
                            "c1kDWym5v8z68b4FQu"
                            "p9O95QSgAvjHIA15OX6Bu68esbQFT9LPzSADJ6qSGBTYBHX5QS"
                            "Zg32trCdHMj80XuDHq"
                            "yBgM4uf6RKq2mgWb8Ovxxr0NwLxjHOfhJ8Mrfd2R7hbUgjespb"
                            "YoQhbgHEj2gKEV3Qvn"
                            "umYmrVXe1BkCzZhKVXodDhj0OfAE67viAy4i3Oag1hr1z4Azo8"
                            "O5Xq68POEZ1CsZPo2D"
                            "XNNR8ebVCdYOz0Q6JLPSl5jasLCFrQN7EiVNjQmCrSsZHRgLNy"
                            "lvgoEFxGYxXJ9gmK4m"
                            "r0OGdZcGJORRGZOQCpQMhXmhezFalNIJXMPPXaRVXiRhRAPCNU"
                            "Eie8DtaCWAMqz4nNUx"
                            "RMZ5UcXBXsXPshygzkyyXnNWTIDojFlrcsnKqSkQ1G6E85gSZb"
                            "tIYBh7sqO6GDXHjOrX"
                            "VaVCVCUubjcJKThlyslt29zHuIs5JGppXxX1");
    result = test.serialize(m_serializer.get());
    QCOMPARE(result.toHex(),
             "3280046f6570534e4c4956473038554a706b3257374a74546"
             "b6b4278794b303658306c51364d4c37494d6435354b385843"
             "3154707363316b4457796d3576387a3638623446517570394"
             "f393551536741766a48494131354f58364275363865736251"
             "4654394c507a5341444a367153474254594248583551535a6"
             "7333274724364484d6a383058754448717942674d34756636"
             "524b71326d675762384f76787872304e774c786a484f66684"
             "a384d726664325237686255676a65737062596f5168626748"
             "456a32674b45563351766e756d596d7256586531426b437a5"
             "a684b56586f6444686a304f6641453637766941793469334f"
             "6167316872317a34417a6f384f3558713638504f455a31437"
             "35a506f3244584e4e52386562564364594f7a3051364a4c50"
             "536c356a61734c434672514e374569564e6a516d437253735"
             "a4852674c4e796c76676f454678475978584a39676d4b346d"
             "72304f47645a63474a4f5252475a4f514370514d68586d686"
             "57a46616c4e494a584d50505861525658695268524150434e"
             "55456965384474614357414d717a346e4e5578524d5a35556"
             "3584258735850736879677a6b7979586e4e575449446f6a46"
             "6c7263736e4b71536b5131473645383567535a62744959426"
             "83773714f36474458486a4f72585661564356435575626a63"
             "4a4b54686c79736c7432397a487549"
             "73354a47707058785831"_ba);
}

void QtProtobufTypesSerializationTest::complexTypeSerializeTest_data()
{
    QTest::addColumn<int>("intField");
    QTest::addColumn<QString>("stringField");
    QTest::addColumn<QByteArray>("expectedData");

    QTest::newRow("empty_value") << 0 << u""_s << "1200"_ba;
    QTest::newRow("value_only_int") << 42 << u""_s << "082a1200"_ba;
    QTest::newRow("value_only_string") << 0 << u"qwerty"_s << "12083206717765727479"_ba;
    QTest::newRow("int_and_string") << 42 << u"qwerty"_s << "082a12083206717765727479"_ba;
    QTest::newRow("int_and_big_string") << 42
                                        << u"YVRfJvjxqbgvFwS1YvOZXgtj5ffGLS7AiNHz9oZIoKbm7z"
                                           "8H79xBuyPkpQXvGoO09OY9xRawx3eOAs9xjoTA1xJhrw28"
                                           "TAcq1CebYlC9WUfQC6hIantaNdyHiKToffi0Zt7la42SRx"
                                           "XZSP4GuxbcZIp53pJnyCwfCy1qdFczT0dmn7h8fpyAdemE"
                                           "avwFeda4d0PApGfSU2jLt39X8kYUBxNM2WgALRBgHdVde8"
                                           "7q6Pi5U69TjhMd28W1SFD1DxyogCCrqOct2ZPICoLnrqdF"
                                           "3OdNzjRVLfeyvQ8LgLvRNFR9WfWAyAz79nKgBamd8Ntlvt"
                                           "4Mg35E5gVS2g7AQ7rkm72cBdnW9sCEyGabeXAuH5j4GRbu"
                                           "LT7qBZWDcFLF4SsCdS3WfFGdNHfwaijzykByo71PvFVlTX"
                                           "H2WJWoFvR5FALjBTn7bCdP0pAiSbLCY8Xz2Msc3dBb5Ff9"
                                           "GISPbUpNmUvBdMZMHQvqOmTNXEPpN0b74MDOMQfWJShOo3NkAvMjs"_s
                                        << "082a128404328104595652664a766a78716267764"
                                           "677533159764f5a5867746a356666474c53374169"
                                           "4e487a396f5a496f4b626d377a384837397842757"
                                           "9506b70515876476f4f30394f5939785261777833"
                                           "654f417339786a6f544131784a687277323854416"
                                           "37131436562596c43395755665143366849616e74"
                                           "614e647948694b546f666669305a74376c6134325"
                                           "35278585a53503447757862635a49703533704a6e"
                                           "79437766437931716446637a5430646d6e3768386"
                                           "670794164656d4561767746656461346430504170"
                                           "47665355326a4c74333958386b595542784e4d325"
                                           "767414c5242674864566465383771365069355536"
                                           "39546a684d6432385731534644314478796f67434"
                                           "372714f6374325a5049436f4c6e72716446334f64"
                                           "4e7a6a52564c6665797651384c674c76524e46523"
                                           "95766574179417a37396e4b6742616d64384e746c"
                                           "7674344d6733354535675653326737415137726b6"
                                           "d37326342646e5739734345794761626558417548"
                                           "356a34475262754c543771425a574463464c46345"
                                           "3734364533357664647644e48667761696a7a796b"
                                           "42796f3731507646566c54584832574a576f46765"
                                           "23546414c6a42546e37624364503070416953624c"
                                           "435938587a324d736333644262354666394749535"
                                           "06255704e6d557642644d5a4d485176714f6d544e"
                                           "584550704e306237344d444f4d5166574a53684f6"
                                           "f334e6b41764d6a73"_ba;
    QTest::newRow("neg_int_and_string")
            << -45 << u"qwerty"_s << "08d3ffffffffffffffff0112083206717765727479"_ba;
}

void QtProtobufTypesSerializationTest::complexTypeSerializeTest()
{
    QFETCH(const int, intField);
    QFETCH(const QString, stringField);
    QFETCH(const QByteArray, expectedData);

    SimpleStringMessage stringMessage;
    stringMessage.setTestFieldString(stringField);
    ComplexMessage test;
    test.setTestFieldInt(intField);
    test.setTestComplexField(stringMessage);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::resetComplexTypeSerializeTest_data()
{
    QTest::addColumn<int>("intField");
    QTest::addColumn<QString>("stringField");
    QTest::addColumn<QByteArray>("expectedData");

    QTest::newRow("empty_value") << 0 << u""_s << ""_ba;
    QTest::newRow("value_only_int") << 42 << u""_s << "082a"_ba;
    QTest::newRow("value_only_string") << 0 << u"qwerty"_s << ""_ba;
    QTest::newRow("int_and_string") << 42 << u"qwerty"_s << "082a"_ba;
}

void QtProtobufTypesSerializationTest::resetComplexTypeSerializeTest()
{
    QFETCH(const int, intField);
    QFETCH(const QString, stringField);
    QFETCH(const QByteArray, expectedData);

    SimpleStringMessage stringMessage;
    stringMessage.setTestFieldString(stringField);
    ComplexMessage test;
    test.setTestFieldInt(intField);
    test.setTestComplexField(stringMessage);

    // Reset Complex field
    test.clearTestComplexField();
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), expectedData);
}

void QtProtobufTypesSerializationTest::defaultConstructedComplexTypeSerializeTest()
{
    ComplexMessage test;
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), ""_ba);
}

void QtProtobufTypesSerializationTest::emptyBytesMessageTest()
{
    SimpleBytesMessage msg;

    QByteArray result = msg.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufTypesSerializationTest::emptyStringMessageTest()
{
    SimpleStringMessage msg;

    QByteArray result = msg.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufTypesSerializationTest::fieldIndexRangeTest()
{
    FieldIndexTest1Message msg1;
    msg1.setTestField(1);
    QByteArray result = msg1.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), "f80102"_ba);

    FieldIndexTest2Message msg2;
    msg2.setTestField(1);
    result = msg2.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), "f8ff0302"_ba);

    FieldIndexTest3Message msg3;
    msg3.setTestField(1);
    result = msg3.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), "f8ffff0702"_ba);

    FieldIndexTest4Message msg4;
    msg4.setTestField(1);
    result = msg4.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), "f8ffffff0f02"_ba);
}

void QtProtobufTypesSerializationTest::oneofMessageEmptyTest()
{
    OneofMessage test;
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), "");
}

void QtProtobufTypesSerializationTest::oneofMessageIntTest()
{
    OneofMessage test;
    test.setTestFieldInt(-45);
    test.setTestOneofFieldInt(-45);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.toHex(),
             "08d3ffffffffffffffff01"
             "d002d3ffffffffffffffff01");

    test.setTestFieldInt(0);
    test.setTestOneofFieldInt(0);
    result = test.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), "d00200");
}

void QtProtobufTypesSerializationTest::oneofMessageClearTest()
{
    OneofMessage test;
    test.setTestFieldInt(-45);
    test.setTestOneofFieldInt(-45);

    test.clearTestOneof();
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), "08d3ffffffffffffffff01");
}

void QtProtobufTypesSerializationTest::oneofMessageComplexTest()
{
    ComplexMessage complexField;
    SimpleStringMessage stringField;
    stringField.setTestFieldString("qwerty");
    complexField.setTestFieldInt(42);
    complexField.setTestComplexField(stringField);

    OneofMessage test;
    test.setTestOneofComplexField(complexField);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), "1a0c082a12083206717765727479");

    test.clearTestOneof();

    test.setTestOneofComplexField({});
    result = test.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), "1a00");

    test.setSecondComplexField({});
    result = test.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), "1a002a00");

    // Check that partially intialized complex field doesn't apply its 'oneof'
    // peculiarity to the child fields.
    complexField.setTestComplexField({});
    complexField.setTestFieldInt(42);
    test.setTestOneofComplexField(complexField);
    test.setSecondComplexField({});
    result = test.serialize(m_serializer.get());
    QCOMPARE(result.toHex(), "1a04082a12002a00");
}

QTEST_MAIN(QtProtobufTypesSerializationTest)
#include "tst_protobuf_serialization_basictypes.moc"
