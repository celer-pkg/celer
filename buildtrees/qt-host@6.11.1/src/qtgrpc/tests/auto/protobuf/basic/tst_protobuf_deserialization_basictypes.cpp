// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "basicmessages.qpb.h"
#include "fieldindexrange.qpb.h"
#include "qtprotobufserializertestdata.h"

#include <QObject>
#include <QProtobufSerializer>
#include <QTest>

#include <limits>

using namespace Qt::Literals::StringLiterals;

class QtProtobufTypesDeserializationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() { serializer.reset(new QProtobufSerializer); }

    void fixedInt32MessageDeserializeTest_data();
    void fixedInt32MessageDeserializeTest();
    void fixedInt64MessageDeserializeTest_data();
    void fixedInt64MessageDeserializeTest();
    void sFixedInt32MessageDeserializeTest_data();
    void sFixedInt32MessageDeserializeTest();
    void sFixedInt64MessageDeserializeTest_data();
    void sFixedInt64MessageDeserializeTest();
    void floatMessageDeserializeTest_data();
    void floatMessageDeserializeTest();
    void doubleMessageDeserializeTest_data();
    void doubleMessageDeserializeTest();
    void intMessageDeserializeTest_data();
    void intMessageDeserializeTest();
    void stringMessageDeserializeTest_data();
    void stringMessageDeserializeTest();
    void complexTypeDeserializeTest_data();
    void complexTypeDeserializeTest();
    void sIntMessageDeserializeTest_data();
    void sIntMessageDeserializeTest();
    void uIntMessageDeserializeTest_data();
    void uIntMessageDeserializeTest();
    void boolDeserializeTest_data();
    void boolDeserializeTest();
    void redundantFieldIsIgnoredAtDeserializationTest_data();
    void redundantFieldIsIgnoredAtDeserializationTest();
    void fieldIndexRangeTest();
    void oneofMessageTest();
    void oneofMessageEmptyTest();
    void oneofMessageMultipleFieldsTest();

private:
    std::unique_ptr<QProtobufSerializer> serializer;
};

using namespace qtprotobufnamespace::tests;

#define EMPTY_VALUE_TEST_ROW(type) QTest::newRow("empty_data") << ""_ba << (type)0

template <typename T, IntTypes type>
void generateTestData()
{
    QTest::addColumn<QByteArray>("hexData");
    QTest::addColumn<T>("expectedValue");

    EMPTY_VALUE_TEST_ROW(T);
    const auto data = SerializeDataGenerator<T, type>::getSerializeData();
    for (const auto &test : data) {
        auto testName = QString("value_%1")
                                .arg(test.name ? *(test.name) : QString::number(test.value));
        QTest::newRow(testName.toStdString().c_str()) << test.hexData << test.value;
    }
}

void QtProtobufTypesDeserializationTest::fixedInt32MessageDeserializeTest_data()
{
    generateTestData<uint32_t, IntTypes::Fixed>();
}

void QtProtobufTypesDeserializationTest::fixedInt32MessageDeserializeTest()
{
    QFETCH(const QByteArray, hexData);
    QFETCH(const uint32_t, expectedValue);

    SimpleFixedInt32Message test;
    test.deserialize(serializer.get(), QByteArray::fromHex(hexData));
    QCOMPARE(test.testFieldFixedInt32(), expectedValue);
}

void QtProtobufTypesDeserializationTest::fixedInt64MessageDeserializeTest_data()
{
    generateTestData<uint64_t, IntTypes::Fixed>();
}

void QtProtobufTypesDeserializationTest::fixedInt64MessageDeserializeTest()
{
    QFETCH(const QByteArray, hexData);
    QFETCH(const uint64_t, expectedValue);

    SimpleFixedInt64Message test;
    test.deserialize(serializer.get(), QByteArray::fromHex(hexData));
    QCOMPARE(test.testFieldFixedInt64(), expectedValue);
}

void QtProtobufTypesDeserializationTest::sFixedInt32MessageDeserializeTest_data()
{
    generateTestData<int32_t, IntTypes::SFixed>();
}

void QtProtobufTypesDeserializationTest::sFixedInt32MessageDeserializeTest()
{
    QFETCH(const QByteArray, hexData);
    QFETCH(const int32_t, expectedValue);

    SimpleSFixedInt32Message test;
    test.deserialize(serializer.get(), QByteArray::fromHex(hexData));
    QCOMPARE(test.testFieldFixedInt32(), expectedValue);
}

