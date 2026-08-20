// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qoaicommonglobal.h"
#include "qoaihttprequest.h"

#include "client1/client/addproptestapi.h"
#include "client1/client/addpropmyenum.h"
#include "client1/client/addpropsignals.h"

#include "client2/client/addprop2testapi.h"
#include "client2/client/addprop2myenum.h"
#include "client2/client/addprop2signals.h"

#include <QtCore/qobject.h>
#include <QtCore/qprocess.h>
#include <QtCore/qtemporaryfile.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;

class tst_GeneratorAdditionalProperties : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void addDownloadProgress();
    void ensureUniqueParamsTest();
    void enumUnknownDefaultCase();
    void allowUnicodeIdentifiers();
    void licenseName();
    void reservedWordPrefix();
    void sortParamsByRequiredFlag();
    void sortModelPropertiesByRequiredFlag();
    void prependFormOrBodyParameters();
    void cleanupTestCase();

private:
    QProcess m_serverProcess;
    QString client1ApiContent;
    QString client2ApiContent;
    const char* warningMsg = "Access manager destroyed while 1 requests were still in progress";
};

static QString readFileContent(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return file.readAll();
}

void tst_GeneratorAdditionalProperties::initTestCase()
{
    if (m_serverProcess.state() != QProcess::ProcessState::Running) {
        m_serverProcess.start(SERVER_PATH);
        if (!m_serverProcess.waitForStarted()) {
            qFatal() << "Couldn't start the server: " << m_serverProcess.errorString();
            exit(EXIT_FAILURE);
        }
        // give the process some time to properly start up the server
        QThread::currentThread()->msleep(1000);
    }
    client1ApiContent = readFileContent(QFINDTESTDATA("client1/client/addproptestapi.h"_L1));
    client2ApiContent = readFileContent(QFINDTESTDATA("client2/client/addprop2testapi.h"_L1));
}

void tst_GeneratorAdditionalProperties::addDownloadProgress()
{
    bool done = false;
    bool progressReceived = false;
    qint64 lastTotal = -1;

    QTemporaryFile tmp;
    QVERIFY2(tmp.open(), "Failed to create temporary file.");
    tmp.write(QByteArray(500 * 1024, 'x'));
    tmp.flush();

    const QString filePath = tmp.fileName();
    QVERIFY(!filePath.isEmpty());
    const QtOpenApiCommon::QOAIHttpFileElement file(filePath);

    AddPropNamespace::AddPropTestApi api;

    connect(&api, &AddPropNamespace::AddPropTestApi::roundtripFinished,
            this, [&](const QtOpenApiCommon::QOAIHttpFileElement &) {
                done = true;
            });

    connect(&api, &AddPropNamespace::AddPropTestApi::roundtripErrorOccurred,
            this, [&](QNetworkReply::NetworkError errType, const QString &errStr) {
                done = false;
                qCritical() << errType << errStr;
            });

    connect(&api, &AddPropNamespace::AddPropTestApi::roundtripProgress,
            this, [&](qint64 bytesReceived, qint64 bytesTotal) {
                progressReceived = true;
                lastTotal = bytesTotal;

                //qDebug() << "bytesReceived =" << bytesReceived << "bytesTotal =" << bytesTotal;
                QVERIFY(bytesReceived > 0);
                QVERIFY(bytesTotal > 0 || bytesTotal == -1); // -1 means unknown size
            });

    api.roundtrip(file);

    QTRY_COMPARE_EQ(done, true);
    QVERIFY(progressReceived);
    QVERIFY(lastTotal > 0);
}

void tst_GeneratorAdditionalProperties::ensureUniqueParamsTest()
{
    // Although the 'testUniqueParams' operation defines two parameters with the same name in the
    // spec, the generator creates unique parameter names because 'ensureUniqueParams' is true.
    AddPropNamespace::AddPropTestApi api;
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    api.testUniqueParams(2, "id2"_L1);
}

