// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QtTest>

#include <QtPositioning/qgeoaddress.h>

QT_USE_NAMESPACE

class tst_QGeoAddress : public QObject
{
    Q_OBJECT

public:
    tst_QGeoAddress();

private Q_SLOTS:
    void constructorTest();
    void moveConstructTest();
    void moveAssignTest();
    void textTest();
    void cityTest();
    void countryCodeTest();
    void countryTest();
    void countyTest();
    void districtTest();
    void postalCodeTest();
    void stateTest();
    void streetTest();
    void streetNumberTest();
    void generatedText();
    void generatedText_data();
    void operatorsTest();
    void emptyClearTest();
    void hashingTest();
    void hashingTest_data();
};

tst_QGeoAddress::tst_QGeoAddress()
{
}

void tst_QGeoAddress::constructorTest()
{
    QGeoAddress testObj;

    testObj.setStreet("testId");
    auto testObjPtr = std::make_unique<QGeoAddress>(testObj);
    QVERIFY2(testObjPtr != NULL, "Copy constructor - null");
    QVERIFY2(*testObjPtr == testObj, "Copy constructor - compare");
}

void tst_QGeoAddress::moveConstructTest()
{
    QGeoAddress address;
    address.setCountry("country");
    address.setCity("city");
    address.setPostalCode("postcode");
    address.setStreet("street");
    address.setStreetNumber("number");

    const QGeoAddress addressCopy = address;
    QCOMPARE(QGeoAddress(std::move(address)), addressCopy);
}

void tst_QGeoAddress::moveAssignTest()
{
    QGeoAddress address;
    address.setCountry("country");
    address.setCity("city");
    address.setPostalCode("postcode");
    address.setStreet("street");
    address.setStreetNumber("number");

    QGeoAddress addressCopy = address;

    QGeoAddress otherAddress;
    otherAddress = std::move(address);
    QCOMPARE(otherAddress, addressCopy);

    // Check that (move)assigning to a moved-from object is fine
    address = std::move(addressCopy);
    QCOMPARE(address, otherAddress);
}

void tst_QGeoAddress::textTest()
{
    QGeoAddress address;
    QVERIFY(address.text().isEmpty());
    address.setText(QStringLiteral("123 Fake Street\nSpringfield"));
    QCOMPARE(address.text(), QStringLiteral("123 Fake Street\nSpringfield"));
}

void tst_QGeoAddress::cityTest()
{
    QGeoAddress testObj;
    QVERIFY2(testObj.city() == QString(), "Wrong default value");
    testObj.setCity("testText");
    QVERIFY2(testObj.city() == "testText", "Wrong value returned");
}

void tst_QGeoAddress::countryCodeTest()
{
    QGeoAddress testObj;
    QVERIFY2(testObj.countryCode() == QString(), "Wrong default value");
    testObj.setCountryCode("testText");
    QVERIFY2(testObj.countryCode() == "testText", "Wrong value returned");
}

void tst_QGeoAddress::countryTest()
{
    QGeoAddress testObj;
    QVERIFY2(testObj.country() == QString(), "Wrong default value");
    testObj.setCountry("testText");
    QVERIFY2(testObj.country() == "testText", "Wrong value returned");
}

void tst_QGeoAddress::countyTest()
{
    QGeoAddress testObj;
    QVERIFY2(testObj.county() == QString(), "Wrong default value");
    testObj.setCounty("testText");
    QVERIFY2(testObj.county() == "testText", "Wrong value returned");
}

void tst_QGeoAddress::districtTest()
{
    QGeoAddress testObj;
    QVERIFY2(testObj.district() == QString(), "Wrong default value");
    testObj.setDistrict("testText");
    QVERIFY2(testObj.district() == "testText", "Wrong value returned");
}

void tst_QGeoAddress::postalCodeTest()
{
    QGeoAddress testObj;
    QVERIFY2(testObj.postalCode() == QString(), "Wrong default value");
    testObj.setPostalCode("testText");
    QVERIFY2(testObj.postalCode() == "testText", "Wrong value returned");
}