void QtProtobufTypesDeserializationTest::sFixedInt64MessageDeserializeTest_data()
{
    generateTestData<int64_t, IntTypes::SFixed>();
}

void QtProtobufTypesDeserializationTest::sFixedInt64MessageDeserializeTest()
{
    QFETCH(const QByteArray, hexData);
    QFETCH(const int64_t, expectedValue);

    SimpleSFixedInt64Message test;
    test.deserialize(serializer.get(), QByteArray::fromHex(hexData));
    QCOMPARE(test.testFieldFixedInt64(), expectedValue);
}

void QtProtobufTypesDeserializationTest::floatMessageDeserializeTest_data()
{
    QTest::addColumn<QByteArray>("hexData");
    QTest::addColumn<float>("expectedValue");

    EMPTY_VALUE_TEST_ROW(float);
    QTest::newRow("value_-4.2") << "3d666686c0"_ba << -4.2f;
    QTest::newRow("value_-0.0") << "3d00000000"_ba << -0.0f;
    QTest::newRow("value_0.1") << "3dcdcccc3d"_ba << 0.1f;
    QTest::newRow("value_float_min") << "3d00008000"_ba << std::numeric_limits<float>::min();
    QTest::newRow("value_float_max") << "3dffff7f7f"_ba << std::numeric_limits<float>::max();
}

void QtProtobufTypesDeserializationTest::floatMessageDeserializeTest()
{
    QFETCH(const QByteArray, hexData);
    QFETCH(const float, expectedValue);

    SimpleFloatMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex(hexData));
    QCOMPARE(test.testFieldFloat(), expectedValue);
}

void QtProtobufTypesDeserializationTest::doubleMessageDeserializeTest_data()
{
    QTest::addColumn<QByteArray>("hexData");
    QTest::addColumn<double>("expectedValue");

    EMPTY_VALUE_TEST_ROW(double);
    QTest::newRow("value_-4.2") << "41cdcccccccccc10c0"_ba << -4.2;
    QTest::newRow("value_0.0") << "410000000000000000"_ba << 0.0;
    QTest::newRow("value_0.1") << "419a9999999999b93f"_ba << 0.1;
    QTest::newRow("value_double_min")
            << "410000000000001000"_ba << std::numeric_limits<double>::min();
    QTest::newRow("value_double_max")
            << "41ffffffffffffef7f"_ba << std::numeric_limits<double>::max();
}

void QtProtobufTypesDeserializationTest::doubleMessageDeserializeTest()
{
    QFETCH(const QByteArray, hexData);
    QFETCH(const double, expectedValue);

    SimpleDoubleMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex(hexData));
    QCOMPARE(test.testFieldDouble(), expectedValue);
}

void QtProtobufTypesDeserializationTest::intMessageDeserializeTest_data()
{
    generateTestData<int32_t, IntTypes::Int>();
}

void QtProtobufTypesDeserializationTest::intMessageDeserializeTest()
{
    QFETCH(const QByteArray, hexData);
    QFETCH(const int32_t, expectedValue);

    SimpleIntMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex(hexData));
    QCOMPARE(test.testFieldInt(), expectedValue);
}