void tst_GeneratorAdditionalProperties::enumUnknownDefaultCase()
{
    AddPropNamespace::AddPropTestApi api;
    AddPropNamespace2::AddProp2TestApi api2;
    bool done = false;

    // enumUnknownDefaultCase=false
    api.getEnumValue(this, [&](const QRestReply &reply,
                               const AddPropNamespace::AddPropMyEnum &enumValue) {
        if (!(done = reply.isSuccess())) {
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
            return;
        }
        QCOMPARE(enumValue.getValue(),
                 AddPropNamespace::AddPropMyEnum::eAddPropMyEnum::INVALID_VALUE_OPENAPI_GENERATED);
    });

    QTRY_COMPARE_EQ(done, true);

    done = false;
    // enumUnknownDefaultCase=true
    api2.getEnumValue(this, [&](const QRestReply &reply,
                                const AddPropNamespace2::AddProp2MyEnum &enumValue) {
        if (!(done = reply.isSuccess())) {
            qWarning() << "ERROR: " << reply.errorString() << reply.error();
            return;
        }

        QEXPECT_FAIL("", "QTBUG-144951: enum values unknown to the client always map to "
                         "INVALID_VALUE_OPENAPI_GENERATED even when enumUnknownDefaultCase is "
                         "enabled", Continue);

        QCOMPARE(enumValue.getValue(),
                 AddPropNamespace2::AddProp2MyEnum::eAddProp2MyEnum::UNKNOWN_DEFAULT_OPEN_API);
    });

    QTRY_COMPARE_EQ(done, true);
}

void tst_GeneratorAdditionalProperties::allowUnicodeIdentifiers()
{
    // client1: allowUnicodeIdentifiers=false
    QVERIFY(client1ApiContent.contains("pic"_L1)); // 'pرicЄ' becomes 'pic'
    QVERIFY(!client1ApiContent.contains(u"pرicЄ"_s));
    AddPropNamespace2::AddProp2TestApi api;
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    api.getPrice("example"_L1);

    // client2: allowUnicodeIdentifiers=true
    QVERIFY(client2ApiContent.contains("pic"_L1));
    QVERIFY(!client2ApiContent.contains(u"pرicЄ"_s));
    AddPropNamespace2::AddProp2TestApi api2;
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    api2.getPrice("example2"_L1);

    // Enum values containing unicode characters are always sanitized.
    // regardless of the allowUnicodeIdentifiers generator option value.
    // 'statЄus1Я' from the spec is sanitized to S_AT_US1_.
    QVERIFY(AddPropNamespace::AddPropMyEnum::eAddPropMyEnum::S_AT_US1_ !=
            AddPropNamespace::AddPropMyEnum::eAddPropMyEnum::INVALID_VALUE_OPENAPI_GENERATED);

    // Server URL is never sanitized regardless of allowUnicodeIdentifiers value.
    const auto configs = api.serverConfigurations("getPrice"_L1);
    QVERIFY(!configs.isEmpty());
    QCOMPARE(configs.first().urlTemplate(), u"http://127.0.0.1:10222/Veرsion/v2"_s);
}

void tst_GeneratorAdditionalProperties::licenseName()
{
    QVERIFY(client1ApiContent.contains("MyImaginaryLicense"_L1));

    QVERIFY(client2ApiContent.contains("MyImaginaryLicense"_L1));
    QVERIFY(!client2ApiContent.contains("MyLicense2"_L1));
}