void tst_QGeoAddress::stateTest()
{
    QGeoAddress testObj;
    QVERIFY2(testObj.state() == QString(), "Wrong default value");
    testObj.setState("testText");
    QVERIFY2(testObj.state() == "testText", "Wrong value returned");
}

void tst_QGeoAddress::streetTest()
{
    QGeoAddress testObj;
    QVERIFY2(testObj.street() == QString(), "Wrong default value");
    testObj.setStreet("testText");
    QVERIFY2(testObj.street() == "testText", "Wrong value returned");
}

void tst_QGeoAddress::streetNumberTest()
{
    QGeoAddress testObj;
    QVERIFY2(testObj.streetNumber() == QString(), "Wrong default value");
    const auto streetNumber = QStringLiteral("some street number");
    testObj.setStreetNumber(streetNumber);
    QVERIFY2(testObj.streetNumber() == streetNumber, "Wrong value returned");
}

void tst_QGeoAddress::generatedText()
{
    QFETCH(QString, countryCode);
    QFETCH(QString, expectedPostalCodeOnly);
    QFETCH(QString, expectedFullAddress);

    QGeoAddress streetOnly;
    streetOnly.setStreet("street");
    streetOnly.setCountryCode(countryCode);

    QCOMPARE(streetOnly.text(), QStringLiteral("street"));

    QGeoAddress cityOnly;
    cityOnly.setCity("city");
    cityOnly.setCountryCode(countryCode);
    if (countryCode == QLatin1String("CYM") || countryCode == QLatin1String("IRL"))
        QCOMPARE(cityOnly.text(), QString());
    else
        QCOMPARE(cityOnly.text(), QStringLiteral("city"));

    QGeoAddress postalCodeOnly;
    postalCodeOnly.setPostalCode("postcode");
    postalCodeOnly.setCountryCode(countryCode);
    QCOMPARE(postalCodeOnly.text(), expectedPostalCodeOnly);

    QGeoAddress fullAddress;
    fullAddress.setStreet("street");
    fullAddress.setStreetNumber("number");
    fullAddress.setDistrict("district");
    fullAddress.setPostalCode("postcode");
    fullAddress.setCity("city");
    fullAddress.setState("state");
    fullAddress.setCountry("country");
    fullAddress.setCountryCode(countryCode);

    QCOMPARE(fullAddress.text(), expectedFullAddress);
}