void QtProtobufTypesDeserializationTest::stringMessageDeserializeTest_data()
{
    QTest::addColumn<QByteArray>("hexData");
    QTest::addColumn<QString>("expectedValue");

    QTest::newRow("empty_value") << ""_ba << u""_s;
    QTest::newRow("simple_string") << "3206717765727479"_ba << u"qwerty"_s;
    QTest::newRow("complex_string")
            << "3280046f6570534e4c4956473038554a706b3257374a74546b6b4278794b303658306c51364d4c"
               "37494d6435354b3858433154707363316b4457796d3576387a3638623446517570394f39355153"
               "6741766a48494131354f583642753638657362514654394c507a5341444a367153474254594248"
               "583551535a67333274724364484d6a383058754448717942674d34756636524b71326d67576238"
               "4f76787872304e774c786a484f66684a384d726664325237686255676a65737062596f51686267"
               "48456a32674b45563351766e756d596d7256586531426b437a5a684b56586f6444686a304f6641"
               "453637766941793469334f6167316872317a34417a6f384f3558713638504f455a3143735a506f"
               "3244584e4e52386562564364594f7a3051364a4c50536c356a61734c434672514e374569564e6a"
               "516d437253735a4852674c4e796c76676f454678475978584a39676d4b346d72304f47645a6347"
               "4a4f5252475a4f514370514d68586d68657a46616c4e494a584d50505861525658695268524150"
               "434e55456965384474614357414d717a346e4e5578524d5a355563584258735850736879677a6b"
               "7979586e4e575449446f6a466c7263736e4b71536b5131473645383567535a6274495942683773"
               "714f36474458486a4f72585661564356435575626a634a4b54686c79736c7432397a4875497335"
               "4a47707058785831"_ba
            << u"oepSNLIVG08UJpk2W7JtTkkBxyK06X0lQ6ML7IMd55K8XC1Tpsc1kDWym5v8z68b4FQup9O95QSgAvjHIA"
               "15OX6Bu68esbQFT9LPzSADJ6qSGBTYBHX5QSZg32trCdHMj80XuDHqyBgM4uf6RKq2mgWb8Ovxxr0NwLxjH"
               "OfhJ8Mrfd2R7hbUgjespbYoQhbgHEj2gKEV3QvnumYmrVXe1BkCzZhKVXodDhj0OfAE67viAy4i3Oag1hr1"
               "z4Azo8O5Xq68POEZ1CsZPo2DXNNR8ebVCdYOz0Q6JLPSl5jasLCFrQN7EiVNjQmCrSsZHRgLNylvgoEFxGY"
               "xXJ9gmK4mr0OGdZcGJORRGZOQCpQMhXmhezFalNIJXMPPXaRVXiRhRAPCNUEie8DtaCWAMqz4nNUxRMZ5Uc"
               "XBXsXPshygzkyyXnNWTIDojFlrcsnKqSkQ1G6E85gSZbtIYBh7sqO6GDXHjOrXVaVCVCUubjcJKThlyslt2"
               "9zHuIs5JGppXxX1"_s;
}

void QtProtobufTypesDeserializationTest::stringMessageDeserializeTest()
{
    QFETCH(const QByteArray, hexData);
    QFETCH(const QString, expectedValue);

    SimpleStringMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex(hexData));
    QCOMPARE(test.testFieldString().toStdString().c_str(), expectedValue);
}