void tst_GeneratorAdditionalProperties::reservedWordPrefix()
{
    // client1: default value of reservedWordPrefix is "r_"
    QVERIFY(client1ApiContent.contains("r_class"_L1));
    QVERIFY(client1ApiContent.contains("r_nullptr"_L1));
    AddPropNamespace::AddPropTestApi api;
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    api.r_class(QString("r_nullptr parameter"));

    AddPropNamespace::AddPropSignals s1;
    s1.setRSlots("slots model property"_L1);
    s1.setRInline("nullptr model property"_L1);
    const QString client1SignalsContent =
            readFileContent(QFINDTESTDATA("client1/client/addpropsignals.h"_L1));
    QVERIFY(client1SignalsContent.contains("r_inline"_L1));
    QVERIFY(client1SignalsContent.contains("r_slots"_L1));

    // client2: reservedWordPrefix=reserved- : This should be sanitized to "reserved_"
    QVERIFY(client2ApiContent.contains("reserved_class"_L1));
    QVERIFY(client2ApiContent.contains("reserved_nullptr"_L1));
    AddPropNamespace2::AddProp2TestApi api2;
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    api2.reserved_class(QString("reserved_nullptr parameter"));

    AddPropNamespace2::AddProp2Signals s2;
    const QString client2SignalsContent =
            readFileContent(QFINDTESTDATA("client2/client/addprop2signals.h"_L1));
    s2.setReservedSlots("slots model property"_L1);
    s2.setReservedInline("nullptr model property"_L1);
    QVERIFY(client2SignalsContent.contains("reserved_inline"_L1));
    QVERIFY(client2SignalsContent.contains("reserved_slots"_L1));
}

void tst_GeneratorAdditionalProperties::sortParamsByRequiredFlag()
{
    const QString req1 = "required argument 1"_L1;
    const QString req2 = "required argument 2"_L1;
    const QtOpenApiCommon::OptionalParameter<QString> opt1("optional argument 1"_L1);

    // sortParamsByRequiredFlag=true
    AddPropNamespace::AddPropTestApi api;
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    api.sortParams(req1, req2, opt1);

    // sortParamsByRequiredFlag=false: ignored
    // Without sorting, an optional parameter with a default value precedes
    // required ones, which is invalid in C++.
    AddPropNamespace2::AddProp2TestApi api2;
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    api2.sortParams(req1, req2, opt1);
}

void tst_GeneratorAdditionalProperties::sortModelPropertiesByRequiredFlag()
{
    const QString clt1SortModelContent = readFileContent("client1/client/addpropsortmodel.h"_L1);
    QVERIFY(clt1SortModelContent.indexOf("m_req1") <  clt1SortModelContent.indexOf("m_req2"));
    QVERIFY(clt1SortModelContent.indexOf("m_req2") < clt1SortModelContent.indexOf("m_opt1"));

    const QString clt2SortModelContent = readFileContent("client2/client/addprop2sortmodel.h"_L1);
    QVERIFY(clt2SortModelContent.indexOf("m_req1") < clt2SortModelContent.indexOf("m_opt1"));
    QVERIFY(clt2SortModelContent.indexOf("m_opt1") < clt2SortModelContent.indexOf("m_req2"));
}

void tst_GeneratorAdditionalProperties::prependFormOrBodyParameters()
{
    QString queryParam("myqueryParam"_L1);

    // prependFormOrBodyParameters=false
    AddPropNamespace::AddPropTestApi api;
    AddPropNamespace::AddPropPrependBodyParam_request myBodyRequest;
    myBodyRequest.setBodyParam("myBodyRequest"_L1);
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    api.prependBodyParam(queryParam, myBodyRequest); // query first, body second

    // prependFormOrBodyParameters=true
    AddPropNamespace2::AddProp2TestApi api2;
    AddPropNamespace2::AddProp2PrependBodyParam_request myBodyRequest2;
    myBodyRequest2.setBodyParam("myBodyRequest2"_L1);
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    api2.prependBodyParam(myBodyRequest2, queryParam); // body first, query second
}

void tst_GeneratorAdditionalProperties::cleanupTestCase()
{
    if (m_serverProcess.state() == QProcess::ProcessState::Running) {
        m_serverProcess.kill();
        m_serverProcess.waitForFinished();
    }
}

QTEST_MAIN(tst_GeneratorAdditionalProperties)
#include "tst_generator_additionalproperties.moc"