void tst_QGeoAddress::generatedText_data()
{
    QTest::addColumn<QString>("countryCode");
    QTest::addColumn<QString>("expectedPostalCodeOnly");
    QTest::addColumn<QString>("expectedFullAddress");

    QTest::newRow("Albania") << QString::fromLatin1("ALB")
                             << QString::fromLatin1("postcode") /* postal code only */
                             << QString::fromLatin1("street number<br/>" /* full address */
                                                    "postcode, city<br/>"
                                                    "country");

    QTest::newRow("Andorra") << QString::fromLatin1("AND")
                             << QString::fromLatin1("postcode")
                             << QString::fromLatin1("street number<br/>"
                                                    "postcode city<br/>"
                                                    "country");
    QTest::newRow("United Arab Emirates") << QString::fromLatin1("ARE")
                                          << QString()
                                          << QString::fromLatin1("street number<br/>"
                                                                 "district city<br/>"
                                                                 "country");
    QTest::newRow("Australia") << QString::fromLatin1("AUS")
                               << QString::fromLatin1("postcode")
                               << QString::fromLatin1("number street<br/>"
                                                      "district state postcode<br/>"
                                                      "country");
    QTest::newRow("Austria") << QString::fromLatin1("AUT")
                             << QString::fromLatin1("postcode")
                             << QString::fromLatin1("street number<br/>"
                                                    "postcode city<br/>"
                                                    "country");
    QTest::newRow("Bahamas") << QString::fromLatin1("BHS")
                             << QString()
                                     << QString::fromLatin1("street number<br/>"
                                                            "district city<br/>"
                                                            "country");
    QTest::newRow("Bahrain") << QString::fromLatin1("BHR")
                             << QString()
                             << QString::fromLatin1("street number<br/>"
                                                    "district, city, state<br/>"
                                                    "country");
    QTest::newRow("Brazil") << QString::fromLatin1("BRA")
                            << QString::fromLatin1("postcode")
                            << QString::fromLatin1("street number<br/>"
                                                   "district city-state postcode<br/>"
                                                   "country");
    QTest::newRow("Brunei Darussalam") << QString::fromLatin1("BRN")
                                       << QString::fromLatin1("postcode")
                                       << QString::fromLatin1("number street<br/>"
                                                              "district city postcode<br/>"
                                                              "country");
    QTest::newRow("Canada") << QString::fromLatin1("CAN")
                            << QString::fromLatin1("postcode")
                            << QString::fromLatin1("number street<br/>"
                                                   "city, state postcode<br/>"
                                                   "country");
    QTest::newRow("China") << QString::fromLatin1("CHN")
                           << QString::fromLatin1("postcode")
                           << QString::fromLatin1("street number, city<br/>"
                                                  "postcode state<br/>"
                                                  "country");
    QTest::newRow("Chile") << QString::fromLatin1("CHL")
                           << QString::fromLatin1("postcode")
                           << QString::fromLatin1("street number<br/>"
                                                  "postcode district, city, state<br/>"
                                                  "country");
    QTest::newRow("Cayman Islands") << QString::fromLatin1("CYM")
                                    << QString::fromLatin1("postcode")
                                    << QString::fromLatin1("number street<br/>"
                                                           "state postcode<br/>"
                                                           "country");
    QTest::newRow("France") << QString::fromLatin1("FRA")
                            << QString::fromLatin1("postcode")
                            << QString::fromLatin1("number street<br/>"
                                                   "postcode city<br/>"
                                                   "country");

    QTest::newRow("United Kingdom") << QString::fromLatin1("GBR")
                            << QString::fromLatin1("postcode")
                            << QString::fromLatin1("number street<br/>"
                                                   "district, city, postcode<br/>"
                                                   "country");
    QTest::newRow("Gibraltar") << QString::fromLatin1("GIB")
                               << QString()
                               << QString::fromLatin1("number street<br/>"
                                                      "city<br/>"
                                                      "country");
    QTest::newRow("Guadeloupe") << QString::fromLatin1("GLP")
                                << QString::fromLatin1("postcode")
                                << QString::fromLatin1("street number<br/>"
                                                       "postcode city<br/>"
                                                       "country");
    QTest::newRow("French Guiana") << QString::fromLatin1("GUF")
                                   << QString::fromLatin1("postcode")
                                   << QString::fromLatin1("number street<br/>"
                                                          "postcode city<br/>"
                                                          "country");
    QTest::newRow("Hong Kong") << QString::fromLatin1("HKG")
                               << QString()
                               << QString::fromLatin1("number street<br/>"
                                                      "district<br/>"
                                                      "city");
    QTest::newRow("India") << QString::fromLatin1("IND")
                           << QString::fromLatin1("postcode")
                           << QString::fromLatin1("number street<br/>"
                                                  "city postcode state<br/>"
                                                  "country");
    QTest::newRow("Indonesia") << QString::fromLatin1("IDN")
                               << QString::fromLatin1("postcode")
                               << QString::fromLatin1("street number<br/>"
                                                      "city, postcode<br/>"
                                                      "country");
    QTest::newRow("Ireland") << QString::fromLatin1("IRL")
                             << QString()
                             << QString::fromLatin1("number street<br/>"
                                                    "district, state<br/>"
                                                    "country");
    QTest::newRow("Italy") << QString::fromLatin1("ITA")
                           << QString::fromLatin1("postcode")
                           << QString::fromLatin1("street number<br/>"
                                                  "postcode city<br/>"
                                                  "country");
    QTest::newRow("Jersey") << QString::fromLatin1("JEY")
                           << QString::fromLatin1("postcode")
                           << QString::fromLatin1("street number<br/>"
                                                  "city, postcode<br/>"
                                                  "country");
    QTest::newRow("Jordan") << QString::fromLatin1("JOR")
                            << QString::fromLatin1("postcode")
                            << QString::fromLatin1("number street<br/>"
                                                   "district city postcode<br/>"
                                                   "country");
    QTest::newRow("Kuwait") << QString::fromLatin1("KWT")
                            << QString::fromLatin1("postcode")
                            << QString::fromLatin1("street number<br/>"
                                                   "postcode, district, city<br/>"
                                                   "country");
    QTest::newRow("Latvia") << QString::fromLatin1("LVA")
                            << QString::fromLatin1("postcode")
                            << QString::fromLatin1("street number<br/>"
                                                   "city, postcode<br/>"
                                                   "country");
    QTest::newRow("Lebanon") << QString::fromLatin1("LBN")
                            << QString::fromLatin1("postcode")
                            << QString::fromLatin1("number street<br/>"
                                                   "district city postcode<br/>"
                                                   "country");
    QTest::newRow("Luxembourg") << QString::fromLatin1("LUX")
                                << QString::fromLatin1("postcode")
                                << QString::fromLatin1("number street<br/>"
                                                       "postcode city<br/>"
                                                       "country");
    QTest::newRow("Malta") << QString::fromLatin1("MLT")
                           << QString::fromLatin1("postcode")
                           << QString::fromLatin1("number street<br/>"
                                                  "city postcode<br/>"
                                                  "country");
    QTest::newRow("Monaco") << QString::fromLatin1("MCO")
                            << QString::fromLatin1("postcode")
                            << QString::fromLatin1("number street<br/>"
                                                   "postcode city<br/>"
                                                   "country");
    QTest::newRow("Mexico") <<  QString::fromLatin1("MEX")
                            << QString::fromLatin1("postcode")
                            << QString::fromLatin1("street number<br/>"
                                                   "district<br/>"
                                                   "postcode city, state<br/>"
                                                   "country");
    QTest::newRow("Martinique") << QString::fromLatin1("MTQ")
                                << QString::fromLatin1("postcode")
                                << QString::fromLatin1("street number<br/>"
                                                       "postcode, city<br/>"
                                                       "country");
    QTest::newRow("Malaysia") << QString::fromLatin1("MYS")
                                << QString::fromLatin1("postcode")
                                << QString::fromLatin1("number street<br/>"
                                                       "postcode city<br/>"
                                                       "state<br/>"
                                                       "country");
    QTest::newRow("New Zealand") << QString::fromLatin1("NZL")
                                 << QString::fromLatin1("postcode")
                                 << QString::fromLatin1("number street<br/>"
                                                        "district city postcode<br/>"
                                                        "country");
    QTest::newRow("Oman") << QString::fromLatin1("OMN")
                          << QString::fromLatin1("postcode")
                          << QString::fromLatin1("number street<br/>"
                                                 "district, postcode, city, country");
    QTest::newRow("Puerto Rico") << QString::fromLatin1("PRI")
                                 << QString::fromLatin1("postcode")
                                 << QString::fromLatin1("street number<br/>"
                                                        "district, city, state, postcode<br/>"
                                                        "country");
    QTest::newRow("Qatar") << QString::fromLatin1("QAT")
                           << QString()
                           << QString::fromLatin1("street number<br/>"
                                                  "district city, country");
    QTest::newRow("Reunion") << QString::fromLatin1("REU")
                             << QString::fromLatin1("postcode")
                             << QString::fromLatin1("number street<br/>"
                                                    "postcode city<br/>"
                                                    "country");
    QTest::newRow("Russian Federation") << QString::fromLatin1("RUS")
                             << QString::fromLatin1("postcode")
                             << QString::fromLatin1("street number<br/>"
                                                    "postcode city<br/>"
                                                    "country");
    QTest::newRow("Saudi Arabia") << QString::fromLatin1("SAU")
                                  << QString::fromLatin1("postcode")
                                  << QString::fromLatin1("number street district<br/>"
                                                         "city postcode<br/>"
                                                         "country");
    QTest::newRow("Singapore") << QString::fromLatin1("SGP")
                               << QString::fromLatin1("postcode")
                               << QString::fromLatin1("number street<br/>"
                                                      "city postcode<br/>"
                                                      "country");
    QTest::newRow("Marino") << QString::fromLatin1("SMR")
                            << QString::fromLatin1("postcode")
                            << QString::fromLatin1("street number<br/>"
                                                   "postcode city<br/>"
                                                   "country");
    QTest::newRow("Taiwan") << QString::fromLatin1("TWN")
                            << QString()
                            << QString::fromLatin1("street number, district, city<br/>"
                                                   "country");
    QTest::newRow("Thailand") << QString::fromLatin1("THA")
                              << QString("postcode")
                              << QString::fromLatin1("street number<br/>"
                                                     "district, city postcode<br/>"
                                                     "country");
    QTest::newRow("Turkey") <<  QString::fromLatin1("TUR")
                            << QString("postcode")
                            << QString::fromLatin1("street number<br/>"
                                                   "postcode district, city<br/>"
                                                   "country");
    QTest::newRow("Ukraine") << QString::fromLatin1("UKR")
                            << QString::fromLatin1("postcode")
                            << QString::fromLatin1("street number<br/>"
                                                   "city postcode<br/>"
                                                   "country");
    QTest::newRow("United States") << QString::fromLatin1("USA")
                                   << QString::fromLatin1("postcode")
                                   << QString::fromLatin1("number street<br/>"
                                                          "city, state postcode<br/>"
                                                          "country");
    QTest::newRow("Virgin Islands, US") << QString::fromLatin1("VIR")
                                        << QString("postcode")
                                        << QString::fromLatin1("number street<br/>"
                                                               "city, state postcode<br/>"
                                                               "country");
    QTest::newRow("Vatican City State") << QString::fromLatin1("VAT")
                                        << QString::fromLatin1("postcode")
                                        << QString::fromLatin1("street number<br/>"
                                                               "postcode city<br/>"
                                                               "country");
    QTest::newRow("Venezuela") << QString::fromLatin1("VEN")
                               << QString::fromLatin1("postcode")
                               << QString::fromLatin1("street number<br/>"
                                                      "city postcode, state<br/>"
                                                      "country");
    QTest::newRow("South Africa") << QString::fromLatin1("ZAF")
                               << QString()
                               << QString::fromLatin1("street number<br/>"
                                                      "district, city<br/>"
                                                      "country");
    QTest::newRow("Finland") << QString::fromLatin1("FIN")
                             << QString::fromLatin1("postcode")
                             << QString::fromLatin1("street number<br/>"
                                                    "postcode city<br/>"
                                                    "country");
}