void QtProtobufTypesDeserializationTest::complexTypeDeserializeTest_data()
{
    QTest::addColumn<QByteArray>("hexData");
    QTest::addColumn<int32_t>("intField");
    QTest::addColumn<QString>("stringField");

    QTest::newRow("empty_value") << ""_ba << 0 << u""_s;
    QTest::newRow("only_int") << "08d3ffffffffffffffff01"_ba << -45 << u""_s;
    QTest::newRow("only_string") << "12083206717765727479"_ba << 0 << u"qwerty"_s;
    QTest::newRow("int_and_string")
            << "1208320671776572747908d3ffffffffffffffff01"_ba << -45 << u"qwerty"_s;
    QTest::newRow("int_and_string2")
            << "1208320671776572747908d3ffffffffffffffff01"_ba << -45 << u"qwerty"_s;
    QTest::newRow("int_and_utf8_string")
            << "128404328104595652664a766a78716267764677533159764f5a5867746a356666474c53374169"
               "4e487a396f5a496f4b626d377a3848373978427579506b70515876476f4f30394f593978526177"
               "7833654f417339786a6f544131784a68727732385441637131436562596c433957556651433668"
               "49616e74614e647948694b546f666669305a74376c613432535278585a53503447757862635a49"
               "703533704a6e79437766437931716446637a5430646d6e3768386670794164656d456176774665"
               "646134643050417047665355326a4c74333958386b595542784e4d325767414c52426748645664"
               "6538377136506935553639546a684d6432385731534644314478796f67434372714f6374325a50"
               "49436f4c6e72716446334f644e7a6a52564c6665797651384c674c76524e465239576657417941"
               "7a37396e4b6742616d64384e746c7674344d6733354535675653326737415137726b6d37326342"
               "646e5739734345794761626558417548356a34475262754c543771425a574463464c4634537343"
               "64533357664647644e48667761696a7a796b42796f3731507646566c54584832574a576f467652"
               "3546414c6a42546e37624364503070416953624c435938587a324d736333644262354666394749"
               "53506255704e6d557642644d5a4d485176714f6d544e584550704e306237344d444f4d5166574a"
               "53684f6f334e6b41764d6a73082a"_ba
            << 42
            << u"YVRfJvjxqbgvFwS1YvOZXgtj5ffGLS7AiNHz9oZIoKbm7z8H79xBuyPkpQXvGoO09OY9xRawx3eOA"
               "s9xjoTA1xJhrw28TAcq1CebYlC9WUfQC6hIantaNdyHiKToffi0Zt7la42SRxXZSP4GuxbcZIp53p"
               "JnyCwfCy1qdFczT0dmn7h8fpyAdemEavwFeda4d0PApGfSU2jLt39X8kYUBxNM2WgALRBgHdVde87"
               "q6Pi5U69TjhMd28W1SFD1DxyogCCrqOct2ZPICoLnrqdF3OdNzjRVLfeyvQ8LgLvRNFR9WfWAyAz7"
               "9nKgBamd8Ntlvt4Mg35E5gVS2g7AQ7rkm72cBdnW9sCEyGabeXAuH5j4GRbuLT7qBZWDcFLF4SsCd"
               "S3WfFGdNHfwaijzykByo71PvFVlTXH2WJWoFvR5FALjBTn7bCdP0pAiSbLCY8Xz2Msc3dBb5Ff9GI"
               "SPbUpNmUvBdMZMHQvqOmTNXEPpN0b74MDOMQfWJShOo3NkAvMjs"_s;

    QTest::newRow("int_and_utf8_string2")
            << "082a128404328104595652664a766a78716267764677533159764f5a5867746a356666474c5337"
               "41694e487a396f5a496f4b626d377a3848373978427579506b70515876476f4f30394f59397852"
               "61777833654f417339786a6f544131784a68727732385441637131436562596c43395755665143"
               "366849616e74614e647948694b546f666669305a74376c613432535278585a5350344775786263"
               "5a49703533704a6e79437766437931716446637a5430646d6e3768386670794164656d45617677"
               "4665646134643050417047665355326a4c74333958386b595542784e4d325767414c5242674864"
               "56646538377136506935553639546a684d6432385731534644314478796f67434372714f637432"
               "5a5049436f4c6e72716446334f644e7a6a52564c6665797651384c674c76524e46523957665741"
               "79417a37396e4b6742616d64384e746c7674344d6733354535675653326737415137726b6d3732"
               "6342646e5739734345794761626558417548356a34475262754c543771425a574463464c463453"
               "734364533357664647644e48667761696a7a796b42796f3731507646566c54584832574a576f46"
               "76523546414c6a42546e37624364503070416953624c435938587a324d73633364426235466639"
               "474953506255704e6d557642644d5a4d485176714f6d544e584550704e306237344d444f4d5166"
               "574a53684f6f334e6b41764d6a73"_ba
            << 42
            << u"YVRfJvjxqbgvFwS1YvOZXgtj5ffGLS7AiNHz9oZIoKbm7z8H79xBuyPkpQXvGoO09OY9xRawx3eOA"
               "s9xjoTA1xJhrw28TAcq1CebYlC9WUfQC6hIantaNdyHiKToffi0Zt7la42SRxXZSP4GuxbcZIp53p"
               "JnyCwfCy1qdFczT0dmn7h8fpyAdemEavwFeda4d0PApGfSU2jLt39X8kYUBxNM2WgALRBgHdVde87"
               "q6Pi5U69TjhMd28W1SFD1DxyogCCrqOct2ZPICoLnrqdF3OdNzjRVLfeyvQ8LgLvRNFR9WfWAyAz7"
               "9nKgBamd8Ntlvt4Mg35E5gVS2g7AQ7rkm72cBdnW9sCEyGabeXAuH5j4GRbuLT7qBZWDcFLF4SsCd"
               "S3WfFGdNHfwaijzykByo71PvFVlTXH2WJWoFvR5FALjBTn7bCdP0pAiSbLCY8Xz2Msc3dBb5Ff9GI"
               "SPbUpNmUvBdMZMHQvqOmTNXEPpN0b74MDOMQfWJShOo3NkAvMjs"_s;
}

void QtProtobufTypesDeserializationTest::complexTypeDeserializeTest()
{
    QFETCH(const QByteArray, hexData);
    QFETCH(const int32_t, intField);
    QFETCH(const QString, stringField);

    SimpleStringMessage msg;
    msg.setTestFieldString("Pre-set message value");
    ComplexMessage test;
    test.setTestComplexField(msg);
    ComplexMessage test2 = test;

    test.deserialize(serializer.get(), QByteArray::fromHex(hexData));
    QCOMPARE(test.testFieldInt(), intField);
    QCOMPARE(test.testComplexField().testFieldString(), stringField);

    QCOMPARE_NE(test, test2);
    QCOMPARE(test2.testComplexField(), msg);
}

void QtProtobufTypesDeserializationTest::sIntMessageDeserializeTest_data()
{
    generateTestData<int32_t, IntTypes::SInt>();
}

void QtProtobufTypesDeserializationTest::sIntMessageDeserializeTest()
{
    QFETCH(const QByteArray, hexData);
    QFETCH(const int32_t, expectedValue);

    SimpleSIntMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex(hexData));
    QCOMPARE(test.testFieldInt(), expectedValue);
}

void QtProtobufTypesDeserializationTest::uIntMessageDeserializeTest_data()
{
    generateTestData<uint32_t, IntTypes::UInt>();
}

void QtProtobufTypesDeserializationTest::uIntMessageDeserializeTest()
{
    QFETCH(const QByteArray, hexData);
    QFETCH(const uint32_t, expectedValue);

    SimpleUIntMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex(hexData));
    QCOMPARE(test.testFieldInt(), expectedValue);
}

void QtProtobufTypesDeserializationTest::boolDeserializeTest_data()
{
    QTest::addColumn<QByteArray>("hexData");
    QTest::addColumn<bool>("expectedValue");

    QTest::newRow("empty") << ""_ba << false;
    QTest::newRow("True") << "0801"_ba << true;
    QTest::newRow("False") << "0800"_ba << false;
}

void QtProtobufTypesDeserializationTest::boolDeserializeTest()
{
    QFETCH(const QByteArray, hexData);
    QFETCH(const bool, expectedValue);

    SimpleBoolMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex(hexData));
    QCOMPARE(test.testFieldBool(), expectedValue);
}

void QtProtobufTypesDeserializationTest::redundantFieldIsIgnoredAtDeserializationTest_data()
{
    QTest::addColumn<QByteArray>("hexData");
    QTest::addColumn<QString>("expectedValue");

    //3206717765727479 length delimited field number 6
    QTest::newRow("field_6") << "120832067177657274793206717765727479"_ba << u"qwerty"_s;
    //3dcdcccc3d fixed32 field number 7
    QTest::newRow("field_7") << "120832067177657274793dcdcccc3d"_ba << u"qwerty"_s;
    //419a9999999999b93f fixed64 field number 8
    QTest::newRow("field_8") << "12083206717765727479419a9999999999b93f"_ba << u"qwerty"_s;
    //60d3ffffffffffffffff01 varint field number 12
    QTest::newRow("field_12") << "60d3ffffffffffffffff0112083206717765727479"_ba << u"qwerty"_s;
}

void QtProtobufTypesDeserializationTest::redundantFieldIsIgnoredAtDeserializationTest()
{
    QFETCH(const QByteArray, hexData);
    QFETCH(const QString, expectedValue);

    ComplexMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex(hexData));
    QCOMPARE(test.testFieldInt(), 0);
    QCOMPARE(test.testComplexField().testFieldString(), expectedValue);
}

void QtProtobufTypesDeserializationTest::fieldIndexRangeTest()
{
    FieldIndexTest1Message msg1;
    msg1.deserialize(serializer.get(), QByteArray::fromHex("f80102"));
    QCOMPARE(msg1.testField(), 1);

    FieldIndexTest2Message msg2;
    msg2.deserialize(serializer.get(), QByteArray::fromHex("f8ff0302"));
    QCOMPARE(msg2.testField(), 1);

    FieldIndexTest3Message msg3;
    msg3.deserialize(serializer.get(), QByteArray::fromHex("f8ffff0702"));
    QCOMPARE(msg3.testField(), 1);

    FieldIndexTest4Message msg4;
    msg4.deserialize(serializer.get(), QByteArray::fromHex("f8ffffff0f02"));
    QCOMPARE(msg4.testField(), 1);
}