void tst_QGeoAddress::operatorsTest()
{
    QGeoAddress testObj;
    testObj.setStreet("testValue");
    QGeoAddress testObj2;
    testObj2 = testObj;
    QVERIFY2(testObj == testObj2, "Not copied correctly");
    testObj2.setCountry("testValue2");
    QVERIFY2(testObj != testObj2, "Object should be different");
}

void tst_QGeoAddress::emptyClearTest()
{
    QGeoAddress testObj;
    QVERIFY(testObj.isEmpty());

    testObj.setCountry(QStringLiteral("country"));
    QVERIFY(!testObj.isEmpty());
    testObj.clear();

    testObj.setCountryCode(QStringLiteral("countryCode"));
    QVERIFY(!testObj.isEmpty());
    testObj.clear();

    testObj.setState(QStringLiteral("state"));
    QVERIFY(!testObj.isEmpty());
    testObj.clear();

    testObj.setCounty(QStringLiteral("county"));
    QVERIFY(!testObj.isEmpty());
    testObj.clear();

    testObj.setCity(QStringLiteral("city"));
    QVERIFY(!testObj.isEmpty());
    testObj.clear();

    testObj.setDistrict(QStringLiteral("district"));
    QVERIFY(!testObj.isEmpty());
    testObj.clear();

    testObj.setPostalCode(QStringLiteral("postalCode"));
    QVERIFY(!testObj.isEmpty());
    testObj.clear();

    testObj.setStreet(QStringLiteral("street"));
    QVERIFY(!testObj.isEmpty());
    testObj.clear();

    testObj.setStreetNumber(QStringLiteral("street number"));
    QVERIFY(!testObj.isEmpty());
    testObj.clear();

    testObj.setText(QStringLiteral("formatted address"));
    QVERIFY(!testObj.isEmpty());
    testObj.clear();

    QVERIFY(testObj.isEmpty());
}

void tst_QGeoAddress::hashingTest()
{
    QFETCH(QGeoAddress, leftAddress);
    QFETCH(QGeoAddress, rightAddress);
    QFETCH(bool, result);

    const size_t leftHash = qHash(leftAddress);
    const size_t rightHash = qHash(rightAddress);
    QCOMPARE(leftHash == rightHash, result);
}

void tst_QGeoAddress::hashingTest_data()
{
    QTest::addColumn<QGeoAddress>("leftAddress");
    QTest::addColumn<QGeoAddress>("rightAddress");
    QTest::addColumn<bool>("result");

    QGeoAddress leftAddress;
    QGeoAddress rightAddress;

    QTest::newRow("empty") << leftAddress << rightAddress << true;
    // country
    leftAddress.setCountry("country");
    QTest::newRow("different country") << leftAddress << rightAddress << false;
    rightAddress.setCountry("country");
    QTest::newRow("same country") << leftAddress << rightAddress << true;
    // country code
    leftAddress.setCountryCode("country code");
    QTest::newRow("different code") << leftAddress << rightAddress << false;
    rightAddress.setCountryCode("country code");
    QTest::newRow("same code") << leftAddress << rightAddress << true;
    // state
    leftAddress.setState("state");
    QTest::newRow("different state") << leftAddress << rightAddress << false;
    rightAddress.setState("state");
    QTest::newRow("same state") << leftAddress << rightAddress << true;
    // county
    leftAddress.setCounty("county");
    QTest::newRow("different county") << leftAddress << rightAddress << false;
    rightAddress.setCounty("county");
    QTest::newRow("same county") << leftAddress << rightAddress << true;
    // city
    leftAddress.setCity("city");
    QTest::newRow("different city") << leftAddress << rightAddress << false;
    rightAddress.setCity("city");
    QTest::newRow("same city") << leftAddress << rightAddress << true;
    // district
    leftAddress.setDistrict("district");
    QTest::newRow("different district") << leftAddress << rightAddress << false;
    rightAddress.setDistrict("district");
    QTest::newRow("same district") << leftAddress << rightAddress << true;
    // street
    leftAddress.setStreet("street");
    QTest::newRow("different street") << leftAddress << rightAddress << false;
    rightAddress.setStreet("street");
    QTest::newRow("same street") << leftAddress << rightAddress << true;
    // street number
    leftAddress.setStreetNumber("number");
    QTest::newRow("different number") << leftAddress << rightAddress << false;
    rightAddress.setStreetNumber("number");
    QTest::newRow("same number") << leftAddress << rightAddress << true;
    // postal code
    leftAddress.setPostalCode("postal code");
    QTest::newRow("different postcode") << leftAddress << rightAddress << false;
    rightAddress.setPostalCode("postal code");
    QTest::newRow("same postcode") << leftAddress << rightAddress << true;
    // custom text
    leftAddress.setText("some custom text");
    QTest::newRow("different custom text") << leftAddress << rightAddress << false;
    rightAddress.setText("some custom text");
    QTest::newRow("same custom text") << leftAddress << rightAddress << true;

    // empty with custom text
    leftAddress.clear();
    leftAddress.setText("some custom text");
    rightAddress.clear();
    QTest::newRow("empty with different custom text") << leftAddress << rightAddress << false;
    rightAddress.setText("some custom text");
    QTest::newRow("empty with same custom text") << leftAddress << rightAddress << true;
}

QTEST_APPLESS_MAIN(tst_QGeoAddress)

#include "tst_qgeoaddress.moc"