void QtProtobufTypesDeserializationTest::oneofMessageTest()
{
    ComplexMessage complexField;
    SimpleStringMessage stringField;
    stringField.setTestFieldString("qwerty");
    complexField.setTestFieldInt(42);
    complexField.setTestComplexField(stringField);

    OneofMessage test;

    test.deserialize(serializer.get(),
                     QByteArray::fromHex("08d3ffffffffffffffff01"
                                         "1a0c082a12083206717765727479"));
    QVERIFY(test.hasTestOneofComplexField());
    QVERIFY(!test.hasTestOneofFieldInt());
    QCOMPARE(test.testOneofComplexField(), complexField);
    QCOMPARE(test.testFieldInt(), -45);

    test.deserialize(serializer.get(),
                     QByteArray::fromHex("08d3ffffffffffffffff01"
                                         "d002d3ffffffffffffffff01"));
    QVERIFY(!test.hasTestOneofComplexField());
    QVERIFY(test.hasTestOneofFieldInt());
    QCOMPARE(test.testOneofFieldInt(), -45);
    QCOMPARE(test.testFieldInt(), -45);

    test.deserialize(serializer.get(), QByteArray::fromHex("08d3ffffffffffffffff01"));
    QVERIFY(!test.hasTestOneofComplexField());
    QVERIFY(!test.hasTestOneofFieldInt());
    QCOMPARE(test.testFieldInt(), -45);

    test.setTestFieldInt(-45);
    test.setTestOneofFieldInt(-42);
    test.deserialize(serializer.get(), QByteArray::fromHex(""));
    QVERIFY(!test.hasTestOneofComplexField());
    QVERIFY(!test.hasTestOneofFieldInt());
    QCOMPARE(test.testFieldInt(), 0);
}

void QtProtobufTypesDeserializationTest::oneofMessageEmptyTest()
{
    ComplexMessage complexField;
    SimpleStringMessage stringField;
    stringField.setTestFieldString("qwerty");
    complexField.setTestFieldInt(42);
    complexField.setTestComplexField(stringField);

    OneofMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex("08d3ffffffffffffffff01"));
    QVERIFY(!test.hasTestOneofComplexField());
    QVERIFY(!test.hasTestOneofFieldInt());
    QVERIFY(!test.hasSecondComplexField());
    QVERIFY(!test.hasSecondFieldInt());
    QCOMPARE(test.testFieldInt(), -45);

    test.setTestFieldInt(-45);
    test.setTestOneofFieldInt(-42);
    test.deserialize(serializer.get(), QByteArray::fromHex(""));
    QVERIFY(!test.hasTestOneofComplexField());
    QVERIFY(!test.hasTestOneofFieldInt());
    QVERIFY(!test.hasSecondComplexField());
    QVERIFY(!test.hasSecondFieldInt());
    QCOMPARE(test.testFieldInt(), 0);
}

void QtProtobufTypesDeserializationTest::oneofMessageMultipleFieldsTest()
{
    ComplexMessage complexField;
    SimpleStringMessage stringField;
    stringField.setTestFieldString("qwerty");
    complexField.setTestFieldInt(42);
    complexField.setTestComplexField(stringField);

    OneofMessage test;

    // Message BLOB contains more than one oneof field. So deserializer should set only the last
    // one.
    test.deserialize(serializer.get(),
                     QByteArray::fromHex("08d3ffffffffffffffff01"
                                         "d002d3ffffffffffffffff01"
                                         "1a0c082a12083206717765727479"));
    QVERIFY(test.hasTestOneofComplexField());
    QVERIFY(!test.hasSecondComplexField());
    QVERIFY(!test.hasTestOneofFieldInt());
    QVERIFY(!test.hasSecondFieldInt());
    QCOMPARE(test.testOneofComplexField(), complexField);
    QCOMPARE(test.testFieldInt(), -45);

    // Check the empty initialized 'oneof' fileds
    test.deserialize(serializer.get(), QByteArray::fromHex("1a002a00"));
    QVERIFY(test.hasTestOneofComplexField());
    QVERIFY(test.hasSecondComplexField());
    QVERIFY(!test.hasTestOneofFieldInt());
    QVERIFY(!test.hasSecondFieldInt());
    QCOMPARE(test.testOneofComplexField(), ComplexMessage());
    QCOMPARE(test.secondComplexField(), ComplexMessage());
}

QTEST_MAIN(QtProtobufTypesDeserializationTest)
#include "tst_protobuf_deserialization_basictypes.moc"
