// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/testapi.h"

#include <QtCore/qobject.h>
#include <QtCore/qprocess.h>
#include <QtNetwork/qnetworkrequestfactory.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;

#define CALL_TEST_OPERATION(OPERATION, PARAM, EXPECTED_STRING) \
    CALL_TEST_OPERATION_MULTI_PARAMS(OPERATION, EXPECTED_STRING, PARAM)

#define CALL_TEST_OPERATION_MULTI_PARAMS(OPERATION, EXPECTED_STRING, ...)                   \
{                                                                                           \
    bool done = false;                                                                      \
    OPERATION(__VA_ARGS__, this, [&](const QRestReply &reply, const QString &summary) {     \
        if (!(done = reply.isSuccess()))                                                    \
            qWarning() << "Error happened while issuing request : " << reply.errorString(); \
        QCOMPARE(getStatusString(summary), EXPECTED_STRING);                                \
    });                                                                                     \
    QCOMPARE(m_manager->m_operationPath, EXPECTED_STRING);                                 \
    QTRY_COMPARE_EQ(done, true);                                                            \
}

#define CALL_TEST_POST_OPERATION(OPERATION, PARAM, EXPECTED_STRING)                         \
{                                                                                           \
    bool done = false;                                                                      \
    OPERATION(PARAM, this, [&](const QRestReply &reply, const QString &summary) {           \
        if (!(done = reply.isSuccess()))                                                    \
            qWarning() << "Error happened while issuing request : " << reply.errorString(); \
        QCOMPARE(getStatusString(summary), EXPECTED_STRING);                                \
        QCOMPARE(getHeaderValue(summary), "application/x-www-form-urlencoded");             \
    });                                                                                     \
    QCOMPARE(m_manager->m_operationPath, EXPECTED_STRING);                                 \
    QTRY_COMPARE_EQ(done, true);                                                            \
}

#define CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(OPERATION, PARAM)                  \
{                                                                                      \
    bool done = false;                                                                 \
    OPERATION(PARAM, this, [&](const QRestReply &reply, const QString &summary) {      \
        Q_UNUSED(summary)                                                              \
        done = reply.isSuccess();                                                      \
        QCOMPARE(reply.httpStatus(), 200);                                             \
        QCOMPARE(getHeaderValue(summary), "application/x-www-form-urlencoded");        \
    });                                                                                \
    QTRY_COMPARE_EQ(done, true);                                                       \
}

#define CALL_NOT_FOUND_TEST_OPERATION(OPERATION, PARAM) \
    CALL_FAIL_TEST_OPERATION(OPERATION, PARAM, QString, 404, "404 page not found"_L1)

#define CALL_FAIL_TEST_OPERATION(OPERATION, PARAM, RESPONSE_TYPE, STATUS, ERROR_MESSAGE)\
{                                                                                       \
    bool done = true;                                                                   \
    OPERATION(PARAM, this, [&](const QRestReply &reply, const RESPONSE_TYPE &summary) { \
        Q_UNUSED(summary)                                                               \
        done = reply.isSuccess();                                                       \
        QCOMPARE(reply.httpStatus(), STATUS);                                           \
        QCOMPARE(reply.networkReply()->readAll(), ERROR_MESSAGE);                       \
    });                                                                                 \
    QTRY_COMPARE_EQ(done, false);                                                       \
}

#define CALL_TEST_NUMERIC_OPERATION(OPERATION, PARAM, EXPECTED_SUMMARY)                     \
{                                                                                           \
    bool done = false;                                                                      \
    using RESPONSE_TYPE = decltype(EXPECTED_SUMMARY);                                       \
    OPERATION(PARAM, this, [&](const QRestReply &reply, const RESPONSE_TYPE &summary) {     \
        if (!(done = reply.isSuccess()))                                                    \
            qWarning() << "Error happened while issuing request : " << reply.errorString(); \
        QCOMPARE(summary.getStringValueValue(), EXPECTED_SUMMARY.getStringValueValue());    \
        auto expectedVal = EXPECTED_SUMMARY.getValueValue();                                \
        if (!std::isnan(expectedVal) && !std::isinf(expectedVal))                           \
            QCOMPARE(summary.getValueValue(), expectedVal);                                 \
    });                                                                                     \
    QCOMPARE(m_manager->m_operationPath, EXPECTED_SUMMARY.getStringValueValue());           \
    QTRY_COMPARE_EQ(done, true);                                                            \
}

class LoggingNetworkAccessManager : public QNetworkAccessManager
{
public:
    LoggingNetworkAccessManager(QObject *parent = nullptr)
        : QNetworkAccessManager(parent)
    {}
    ~LoggingNetworkAccessManager() override
    {}

protected:
    QNetworkReply *createRequest(QNetworkAccessManager::Operation op,
                                 const QNetworkRequest &originalReq,
                                 QIODevice *outgoingData = nullptr) override;

public:
    QString m_operationPath;
};

QNetworkReply *LoggingNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op,
                                                          const QNetworkRequest &originalReq,
                                                          QIODevice *outgoingData)
{
    const QUrl fullUrl = originalReq.url();
    // we only need the path and query parameters
    m_operationPath = fullUrl.path(QUrl::FullyEncoded);
    if (fullUrl.hasQuery())
        m_operationPath += u'?' + fullUrl.query(QUrl::FullyEncoded);

    return QNetworkAccessManager::createRequest(op, originalReq, outgoingData);
}

namespace QtOpenAPI {

static QProcess serverProcess;
void startServerProcess()
{
    serverProcess.start(SERVER_PATH);
    if (!serverProcess.waitForStarted()) {
        qFatal() << "Couldn't start the server: " << serverProcess.errorString();
        exit(EXIT_FAILURE);
    }
    // give the process some time to properly start up the server
    QThread::currentThread()->msleep(1000);
}

static QJsonValue getObjectValue(const QString &summary, const QString &key)
{
    QJsonDocument doc = QJsonDocument::fromJson(summary.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        return obj.value(key);
    }
    return QJsonValue();
}

static QString getStatusString(const QString &summary)
{
    const QJsonValue val = getObjectValue(summary, "status"_L1);
    return val.toString();
}

static QString getHeaderValue(const QString &summary,
                              const QString &headerType = "Content-Type"_L1)
{
    const QJsonValue val = getObjectValue(summary, "header"_L1);
    if (!val.isNull()) {
        const QStringList headers = val.toVariant().toMap().value(headerType).toStringList();
        if (headers.size() > 0)
            return headers.at(0);
    }
    return QString();
}

class OperationParameters : public TestApi {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        if (serverProcess.state() != QProcess::ProcessState::Running)
            startServerProcess();

        m_manager = new LoggingNetworkAccessManager(this);
        m_restManager = new QRestAccessManager(m_manager, this);
        setRestAccessManager(m_restManager);
    }
    void pathStringParameters_data();
    void pathStringParameters();
    void pathIntParameters_data();
    void pathIntParameters();
    void pathFloatParameters_data();
    void pathFloatParameters();
    void pathDoubleParameters_data();
    void pathDoubleParameters();
    void pathArrayParameters_data();
    void pathArrayParameters();
    void pathAnyTypeParameters_data();
    void pathAnyTypeParameters();
    void pathObjectParameters_data();
    void pathObjectParameters();
    void pathStringMapParameters_data();
    void pathStringMapParameters();
    void pathModelMapParameters();
    void multiplePathParameters();
    void queryParameters();
    void queryAnyTypeParameters_data();
    void queryAnyTypeParameters();
    void pathNACombinations();
    void queryNACombinations();
    void pathAndQueryUndefined();
    void severalQueryParametersPerOPeration();
    void pathAndQueryParameters();
    void headerAnyTypeParameters_data();
    void headerAnyTypeParameters();
    void headerStringParameters_data();
    void headerStringParameters();
    void headerArrayParameters_data();
    void headerArrayParameters();
    void headerObjectParameters_data();
    void headerObjectParameters();
    void headerMapParameters_data();
    void headerMapParameters();
    void headerAdditionalCases_data();
    void headerAdditionalCases();
    void severalHeaderParameters();
    void headerInvalidStyle();
    void cookieAnyTypeParameters_data();
    void cookieAnyTypeParameters();
    void cookieStringParameters_data();
    void cookieStringParameters();
    void cookieArrayParameters_data();
    void cookieArrayParameters();
    void cookieObjectParameters_data();
    void cookieObjectParameters();
    void cookieMapParameters_data();
    void cookieMapParameters();
    void severalCookies();
    void cookieInvalidStyle();
    void cleanupTestCase();

private:
    LoggingNetworkAccessManager *m_manager = nullptr;
    QRestAccessManager *m_restManager = nullptr;
};

QString invalidExplodeWarningMsg(const QString& paramName, const QString& style, bool explode) {
#ifdef Q_OS_WIN
    return QString("Invalid combination for query parameter '%1': style=%2, explode=%3.\r\n"
                   "Using valid explode=%4 instead.")
#else
    return QString("Invalid combination for query parameter '%1': style=%2, explode=%3.\n"
                   "Using valid explode=%4 instead.")
#endif
        .arg(paramName, style, explode ? "true" : "false", explode ? "false" : "true");
}

// The latest implementation is done based on this information:
// Here is standard 3.1.1 https://spec.openapis.org/oas/v3.1.1.html#style-values
// NOTE! All QUERY and PATH params are encoded.
// https://www.speakeasy.com/openapi/requests/parameters/path-parameters#how-to-override-path-parameter-encoding
// and https://www.speakeasy.com/openapi/requests/parameters/query-parameters
void OperationParameters::pathStringParameters_data()
{
    QTest::addColumn<QString>("stringValue");
    QTest::addColumn<QString>("expectedSimpleExplode");
    QTest::addColumn<QString>("expectedSimpleNotExplode");
    QTest::addColumn<QString>("expectedLabelExplode");
    QTest::addColumn<QString>("expectedLabelNotExplode");
    QTest::addColumn<QString>("expectedMatrixExplode");
    QTest::addColumn<QString>("expectedMatrixNotExplode");
    QTest::newRow("QString(Hello World!)") << QString("Hello World!")
                                           << "/v2/path/string/simple-explode/Hello%20World%21"
                                           << "/v2/path/string/simple-not-explode/Hello%20World%21"
                                           << "/v2/path/string/label-explode/.Hello%20World%21"
                                           << "/v2/path/string/label-not-explode/.Hello%20World%21"
                                           << "/v2/path/string/matrix-explode/;stringParameter=Hello%20World%21"
                                           << "/v2/path/string/matrix-not-explode/;stringParameter=Hello%20World%21";

    QTest::newRow("QString(QwerTy12399.)") << QString("QwerTy12399.")
                                           << "/v2/path/string/simple-explode/QwerTy12399."
                                           << "/v2/path/string/simple-not-explode/QwerTy12399."
                                           << "/v2/path/string/label-explode/.QwerTy12399."
                                           << "/v2/path/string/label-not-explode/.QwerTy12399."
                                           << "/v2/path/string/matrix-explode/;stringParameter=QwerTy12399."
                                           << "/v2/path/string/matrix-not-explode/;stringParameter=QwerTy12399.";
    // test sub-delims: *+,;=!$&'()
    QTest::newRow("QString(sub-delims: *+,;=!$&'())") << QString("*+,;=!$&'()")
                                                      << "/v2/path/string/simple-explode/%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                      << "/v2/path/string/simple-not-explode/%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                      << "/v2/path/string/label-explode/.%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                      << "/v2/path/string/label-not-explode/.%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                      << "/v2/path/string/matrix-explode/;stringParameter=%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                      << "/v2/path/string/matrix-not-explode/;stringParameter=%2A%2B%2C%3B%3D%21%24%26%27%28%29";

}

void OperationParameters::pathStringParameters()
{
    QFETCH(QString, stringValue);
    QFETCH(QString, expectedSimpleExplode);
    QFETCH(QString, expectedSimpleNotExplode);
    QFETCH(QString, expectedLabelExplode);
    QFETCH(QString, expectedLabelNotExplode);
    QFETCH(QString, expectedMatrixExplode);
    QFETCH(QString, expectedMatrixNotExplode);

    // style=simple, explode=true, type=string
    CALL_TEST_OPERATION(simpleExplodeString, stringValue, expectedSimpleExplode);

    // style=simple, explode=false, type=string
    CALL_TEST_OPERATION(simpleNotExplodeString, stringValue, expectedSimpleNotExplode);

    // style=label, explode=true, type=string
    CALL_TEST_OPERATION(labelExplodeString, stringValue, expectedLabelExplode);

    // style=label, explode=false, type=string
    CALL_TEST_OPERATION(labelNotExplodeString, stringValue, expectedLabelNotExplode);

    // style=matrix, explode=true, type=string
    CALL_TEST_OPERATION(matrixExplodeString, stringValue, expectedMatrixExplode);

    // style=matrix, explode=false, type=string
    CALL_TEST_OPERATION(matrixNotExplodeString, stringValue, expectedMatrixNotExplode);
}

void OperationParameters::pathIntParameters_data()
{
    QTest::addColumn<qint64>("intValue");
    QTest::addColumn<QString>("expectedSimpleExplode");
    QTest::addColumn<QString>("expectedSimpleNotExplode");
    QTest::addColumn<QString>("expectedLabelExplode");
    QTest::addColumn<QString>("expectedLabelNotExplode");
    QTest::addColumn<QString>("expectedMatrixExplode");
    QTest::addColumn<QString>("expectedMatrixNotExplode");

    const std::array<qint64, 3> values = {22, std::numeric_limits<qint64>::max(),
                                          std::numeric_limits<qint64>::min()};
    for (auto int_val : values) {
        const QString strVal = QString::number(int_val);
        QTest::addRow("int %s", strVal.toLatin1().constData())
            << int_val
            << "/v2/path/int/simple-explode/"_L1 + strVal
            << "/v2/path/int/simple-not-explode/"_L1 + strVal
            << "/v2/path/int/label-explode/."_L1 + strVal
            << "/v2/path/int/label-not-explode/."_L1 + strVal
            << "/v2/path/int/matrix-explode/;intParameter="_L1 + strVal
            << "/v2/path/int/matrix-not-explode/;intParameter="_L1 + strVal;
    }
}

void OperationParameters::pathIntParameters()
{
    QFETCH(qint64, intValue);
    QFETCH(QString, expectedSimpleExplode);
    QFETCH(QString, expectedSimpleNotExplode);
    QFETCH(QString, expectedLabelExplode);
    QFETCH(QString, expectedLabelNotExplode);
    QFETCH(QString, expectedMatrixExplode);
    QFETCH(QString, expectedMatrixNotExplode);

    // style=simple, explode=true, type=integer, format=int64
    CALL_TEST_OPERATION(simpleExplodeInt, intValue, expectedSimpleExplode);

    // style=simple, explode=false, type=integer, format=int64
    CALL_TEST_OPERATION(simpleNotExplodeInt, intValue, expectedSimpleNotExplode);

    // style=label, explode=true, type=integer, format=int64
    CALL_TEST_OPERATION(labelExplodeInt, intValue, expectedLabelExplode);

    // style=label, explode=false, type=integer, format=int64
    CALL_TEST_OPERATION(labelNotExplodeInt, intValue, expectedLabelNotExplode);

    // style=matrix, explode=true, type=integer, format=int64
    CALL_TEST_OPERATION(matrixExplodeInt, intValue, expectedMatrixExplode);

    // style=matrix, explode=false, type=integer, format=int64
    CALL_TEST_OPERATION(matrixNotExplodeInt, intValue, expectedMatrixNotExplode);
}

void OperationParameters::pathFloatParameters_data()
{
    QTest::addColumn<float>("floatValue");
    QTest::addColumn<QString>("expectedSimpleExplode");
    QTest::addColumn<QString>("expectedSimpleNotExplode");
    QTest::addColumn<QString>("expectedLabelExplode");
    QTest::addColumn<QString>("expectedLabelNotExplode");
    QTest::addColumn<QString>("expectedMatrixExplode");
    QTest::addColumn<QString>("expectedMatrixNotExplode");

    const std::array<float, 5> values = {22.012345f, 22.01236f,
                                         std::numeric_limits<float>::quiet_NaN(),
                                         std::numeric_limits<float>::infinity(),
                                         -std::numeric_limits<float>::infinity()};
    for (auto f_val : values) {
        const QString strVal = QString::number(f_val, 'g', QLocale::FloatingPointShortest);
        QTest::addRow("float %s", strVal.toLatin1().constData())
            << f_val
            << "/v2/path/float/simple-explode/"_L1 + strVal
            << "/v2/path/float/simple-not-explode/"_L1 + strVal
            << "/v2/path/float/label-explode/."_L1 + strVal
            << "/v2/path/float/label-not-explode/."_L1 + strVal
            << "/v2/path/float/matrix-explode/;floatParameter="_L1 + strVal
            << "/v2/path/float/matrix-not-explode/;floatParameter="_L1 + strVal;
    }
}

void OperationParameters::pathFloatParameters()
{
    QFETCH(float, floatValue);
    QFETCH(QString, expectedSimpleExplode);
    QFETCH(QString, expectedSimpleNotExplode);
    QFETCH(QString, expectedLabelExplode);
    QFETCH(QString, expectedLabelNotExplode);
    QFETCH(QString, expectedMatrixExplode);
    QFETCH(QString, expectedMatrixNotExplode);

    FloatResponse res;
    res.setValue(floatValue);

    // style=simple, explode=true, type=number, format=float
    res.setStringValue(expectedSimpleExplode);
    CALL_TEST_NUMERIC_OPERATION(simpleExplodeFloat, floatValue, res);

    // style=simple, explode=false, type=number, format=float
    res.setStringValue(expectedSimpleNotExplode);
    CALL_TEST_NUMERIC_OPERATION(simpleNotExplodeFloat, floatValue, res);

    // style=label, explode=true, type=number, format=float
    res.setStringValue(expectedLabelExplode);
    CALL_TEST_NUMERIC_OPERATION(labelExplodeFloat, floatValue, res);

    // style=label, explode=false, type=number, format=float
    res.setStringValue(expectedLabelNotExplode);
    CALL_TEST_NUMERIC_OPERATION(labelNotExplodeFloat, floatValue, res);

    // style=matrix, explode=true, type=number, format=float
    res.setStringValue(expectedMatrixExplode);
    CALL_TEST_NUMERIC_OPERATION(matrixExplodeFloat, floatValue, res);

    // style=matrix, explode=false, type=number, format=float
    res.setStringValue(expectedMatrixNotExplode);
    CALL_TEST_NUMERIC_OPERATION(matrixNotExplodeFloat, floatValue, res);
}

void OperationParameters::pathDoubleParameters_data()
{
    QTest::addColumn<double>("doubleValue");
    QTest::addColumn<QString>("expectedSimpleExplode");
    QTest::addColumn<QString>("expectedSimpleNotExplode");
    QTest::addColumn<QString>("expectedLabelExplode");
    QTest::addColumn<QString>("expectedLabelNotExplode");
    QTest::addColumn<QString>("expectedMatrixExplode");
    QTest::addColumn<QString>("expectedMatrixNotExplode");

    const std::array<double, 5> values = {22.012345, 2.987653212346578627, 2.987653212346578627e9,
                                          std::numeric_limits<double>::max(),
                                          std::numeric_limits<double>::min()};
    for (auto d_val : values) {
        const QString strVal = QUrl::toPercentEncoding(
                            QString::number(d_val, 'g', QLocale::FloatingPointShortest));
        QTest::addRow("double %s", strVal.toLatin1().constData())
            << d_val
            << "/v2/path/double/simple-explode/"_L1 + strVal
            << "/v2/path/double/simple-not-explode/"_L1 + strVal
            << "/v2/path/double/label-explode/."_L1 + strVal
            << "/v2/path/double/label-not-explode/."_L1 + strVal
            << "/v2/path/double/matrix-explode/;doubleParameter="_L1 + strVal
            << "/v2/path/double/matrix-not-explode/;doubleParameter="_L1 + strVal;
    }
}

void OperationParameters::pathDoubleParameters()
{
    QFETCH(double, doubleValue);
    QFETCH(QString, expectedSimpleExplode);
    QFETCH(QString, expectedSimpleNotExplode);
    QFETCH(QString, expectedLabelExplode);
    QFETCH(QString, expectedLabelNotExplode);
    QFETCH(QString, expectedMatrixExplode);
    QFETCH(QString, expectedMatrixNotExplode);

    DoubleResponse res;
    res.setValue(doubleValue);

    // style=simple, explode=true, type=number, format=double
    res.setStringValue(expectedSimpleExplode);
    CALL_TEST_NUMERIC_OPERATION(simpleExplodeDouble, doubleValue,res);

    // style=simple, explode=false, type=number, format=double
    res.setStringValue(expectedSimpleNotExplode);
    CALL_TEST_NUMERIC_OPERATION(simpleNotExplodeDouble, doubleValue,res);

    // style=label, explode=true, type=number, format=double
    res.setStringValue(expectedLabelExplode);
    CALL_TEST_NUMERIC_OPERATION(labelExplodeDouble, doubleValue,res);

    // style=label, explode=false, type=number, format=double
    res.setStringValue(expectedLabelNotExplode);
    CALL_TEST_NUMERIC_OPERATION(labelNotExplodeDouble, doubleValue,res);

    // style=matrix, explode=true, type=number, format=double
    res.setStringValue(expectedMatrixExplode);
    CALL_TEST_NUMERIC_OPERATION(matrixExplodeDouble, doubleValue,res);

    // style=matrix, explode=false, type=number, format=double
    res.setStringValue(expectedMatrixNotExplode);
    CALL_TEST_NUMERIC_OPERATION(matrixNotExplodeDouble, doubleValue,res);
}

void OperationParameters::pathArrayParameters_data()
{
    QTest::addColumn<QList<int>>("arrayIntValues");
    QTest::addColumn<QString>("expectedSimpleExplode");
    QTest::addColumn<QString>("expectedSimpleNotExplode");
    QTest::addColumn<QString>("expectedLabelExplode");
    QTest::addColumn<QString>("expectedLabelNotExplode");
    QTest::addColumn<QString>("expectedMatrixExplode");
    QTest::addColumn<QString>("expectedMatrixNotExplode");
    QTest::newRow("QList<int>({-90, 0, 0, 2, 87867})") << QList<int>({-90, 0, 0, 2, 87867})
                                                       << "/v2/path/array/simple-explode/-90,0,0,2,87867"
                                                       << "/v2/path/array/simple-not-explode/-90,0,0,2,87867"
                                                       << "/v2/path/array/label-explode/.-90.0.0.2.87867"
                                                       << "/v2/path/array/label-not-explode/.-90,0,0,2,87867"
                                                       << "/v2/path/array/matrix-explode/;arrayParameter=-90;arrayParameter=0;arrayParameter=0;arrayParameter=2;arrayParameter=87867"
                                                       << "/v2/path/array/matrix-not-explode/;arrayParameter=-90,0,0,2,87867";
    QTest::newRow("QList<int>({1, 2, -9, 90})") << QList<int>({1, 2, -9, 90})
                                                << "/v2/path/array/simple-explode/1,2,-9,90"
                                                << "/v2/path/array/simple-not-explode/1,2,-9,90"
                                                << "/v2/path/array/label-explode/.1.2.-9.90"
                                                << "/v2/path/array/label-not-explode/.1,2,-9,90"
                                                << "/v2/path/array/matrix-explode/;arrayParameter=1;arrayParameter=2;arrayParameter=-9;arrayParameter=90"
                                                << "/v2/path/array/matrix-not-explode/;arrayParameter=1,2,-9,90";

}

void OperationParameters::pathArrayParameters()
{
    QFETCH(QList<int>, arrayIntValues);
    QFETCH(QString, expectedSimpleExplode);
    QFETCH(QString, expectedSimpleNotExplode);
    QFETCH(QString, expectedLabelExplode);
    QFETCH(QString, expectedLabelNotExplode);
    QFETCH(QString, expectedMatrixExplode);
    QFETCH(QString, expectedMatrixNotExplode);

    // style=simple, explode=true, type=array
    CALL_TEST_OPERATION(simpleExplodeArray, arrayIntValues, expectedSimpleExplode);

    // style=simple, explode=false, type=array
    CALL_TEST_OPERATION(simpleNotExplodeArray, arrayIntValues, expectedSimpleNotExplode);

    // style=label, explode=true, type=array
    CALL_TEST_OPERATION(labelExplodeArray, arrayIntValues, expectedLabelExplode);

    // style=label, explode=false, type=array
    CALL_TEST_OPERATION(labelNotExplodeArray, arrayIntValues, expectedLabelNotExplode);

    // style=matrix, explode=true, type=array
    CALL_TEST_OPERATION(matrixExplodeArray, arrayIntValues, expectedMatrixExplode);

    // style=matrix, explode=false, type=array
    CALL_TEST_OPERATION(matrixNotExplodeArray, arrayIntValues,  expectedMatrixNotExplode);
}

void OperationParameters::pathAnyTypeParameters_data()
{
    // Use "ΣΨ" to test non-ascii characters.
    // They're encoded as 0xCE 0xA3 0xCE 0xA8 in UTF-8, and should be %-encoded
    // similarly.

    TestObject obj;
    obj.setName("Super*+,;=!$&'()Puper");
    obj.setStatus("Awake or Not $ΣΨ");

    QTest::addColumn<QJsonValue>("jsonValue");
    QTest::addColumn<QString>("expectedSimpleExplode");
    QTest::addColumn<QString>("expectedSimpleNotExplode");
    QTest::addColumn<QString>("expectedLabelExplode");
    QTest::addColumn<QString>("expectedLabelNotExplode");
    QTest::addColumn<QString>("expectedMatrixExplode");
    QTest::addColumn<QString>("expectedMatrixNotExplode");
    QTest::newRow("QJsonValue(string)") << QJsonValue("ΣΨ*+,;=!$&'()John Doe")
                                        << "/v2/path/anytype/simple-explode/%CE%A3%CE%A8%2A%2B%2C%3B%3D%21%24%26%27%28%29John%20Doe"
                                        << "/v2/path/anytype/simple-not-explode/%CE%A3%CE%A8%2A%2B%2C%3B%3D%21%24%26%27%28%29John%20Doe"
                                        << "/v2/path/anytype/label-explode/.%CE%A3%CE%A8%2A%2B%2C%3B%3D%21%24%26%27%28%29John%20Doe"
                                        << "/v2/path/anytype/label-not-explode/.%CE%A3%CE%A8%2A%2B%2C%3B%3D%21%24%26%27%28%29John%20Doe"
                                        << "/v2/path/anytype/matrix-explode/;anytypeParameter=%CE%A3%CE%A8%2A%2B%2C%3B%3D%21%24%26%27%28%29John%20Doe"
                                        << "/v2/path/anytype/matrix-not-explode/;anytypeParameter=%CE%A3%CE%A8%2A%2B%2C%3B%3D%21%24%26%27%28%29John%20Doe";
    QTest::newRow("QJsonValue(int)")    << QJsonValue(100)
                                     << "/v2/path/anytype/simple-explode/100"
                                     << "/v2/path/anytype/simple-not-explode/100"
                                     << "/v2/path/anytype/label-explode/.100"
                                     << "/v2/path/anytype/label-not-explode/.100"
                                     << "/v2/path/anytype/matrix-explode/;anytypeParameter=100"
                                     << "/v2/path/anytype/matrix-not-explode/;anytypeParameter=100";
    QTest::newRow("QJsonValue(array)")  << QJsonValue({ 1, 2.2, QString("ΣΨStrange*+,;=!$&'()")})
                                       << "/v2/path/anytype/simple-explode/1,2.2,%CE%A3%CE%A8Strange%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                       << "/v2/path/anytype/simple-not-explode/1,2.2,%CE%A3%CE%A8Strange%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                       << "/v2/path/anytype/label-explode/.1.2.2.%CE%A3%CE%A8Strange%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                       << "/v2/path/anytype/label-not-explode/.1,2.2,%CE%A3%CE%A8Strange%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                       << "/v2/path/anytype/matrix-explode/;anytypeParameter=1;anytypeParameter=2.2;anytypeParameter=%CE%A3%CE%A8Strange%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                       << "/v2/path/anytype/matrix-not-explode/;anytypeParameter=1,2.2,%CE%A3%CE%A8Strange%2A%2B%2C%3B%3D%21%24%26%27%28%29";
    QTest::newRow("QJsonValue(object)") << QJsonValue(obj.asJsonObject())
                                        << "/v2/path/anytype/simple-explode/name=Super%2A%2B%2C%3B%3D%21%24%26%27%28%29Puper,status=Awake%20or%20Not%20%24%CE%A3%CE%A8"
                                        << "/v2/path/anytype/simple-not-explode/name,Super%2A%2B%2C%3B%3D%21%24%26%27%28%29Puper,status,Awake%20or%20Not%20%24%CE%A3%CE%A8"
                                        << "/v2/path/anytype/label-explode/.name=Super%2A%2B%2C%3B%3D%21%24%26%27%28%29Puper.status=Awake%20or%20Not%20%24%CE%A3%CE%A8"
                                        << "/v2/path/anytype/label-not-explode/.name,Super%2A%2B%2C%3B%3D%21%24%26%27%28%29Puper,status,Awake%20or%20Not%20%24%CE%A3%CE%A8"
                                        << "/v2/path/anytype/matrix-explode/;name=Super%2A%2B%2C%3B%3D%21%24%26%27%28%29Puper;status=Awake%20or%20Not%20%24%CE%A3%CE%A8"
                                        << "/v2/path/anytype/matrix-not-explode/;anytypeParameter=name,Super%2A%2B%2C%3B%3D%21%24%26%27%28%29Puper,status,Awake%20or%20Not%20%24%CE%A3%CE%A8";
}

void OperationParameters::pathAnyTypeParameters()
{
    QFETCH(QJsonValue, jsonValue);
    QFETCH(QString, expectedSimpleExplode);
    QFETCH(QString, expectedSimpleNotExplode);
    QFETCH(QString, expectedLabelExplode);
    QFETCH(QString, expectedLabelNotExplode);
    QFETCH(QString, expectedMatrixExplode);
    QFETCH(QString, expectedMatrixNotExplode);

    // style=simple, explode=true, type=AnyType
    CALL_TEST_OPERATION(simpleExplodeAnytype, jsonValue, expectedSimpleExplode);

    // style=simple, explode=false, type=AnyType
    CALL_TEST_OPERATION(simpleNotExplodeAnytype, jsonValue, expectedSimpleNotExplode);

    // style=label, explode=true, type=AnyType
    CALL_TEST_OPERATION(labelExplodeAnytype, jsonValue, expectedLabelExplode);

    // style=label, explode=false, type=AnyType
    CALL_TEST_OPERATION(labelNotExplodeAnytype, jsonValue, expectedLabelNotExplode);

    // style=matrix, explode=true, type=AnyType
    CALL_TEST_OPERATION(matrixExplodeAnytype, jsonValue, expectedMatrixExplode);

    // style=matrix, explode=false, type=AnyType
    CALL_TEST_OPERATION(matrixNotExplodeAnytype, jsonValue, expectedMatrixNotExplode);
}

void OperationParameters::pathObjectParameters_data()
{
    TestObject object;
    object.setName("Igor");
    object.setStatus("Sleep");
    QTest::addColumn<TestObject>("objectValue");
    QTest::addColumn<QString>("expectedSimpleExplode");
    QTest::addColumn<QString>("expectedSimpleNotExplode");
    QTest::addColumn<QString>("expectedLabelExplode");
    QTest::addColumn<QString>("expectedLabelNotExplode");
    QTest::addColumn<QString>("expectedMatrixExplode");
    QTest::addColumn<QString>("expectedMatrixNotExplode");
    QTest::newRow("TestObject({Igor, Sleep})") << object
                                                  << "/v2/path/object/simple-explode/name=Igor,status=Sleep"
                                                  << "/v2/path/object/simple-not-explode/name,Igor,status,Sleep"
                                                  << "/v2/path/object/label-explode/.name=Igor.status=Sleep"
                                                  << "/v2/path/object/label-not-explode/.name,Igor,status,Sleep"
                                                  << "/v2/path/object/matrix-explode/;name=Igor;status=Sleep"
                                                  << "/v2/path/object/matrix-not-explode/;objectParameter=name,Igor,status,Sleep";
    object.setName("TestName123");
    object.setStatus("Maybe-Awake");
    QTest::newRow("TestObject({TestName123, Maybe-Awake})") << object
                                                               << "/v2/path/object/simple-explode/name=TestName123,status=Maybe-Awake"
                                                               << "/v2/path/object/simple-not-explode/name,TestName123,status,Maybe-Awake"
                                                               << "/v2/path/object/label-explode/.name=TestName123.status=Maybe-Awake"
                                                               << "/v2/path/object/label-not-explode/.name,TestName123,status,Maybe-Awake"
                                                               << "/v2/path/object/matrix-explode/;name=TestName123;status=Maybe-Awake"
                                                               << "/v2/path/object/matrix-not-explode/;objectParameter=name,TestName123,status,Maybe-Awake";
    object.setName("SoMeΣΨ");
    object.setStatus(" *+,;=!$&'()");
    QTest::newRow("QOAITestObject({SoMeΣΨ, ' *+,;=!$&'()'})") << object
                                                << "/v2/path/object/simple-explode/name=SoMe%CE%A3%CE%A8,status=%20%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                << "/v2/path/object/simple-not-explode/name,SoMe%CE%A3%CE%A8,status,%20%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                << "/v2/path/object/label-explode/.name=SoMe%CE%A3%CE%A8.status=%20%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                << "/v2/path/object/label-not-explode/.name,SoMe%CE%A3%CE%A8,status,%20%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                << "/v2/path/object/matrix-explode/;name=SoMe%CE%A3%CE%A8;status=%20%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                                << "/v2/path/object/matrix-not-explode/;objectParameter=name,SoMe%CE%A3%CE%A8,status,%20%2A%2B%2C%3B%3D%21%24%26%27%28%29";

}

void OperationParameters::pathObjectParameters()
{
    QFETCH(TestObject, objectValue);
    QFETCH(QString, expectedSimpleExplode);
    QFETCH(QString, expectedSimpleNotExplode);
    QFETCH(QString, expectedLabelExplode);
    QFETCH(QString, expectedLabelNotExplode);
    QFETCH(QString, expectedMatrixExplode);
    QFETCH(QString, expectedMatrixNotExplode);

    // style=simple, explode=true, type=object
    CALL_TEST_OPERATION(simpleExplodeObject, objectValue, expectedSimpleExplode);

    // style=simple, explode=false, type=object
    CALL_TEST_OPERATION(simpleNotExplodeObject, objectValue, expectedSimpleNotExplode);

    // style=label, explode=true, type=object
    CALL_TEST_OPERATION(labelExplodeObject, objectValue, expectedLabelExplode);

    // style=label, explode=false, type=object
    CALL_TEST_OPERATION(labelNotExplodeObject, objectValue, expectedLabelNotExplode);

    // style=matrix, explode=true, type=object
    CALL_TEST_OPERATION(matrixExplodeObject, objectValue, expectedMatrixExplode);

    // style=matrix, explode=false, type=object
    CALL_TEST_OPERATION(matrixNotExplodeObject, objectValue, expectedMatrixNotExplode);
}

void OperationParameters::pathStringMapParameters_data()
{
    QMap<QString, QString> stringMap;
    const QString val1 = "str1 *+,;=!$&'()"_L1;
    stringMap["key1"_L1] = val1;
    const QString key2 = "key2 *+,;=!$&'()"_L1;
    stringMap[key2] = "str2"_L1;

    QTest::addColumn<QMap<QString, QString>>("mapValue");
    QTest::addColumn<QString>("expectedSimpleExplode");
    QTest::addColumn<QString>("expectedSimpleNotExplode");
    QTest::addColumn<QString>("expectedLabelExplode");
    QTest::addColumn<QString>("expectedLabelNotExplode");
    QTest::addColumn<QString>("expectedMatrixExplode");
    QTest::addColumn<QString>("expectedMatrixNotExplode");

    QString urlEncodedVal1 = QUrl::toPercentEncoding(val1);
    QString urlEncodedKey2 = QUrl::toPercentEncoding(key2);

    QTest::newRow("QMap<QString, QString>")
        << stringMap
        << "/v2/path/map/string-mapping/simple-explode/key1="_L1 + urlEncodedVal1 + ","_L1 +
               urlEncodedKey2 + "=str2"_L1
        << "/v2/path/map/string-mapping/simple-not-explode/key1,"_L1 + urlEncodedVal1 + ","_L1 +
               urlEncodedKey2 + ",str2"_L1
        << "/v2/path/map/string-mapping/label-explode/.key1="_L1 + urlEncodedVal1 + "."_L1 +
               urlEncodedKey2 + "=str2"_L1
        << "/v2/path/map/string-mapping/label-not-explode/.key1,"_L1 + urlEncodedVal1 + ","_L1 +
               urlEncodedKey2 + ",str2"_L1
        << "/v2/path/map/string-mapping/matrix-explode/;key1="_L1 + urlEncodedVal1 + ";"_L1 +
               urlEncodedKey2 + "=str2"_L1
        << "/v2/path/map/string-mapping/matrix-not-explode/;mapParameter=key1,"_L1 + urlEncodedVal1
               + ","_L1 + urlEncodedKey2 + ",str2"_L1;

    stringMap.remove(key2);

    stringMap["key1"_L1] = "str1"_L1;
    stringMap["* +key2"_L1] = "str2"_L1;
    urlEncodedKey2 = QUrl::toPercentEncoding("* +key2"_L1);

    // With QMap, the items are always sorted by key. Therefore, we can notice an order change.
    QTest::newRow("QMap<QString, QString> order change")
        << stringMap
        << "/v2/path/map/string-mapping/simple-explode/"_L1 + urlEncodedKey2 + "=str2,key1=str1"_L1
        << "/v2/path/map/string-mapping/simple-not-explode/"_L1 + urlEncodedKey2 +
               ",str2,key1,str1"_L1
        << "/v2/path/map/string-mapping/label-explode/."_L1 + urlEncodedKey2 + "=str2.key1=str1"_L1
        << "/v2/path/map/string-mapping/label-not-explode/."_L1 + urlEncodedKey2
               + ",str2,key1,str1"_L1
        << "/v2/path/map/string-mapping/matrix-explode/;"_L1 + urlEncodedKey2 + "=str2;key1=str1"_L1
        << "/v2/path/map/string-mapping/matrix-not-explode/;mapParameter="_L1 + urlEncodedKey2 +
               ",str2,key1,str1"_L1;

    const QString nonAsciiKey = QStringLiteral("πσ"); // 0xcf 0x80 0xcf 0x83
    const QString nonAsciiValue = QStringLiteral("ΣΨ"); // 0xce 0xa3 0xce 0xa8

    QMap<QString, QString> nonAsciiMap;
    nonAsciiMap[nonAsciiKey] = nonAsciiValue;
    nonAsciiMap[QStringLiteral("key")] = QStringLiteral("value");
    QTest::newRow("non-ascii-map")
            << nonAsciiMap
            << u"/v2/path/map/string-mapping/simple-explode/key=value,%CF%80%CF%83=%CE%A3%CE%A8"_s
            << u"/v2/path/map/string-mapping/simple-not-explode/key,value,%CF%80%CF%83,%CE%A3%CE%A8"_s
            << u"/v2/path/map/string-mapping/label-explode/.key=value.%CF%80%CF%83=%CE%A3%CE%A8"_s
            << u"/v2/path/map/string-mapping/label-not-explode/.key,value,%CF%80%CF%83,%CE%A3%CE%A8"_s
            << u"/v2/path/map/string-mapping/matrix-explode/;key=value;%CF%80%CF%83=%CE%A3%CE%A8"_s
            << u"/v2/path/map/string-mapping/matrix-not-explode/;mapParameter=key,value,%CF%80%CF%83,%CE%A3%CE%A8"_s;
}

void OperationParameters::pathStringMapParameters()
{
    using StringMap = QMap<QString, QString>;
    QFETCH(StringMap, mapValue);
    QFETCH(QString, expectedSimpleExplode);
    QFETCH(QString, expectedSimpleNotExplode);
    QFETCH(QString, expectedLabelExplode);
    QFETCH(QString, expectedLabelNotExplode);
    QFETCH(QString, expectedMatrixExplode);
    QFETCH(QString, expectedMatrixNotExplode);

    // style=simple, explode=true, type=object
    CALL_TEST_OPERATION(simpleExplodeStringMap, mapValue, expectedSimpleExplode);

    // style=simple, explode=false, type=object
    CALL_TEST_OPERATION(simpleNotExplodeStringMap, mapValue, expectedSimpleNotExplode);

    // style=label, explode=true, type=object
    CALL_TEST_OPERATION(labelExplodeStringMap, mapValue, expectedLabelExplode);

    // style=label, explode=false, type=object
    CALL_TEST_OPERATION(labelNotExplodeStringMap, mapValue, expectedLabelNotExplode);

    // style=matrix, explode=true, type=object
    CALL_TEST_OPERATION(matrixExplodeStringMap, mapValue, expectedMatrixExplode);

    // style=matrix, explode=false, type=object
    CALL_TEST_OPERATION(matrixNotExplodeStringMap, mapValue, expectedMatrixNotExplode);
}

// In OpenAPI terminology, a string to model mapping refers to a map with string keys and object values.
void OperationParameters::pathModelMapParameters()
{
    QMap<QString, TestObject> mapValue;
    TestObject obj1;
    obj1.setName("Azer"_L1);
    obj1.setStatus("Ready"_L1);
    mapValue["key1"_L1] = obj1;
    TestObject obj2;
    obj2.setName("Celine"_L1);
    obj2.setStatus("Present"_L1);
    mapValue["key2"_L1] = obj2;

    const char* warningMsg = "Serialization of complex array or object properties in path or query "
                             "parameters is undefined. The generated result will not conform to "
                             "the OpenAPI standard.";

    // style=simple, explode=true, type=map with object values
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(simpleExplodeModelMap, mapValue);

    // style=simple, explode=false, type=map with object values
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(simpleNotExplodeModelMap, mapValue);

    // style=label, explode=true, type=map with object values
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(labelExplodeModelMap, mapValue);

    // style=label, explode=false, type=map with object values
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(labelNotExplodeModelMap, mapValue);

    // style=matrix, explode=true, type=map with object values
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(matrixExplodeModelMap, mapValue);

    // style=matrix, explode=false, type=map with object values
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(matrixNotExplodeModelMap, mapValue);
}

void OperationParameters::multiplePathParameters()
{
    QString str1("First string param");
    QString str2("Second string param");
    QList<int> array {-90, 0, 0, 2, 87867};

    QString expectedResult;
    bool done = false;

    connect(this, &OperationParameters::simpleExplodeStringsFinished,
            this, [&](const QString &summary) {
                done = true;
                QCOMPARE(getStatusString(summary), expectedResult);
            });
    connect(this, &OperationParameters::simpleExplodeStringsErrorOccurred,
            this, [&](QNetworkReply::NetworkError errType, const QString &errStr) {
                done = false;
                qDebug() << errType << errStr;
            });

    expectedResult = "/v2/path/strings/simple-explode/First%20string%20param/"
                     "Second%20string%20param";
    simpleExplodeStrings(str1,str2);
    QCOMPARE(m_manager->m_operationPath, expectedResult);
    QTRY_COMPARE_EQ(done, true);

    done = false;

    connect(this, &OperationParameters::labelStringMatrixArrayNotExplodeFinished,
            this, [&](const QString &summary) {
                done = true;
                QCOMPARE(getStatusString(summary), expectedResult);
            });
    connect(this, &OperationParameters::labelStringMatrixArrayNotExplodeErrorOccurred,
            this, [&](QNetworkReply::NetworkError errType, const QString &errStr) {
                done = false;
                qDebug() << errType << errStr;
            });

    expectedResult = "/v2/path/string/label-not-explode/array/matrix-not-explode/"
                     ".First%20string%20param/;arrayParameter=-90,0,0,2,87867";
    labelStringMatrixArrayNotExplode(str1, array);
    QCOMPARE(m_manager->m_operationPath, expectedResult);
    QTRY_COMPARE_EQ(done, true);
}

// The latest implementation is done based on this information:
// Here is standard 3.1.1 https://spec.openapis.org/oas/v3.1.1.html#style-values
// NOTE! We add data-driven test-cases for queries later with adding new operations in the OP.yaml.
void OperationParameters::queryParameters()
{
    // style=form, explode=true, type=array
    CALL_TEST_POST_OPERATION(formExplodeArray, QList<int>({-90, 0, 0, 2, 87867}),
                             "/v2/query/array/form-explode/formExplodeArray?arrayParameter=-90&arrayParameter=0&arrayParameter=0&arrayParameter=2&arrayParameter=87867");

    // style=form, explode=false, type=array
    CALL_TEST_POST_OPERATION(formNotExplodeArray, QList<int>({1, 2, -9, 90}),
                             "/v2/query/array/form-not-explode/formNotExplodeArray?arrayParameter=1,2,-9,90");

    // Only style=FORM supports primitive types (string, int, double, float)
    // style=form, explode=true, type=string
    CALL_TEST_POST_OPERATION(formExplodeString, OptionalParameter<QString>("hello, guys!"),
                             "/v2/query/string/form-explode/formExplodeString?stringParameter=hello%2C%20guys%21");

    // style=form, explode=false, type=string
    CALL_TEST_POST_OPERATION(formNotExplodeString, OptionalParameter<QString>("hello, guys!"),
                             "/v2/query/string/form-not-explode/formNotExplodeString?stringParameter=hello%2C%20guys%21");

    // style=form, explode=true, type=integer, format=int64
    CALL_TEST_POST_OPERATION(formExplodeInt, OptionalParameter<qint64>(22),
                             "/v2/query/int/form-explode/formExplodeInt?intParameter=22");

    // style=form, explode=false, type=integer, format=int64
    CALL_TEST_POST_OPERATION(formNotExplodeInt, OptionalParameter<qint64>(22),
                             "/v2/query/int/form-not-explode/formNotExplodeInt?intParameter=22");

    // style=form, explode=true, type=number, format=float
    float f_val = 22.0123f;
    FloatResponse f_res;
    f_res.setValue(f_val);
    QString strVal = QString::number(f_val, 'g', QLocale::FloatingPointShortest);
    QString expectedPath = "/v2/query/float/form-explode/formExplodeFloat?floatParameter="_L1 +
                           strVal;
    f_res.setStringValue(expectedPath);
    CALL_TEST_NUMERIC_OPERATION(formExplodeFloat, OptionalParameter<float>(22.0123f),
                               f_res);

    // style=form, explode=false, type=number, format=float
    expectedPath = "/v2/query/float/form-not-explode/formNotExplodeFloat?floatParameter="_L1 +
                   strVal;
    f_res.setStringValue(expectedPath);
    CALL_TEST_NUMERIC_OPERATION(formNotExplodeFloat, OptionalParameter<float>(22.0123f),
                               f_res);

    double d_val = 2.987653212346578627e9;
    strVal = QString::number(d_val, 'g', QLocale::FloatingPointShortest);
    DoubleResponse d_res;
    d_res.setValue(d_val);

    // style=form, explode=true, type=number, format=double
    expectedPath = "/v2/query/double/form-explode/formExplodeDouble?doubleParameter="_L1 + strVal;
    d_res.setStringValue(expectedPath);
    CALL_TEST_NUMERIC_OPERATION(formExplodeDouble, OptionalParameter<double>(d_val), d_res);

    // style=form, explode=false, type=number, format=double
    expectedPath = "/v2/query/double/form-not-explode/formNotExplodeDouble?doubleParameter="_L1 +
                   strVal;
    d_res.setStringValue(expectedPath);
    CALL_TEST_NUMERIC_OPERATION(formNotExplodeDouble, OptionalParameter<double>(d_val),
                                d_res);

    // style=form, explode=true, type=object
    TestObject formObj;
    formObj.setName("TestName+123");
    formObj.setStatus("Awake");
    CALL_TEST_POST_OPERATION(formExplodeObject, formObj,
                             "/v2/query/object/form-explode/formExplodeObject?name=TestName%2B123&status=Awake");

    // style=form, explode=false, type=object
    CALL_TEST_POST_OPERATION(formNotExplodeObject, formObj,
                             "/v2/query/object/form-not-explode/formNotExplodeObject?objectParameter=name,TestName%2B123,status,Awake");

    // style=form, explode=true, type=map with string values
    QMap<QString, QString> formStringMap;
    const QString val1 = "str1 *+,;=!$&'()"_L1;
    const QString key2 = "key2 *+,;=!$&'()"_L1;
    formStringMap["key1"_L1] = val1;
    formStringMap[key2] = "str2"_L1;
    QString urlEncodedVal1 = QUrl::toPercentEncoding(val1);
    QString urlEncodedKey2 = QUrl::toPercentEncoding(key2);
    CALL_TEST_POST_OPERATION(formExplodeStringMap, formStringMap,
                             "/v2/query/map/string-mapping/form-explode/formExplodeMap?"
                             "key1="_L1 + urlEncodedVal1 + "&"_L1 + urlEncodedKey2 + "=str2");

    // style=form, explode=false, type=map with string values
    CALL_TEST_POST_OPERATION(formNotExplodeStringMap, formStringMap,
                             "/v2/query/map/string-mapping/form-not-explode/formNotExplodeMap?"
                             "mapParameter=key1,"_L1 + urlEncodedVal1 + ","_L1 + urlEncodedKey2 + ",str2");

    QMap<QString, TestObject> mapValue;
    TestObject obj1;
    obj1.setName("Azer"_L1);
    obj1.setStatus("Ready"_L1);
    mapValue["key1"_L1] = obj1;
    TestObject obj2;
    obj2.setName("Celine"_L1);
    obj2.setStatus("Present"_L1);
    mapValue["key2"_L1] = obj2;

    const char* warningMsg = "Serialization of complex array or object properties in path or query "
                             "parameters is undefined. The generated result will not conform to "
                             "the OpenAPI standard.";

    // style=form, explode=true, type=map with object values
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(formExplodeModelMap, mapValue);

    // style=form, explode=false, type=map with object values
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(formNotExplodeModelMap, mapValue);

    // style=spaceDelimited, explode=false, type=array
    CALL_TEST_POST_OPERATION(spaceDelimitedNotExplodeArray, QList<int>({1, 2, -9, 90}),
                             "/v2/query/array/spaceDelimited-not-explode/spaceDelimitedNotExplodeArray?arrayParameter=1%202%20-9%2090");

    // style=spaceDelimited, explode=false, type=object
    TestObject spaceDelimitedObj;
    spaceDelimitedObj.setName("TestName 123 *+,;=!$&'()");
    spaceDelimitedObj.setStatus("Awake!");
    CALL_TEST_POST_OPERATION(spaceDelimitedNotExplodeObject, spaceDelimitedObj,
                             "/v2/query/object/spaceDelimited-not-explode/spaceDelimitedNotExplodeObject?objectParameter=name%20TestName%20123%20%2A%2B%2C%3B%3D%21%24%26%27%28%29%20status%20Awake%21");

    // style=spaceDelimited, explode=false, type=map with string values
    QMap<QString, QString> spaceDelimitedStringMap;
    spaceDelimitedStringMap["key1"_L1] = val1;
    spaceDelimitedStringMap[key2] = "str2"_L1;
    CALL_TEST_POST_OPERATION(spaceDelimitedNotExplodeStringMap, spaceDelimitedStringMap,
                             "/v2/query/map/string-mapping/spaceDelimited-not-explode/"
                             "spaceDelimitedNotExplodeMap?mapParameter="
                             "key1%20"_L1 + urlEncodedVal1 + "%20"_L1 + urlEncodedKey2
                                 + "%20str2"_L1);

    // style=spaceDelimited, explode=false, with object values
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(spaceDelimitedNotExplodeModelMap, mapValue);

    // Primitives are not defined for spaceDelimited and pipeDelimited, so anytype=array or anytype-object are not possible
    // style=spaceDelimited, explode=false, type=anytype array
    CALL_TEST_POST_OPERATION(spaceDelimitedNotExplodeAnytype, QJsonValue({ 1, 2.2, QString("Strange *+,;=!$&'()")}),
                             "/v2/query/anytype/spaceDelimited-not-explode/spaceDelimitedNotExplodeAnytype?anytypeParameter=1%202.2%20Strange%20%2A%2B%2C%3B%3D%21%24%26%27%28%29");

    // style=spaceDelimited, explode=false, type=anytype object
    CALL_TEST_POST_OPERATION(spaceDelimitedNotExplodeAnytype, QJsonValue(spaceDelimitedObj.asJsonObject()),
                             "/v2/query/anytype/spaceDelimited-not-explode/spaceDelimitedNotExplodeAnytype?anytypeParameter=name%20TestName%20123%20%2A%2B%2C%3B%3D%21%24%26%27%28%29%20status%20Awake%21");

    // style=pipeDelimited, explode=false, type=array
    CALL_TEST_POST_OPERATION(pipeDelimitedNotExplodeArray, QList<int>({1, 2, -9, 90}),
                             "/v2/query/array/pipeDelimited-not-explode/pipeDelimitedNotExplodeArray?arrayParameter=1%7C2%7C-9%7C90");

    // style=pipeDelimited, explode=false, type=object
    TestObject pipeDelimitedObj;
    pipeDelimitedObj.setName("pipeDelimited=TestName");
    pipeDelimitedObj.setStatus("pipeDelimited-Sleeping *+,;=!$&'()");
    CALL_TEST_POST_OPERATION(pipeDelimitedNotExplodeObject, pipeDelimitedObj,
                             "/v2/query/object/pipeDelimited-not-explode/pipeDelimitedNotExplodeObject?objectParameter=name%7CpipeDelimited%3DTestName%7Cstatus%7CpipeDelimited-Sleeping%20%2A%2B%2C%3B%3D%21%24%26%27%28%29");

    // style=pipeDelimited, explode=false, type=map with string values
    QMap<QString, QString> pipeDelimitedStringMap;
    pipeDelimitedStringMap["key1"_L1] = val1;
    pipeDelimitedStringMap[key2] = "str2"_L1;
    CALL_TEST_POST_OPERATION(pipeDelimitedNotExplodeStringMap, pipeDelimitedStringMap,
                             "/v2/query/map/string-mapping/pipeDelimited-not-explode/"
                             "pipeDelimitedNotExplodeMap?mapParameter="
                             "key1%7C"_L1 + urlEncodedVal1 + "%7C"_L1 + urlEncodedKey2
                                 + "%7Cstr2"_L1);

    // style=pipeDelimited, explode=false, with object values
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(pipeDelimitedNotExplodeModelMap, mapValue);

    // style=pipeDelimited, explode=false, type=anytype array
    CALL_TEST_POST_OPERATION(pipeDelimitedNotExplodeAnytype, QJsonValue({ 1, 2.2, QString("Strange *+,;=!$&'()")}),
                             "/v2/query/anytype/pipeDelimited-not-explode/pipeDelimitedNotExplodeAnytype?anytypeParameter=1%7C2.2%7CStrange%20%2A%2B%2C%3B%3D%21%24%26%27%28%29");

    // style=pipeDelimited, explode=false, type=anytype object
    CALL_TEST_POST_OPERATION(pipeDelimitedNotExplodeAnytype, QJsonValue(pipeDelimitedObj.asJsonObject()),
                             "/v2/query/anytype/pipeDelimited-not-explode/pipeDelimitedNotExplodeAnytype?anytypeParameter=name%7CpipeDelimited%3DTestName%7Cstatus%7CpipeDelimited-Sleeping%20%2A%2B%2C%3B%3D%21%24%26%27%28%29");

    // style=deepObject, explode=true, type=object
    TestObject deepObjectObj;
    deepObjectObj.setName("deepObject *+,;=!$&'()-TestName");
    deepObjectObj.setStatus("deepObject-Sleeping");
    CALL_TEST_POST_OPERATION(deepObjectExplodeObject, deepObjectObj,
                             "/v2/query/object/deepObject-explode/deepObjectExplodeObject?objectParameter%5Bname%5D=deepObject%20%2A%2B%2C%3B%3D%21%24%26%27%28%29-TestName&objectParameter%5Bstatus%5D=deepObject-Sleeping");

    // style=deepObject, explode=true, type=anytype object
    CALL_TEST_POST_OPERATION(deepObjectExplodeAnytype, QJsonValue(deepObjectObj.asJsonObject()),
                        "/v2/query/anytype/deepObject-explode/deepObjectExplodeAnytype?anytypeParameter%5Bname%5D=deepObject%20%2A%2B%2C%3B%3D%21%24%26%27%28%29-TestName&anytypeParameter%5Bstatus%5D=deepObject-Sleeping");

    // style=deepObject, explode=true, type=map with string values
    QMap<QString, QString> deepObjectStringMap;
    deepObjectStringMap["key1"_L1] = val1;
    deepObjectStringMap[key2] = "str2"_L1;
    CALL_TEST_POST_OPERATION(deepObjectExplodeStringMap, deepObjectStringMap,
                             "/v2/query/map/string-mapping/deepObject-explode/deepObjectExplodeMap?"
                             "mapParameter%5Bkey1%5D="_L1 + urlEncodedVal1 + "&"_L1
                                 + "mapParameter%5B"_L1 + urlEncodedKey2
                                 + "%5D=str2"_L1);

    // style=deepObject, explode=false, with object values
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(deepObjectExplodeModelMap, mapValue);
}

void OperationParameters::queryAnyTypeParameters_data()
{
    TestObject obj;
    obj.setName("Super Puper *+,;=!$&'()");
    obj.setStatus("Awake!");

    QTest::addColumn<QJsonValue>("jsonValue");
    QTest::addColumn<QString>("expectedFormExplode");
    QTest::addColumn<QString>("expectedFormNotExplode");
    QTest::newRow("QJsonValue(string)") << QJsonValue("John Doe")
                                        << "/v2/query/anytype/form-explode/formExplodeAnytype?anytypeParameter=John%20Doe"
                                        << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=John%20Doe";
    QTest::newRow("QJsonValue(int)")    << QJsonValue(100)
                                     << "/v2/query/anytype/form-explode/formExplodeAnytype?anytypeParameter=100"
                                     << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=100";
    QTest::newRow("QJsonValue(array)")  << QJsonValue({ 1, 2.2, QString("Strange *+,;=!$&'()") })
                                       << "/v2/query/anytype/form-explode/formExplodeAnytype?anytypeParameter=1&anytypeParameter=2.2&anytypeParameter=Strange%20%2A%2B%2C%3B%3D%21%24%26%27%28%29"
                                       << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=1,2.2,Strange%20%2A%2B%2C%3B%3D%21%24%26%27%28%29";
    QTest::newRow("QJsonValue(object)") << QJsonValue(obj.asJsonObject())
                                        << "/v2/query/anytype/form-explode/formExplodeAnytype?name=Super%20Puper%20%2A%2B%2C%3B%3D%21%24%26%27%28%29&status=Awake%21"
                                        << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=name,Super%20Puper%20%2A%2B%2C%3B%3D%21%24%26%27%28%29,status,Awake%21";
    QTest::newRow("QJsonValue(Null)")   << QJsonValue(QJsonValue::Null)
                                        << "/v2/query/anytype/form-explode/formExplodeAnytype?anytypeParameter="
                                        << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=";
    QTest::newRow("QJsonValue(Undefined)") << QJsonValue(QJsonValue::Undefined)
                                           << "/v2/query/anytype/form-explode/formExplodeAnytype?anytypeParameter="
                                           << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=";
    QTest::newRow("QJsonValue()") << QJsonValue()
                                  << "/v2/query/anytype/form-explode/formExplodeAnytype?anytypeParameter="
                                  << "/v2/query/anytype/form-not-explode/formNotExplodeAnytype?anytypeParameter=";
}

// spaceDelimited, pipeDelimited, deepObject are NOT defined for Primitives.
// It means the standard doesn't say anything about those combinations. So we can skip
// spaceDelimited, pipeDelimited, deepObject cases for AnyType(primitives).
// AnyType(array) is not defined for deepObject.
// See https://spec.openapis.org/oas/v3.1.1.html#style-values
void OperationParameters::queryAnyTypeParameters()
{
    QFETCH(QJsonValue, jsonValue);
    QFETCH(QString, expectedFormExplode);
    QFETCH(QString, expectedFormNotExplode);

    // style=form, explode=true, type=anytype
    CALL_TEST_POST_OPERATION(formExplodeAnytype, jsonValue, expectedFormExplode);
    // style=form, explode=false, type=anytype
    CALL_TEST_POST_OPERATION(formNotExplodeAnytype, jsonValue, expectedFormNotExplode);
}

void OperationParameters::pathNACombinations()
{
    // style=form, explode=true, type=string : Invalid style for path parameters.
    // Falling back to the default style simple instead.
#ifdef Q_OS_WIN
    const char *warningMsg = "'form' style is invalid for path parameters.\r\n"
                             "Allowed styles are: 'matrix', 'label' and 'simple'.\r\n"
                             "Falling back to the default style 'simple'.";
#else
    const char *warningMsg = "'form' style is invalid for path parameters.\nAllowed styles are: "
                             "'matrix', 'label' and 'simple'.\nFalling back to the default style "
                             "'simple'.";
#endif
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    CALL_TEST_OPERATION(invalidFormExplodeString, QString("Test!*%"),
                        "/v2/path/string/invalid-form-explode/formExplodeString/Test%21%2A%25");
}

/**
 * Pay attention, the behavior of spaceDelimited, pipeDelimited and deepObject combinations
 * below are undefined!!!
 * See https://spec.openapis.org/oas/v3.1.1.html#style-values
 * But we decided to generate a warning and to fallback to:
 * - explode=false for spaceDelimited and pipeDelimited cases.
 * - explode=true for deepObject case.
 * - the default query style 'form' in case the style is unsupported.
**/
void OperationParameters::queryNACombinations()
{
    QString warningMsg;

    // style=spaceDelimited, explode=false, type=string
#ifdef Q_OS_WIN
    warningMsg = "'spaceDelimited' style is invalid for primitive parameters.\r\n"
                 "Falling back to the default style: 'form'.";
#else
    warningMsg = "'spaceDelimited' style is invalid for primitive parameters.\n"
                 "Falling back to the default style: 'form'.";
#endif
    QTest::ignoreMessage(QtWarningMsg, warningMsg.toLatin1());
    CALL_TEST_OPERATION(spaceDelimitedNotExplodeString, QString("string * test!"_L1),
                        "/v2/query/string/spaceDelimited-not-explode/spaceDelimitedNotExplodeString?stringParameter=string%20%2A%20test%21");

    // style=spaceDelimited, explode=false, type=anytype string : Invalid combination.
    QTest::ignoreMessage(QtWarningMsg, "Be aware that 'spaceDelimited' style is invalid for "
                                       "primitive parameters. The generated result will not "
                                       "conform to the OpenAPI standard.");
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(spaceDelimitedNotExplodeAnytype,
                                                QJsonValue("Hey*+,;=!$&'( )"));

    // style=spaceDelimited, explode=true, type=array
    warningMsg = invalidExplodeWarningMsg("arrayParameter"_L1 , "spaceDelimited"_L1, true);
    QTest::ignoreMessage(QtWarningMsg, warningMsg.toLatin1());
    CALL_TEST_POST_OPERATION(spaceDelimitedExplodeArray, QList<int>({-90, 0, 0, 2, 87867}),
                             "/v2/query/array/spaceDelimited-explode/spaceDelimitedExplodeArray?arrayParameter=-90%200%200%202%2087867");

    // style=spaceDelimited, explode=true, type=empty array
    QTest::ignoreMessage(QtWarningMsg, warningMsg.toLatin1());
    CALL_TEST_POST_OPERATION(spaceDelimitedExplodeArray, QList<int>(),
                             "/v2/query/array/spaceDelimited-explode/spaceDelimitedExplodeArray?arrayParameter=");

    // style=pipeDelimited, explode=false, type=anytype string : Invalid combination.
    QTest::ignoreMessage(QtWarningMsg, "Be aware that 'pipeDelimited' style is invalid for "
                                       "primitive parameters. The generated result will not "
                                       "conform to the OpenAPI standard.");
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(pipeDelimitedNotExplodeAnytype,
                                                QJsonValue("*+,;=!$&'( Invalid )"));

    // style=pipeDelimited, explode=true, type=array
    warningMsg = invalidExplodeWarningMsg("arrayParameter"_L1 , "pipeDelimited"_L1, true);
    QTest::ignoreMessage(QtWarningMsg, warningMsg.toLatin1());
    CALL_TEST_POST_OPERATION(pipeDelimitedExplodeArray, QList<int>({-90, 0, 0, 2, 87867}),
                             "/v2/query/array/pipeDelimited-explode/pipeDelimitedExplodeArray?arrayParameter=-90%7C0%7C0%7C2%7C87867");

    // style=pipeDelimited, explode=true, type=empty array
    QTest::ignoreMessage(QtWarningMsg, warningMsg.toLatin1());
    CALL_TEST_POST_OPERATION(pipeDelimitedExplodeArray, QList<int>(),
                             "/v2/query/array/pipeDelimited-explode/pipeDelimitedExplodeArray?arrayParameter=");

    // style=spaceDelimited, explode=true, type=object
    TestObject spaceDelimitedObj;
    spaceDelimitedObj.setName("TestName123");
    spaceDelimitedObj.setStatus("Awake");
    warningMsg = invalidExplodeWarningMsg("objectParameter"_L1 , "spaceDelimited"_L1, true);
    QTest::ignoreMessage(QtWarningMsg, warningMsg.toLatin1());
    CALL_TEST_POST_OPERATION(spaceDelimitedExplodeObject, spaceDelimitedObj,
                             "/v2/query/object/spaceDelimited-explode/spaceDelimitedExplodeObject?objectParameter=name%20TestName123%20status%20Awake");

    // style=pipeDelimited, explode=true, type=object
    TestObject pipeDelimitedObj;
    pipeDelimitedObj.setName("pipeDelimited-TestName");
    pipeDelimitedObj.setStatus("pipeDelimited-Sleeping");
    warningMsg = invalidExplodeWarningMsg("objectParameter"_L1 , "pipeDelimited"_L1, true);
    QTest::ignoreMessage(QtWarningMsg, warningMsg.toLatin1());
    CALL_TEST_POST_OPERATION(pipeDelimitedExplodeObject, pipeDelimitedObj,
                             "/v2/query/object/pipeDelimited-explode/pipeDelimitedExplodeObject?objectParameter=name%7CpipeDelimited-TestName%7Cstatus%7CpipeDelimited-Sleeping");

    // style=deepObject, explode=false, type=object
    TestObject deepObjectObj;
    deepObjectObj.setName("deepObject-TestName");
    deepObjectObj.setStatus("deepObject-Sleeping");
    warningMsg = invalidExplodeWarningMsg("objectParameter"_L1 , "deepObject"_L1, false);
    QTest::ignoreMessage(QtWarningMsg, warningMsg.toLatin1());
    CALL_TEST_POST_OPERATION(deepObjectNotExplodeObject, deepObjectObj,
                             "/v2/query/object/deepObject-not-explode/deepObjectNotExplodeObject?objectParameter%5Bname%5D=deepObject-TestName&objectParameter%5Bstatus%5D=deepObject-Sleeping");

    // style=deepObject, explode=false, type=string : Invalid combination.
    // Falling back to the default style form instead.
    warningMsg = "'deepObject' style is only valid for parameters of type 'object'.\n"
                 "Falling back to the default style: 'form'.";
    QTest::ignoreMessage(QtWarningMsg, warningMsg.toLatin1());
    CALL_TEST_POST_OPERATION(invalidDeepObjectNotExplodeString, QString("Test!"),
                             "/v2/query/string/invalid-deepObject-not-explode/deepObjectNotExplodeString?stringParameter=Test%21");

    // style=deepObject, explode=false, type=array : Invalid combination.
    // Falling back to the default style form instead.
    QTest::ignoreMessage(QtWarningMsg, warningMsg.toLatin1());
    CALL_TEST_POST_OPERATION(invalidDeepObjectNotExplodeArray, QList<int>({-90, 0, 0, 2, 87867}),
                        "/v2/query/array/invalid-deepObject-not-explode/deepObjectNotExplodeArray?arrayParameter=-90,0,0,2,87867");

    // style=deepObject, explode=false, type=anytype string : Invalid combination.
    warningMsg = "Be aware that 'deepObject' style is only valid for parameters of type 'object'. "
                 "The generated result will not conform to the OpenAPI standard.";
    QTest::ignoreMessage(QtWarningMsg, warningMsg.toLatin1());
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(deepObjectExplodeAnytype, QJsonValue("*Invalid )"));

    // style=deepObject, explode=false, type=anytype array : Invalid combination.
    QTest::ignoreMessage(QtWarningMsg, warningMsg.toLatin1());
    CALL_POST_NO_EXPECTED_RESULT_TEST_OPERATION(deepObjectExplodeAnytype,
                                                QJsonValue({-90, 0, 0, 2, 87867}));

    // style=matrix, explode=true, type=string : Invalid style for query parameters.
    // Falling back to the default style form instead.
#ifdef Q_OS_WIN
    warningMsg = "'matrix' style is invalid for query parameters.\r\nAllowed styles are: 'form', "
                 "'spaceDelimited', 'pipeDelimited' and 'deepObject'.\r\nFalling back to the "
                 "default style 'form'.";
#else
    warningMsg = "'matrix' style is invalid for query parameters.\nAllowed styles are: 'form', "
                 "'spaceDelimited', 'pipeDelimited' and 'deepObject'.\nFalling back to the default "
                 "style 'form'.";
#endif
    QTest::ignoreMessage(QtWarningMsg, warningMsg.toLatin1());
    CALL_TEST_POST_OPERATION(invalidMatrixExplodeString, QString("Test!"),
                        "/v2/query/string/invalid-matrix-explode/matrixExplodeString?stringParameter=Test%21");
}

void OperationParameters::pathAndQueryUndefined()
{
    QString emptyLine;
    QJsonValue nullJson(QJsonValue::Null);
    QJsonValue undefinedJson(QJsonValue::Undefined);
    QJsonValue emptyJson;

    // style=form, explode=true, type=string
    CALL_TEST_POST_OPERATION(formExplodeString, emptyLine, "/v2/query/string/form-explode/formExplodeString?stringParameter=");

    // style=form, explode=false, type=string
    CALL_TEST_POST_OPERATION(formNotExplodeString, emptyLine, "/v2/query/string/form-not-explode/formNotExplodeString?stringParameter=");

    // style=matrix, explode=true, type=string
    CALL_TEST_OPERATION(matrixExplodeString, emptyLine, "/v2/path/string/matrix-explode/;stringParameter");

    // style=matrix, explode=false, type=string
    CALL_TEST_OPERATION(matrixNotExplodeString, emptyLine, "/v2/path/string/matrix-not-explode/;stringParameter");

    // style=matrix, explode=true, type=AnyType(NULL)
    CALL_TEST_OPERATION(matrixExplodeAnytype, nullJson, "/v2/path/anytype/matrix-explode/;anytypeParameter");

    // style=matrix, explode=false, type=AnyType(NULL)
    CALL_TEST_OPERATION(matrixNotExplodeAnytype, nullJson, "/v2/path/anytype/matrix-not-explode/;anytypeParameter");

    // style=matrix, explode=true, type=AnyType(undefined)
    CALL_TEST_OPERATION(matrixExplodeAnytype, undefinedJson, "/v2/path/anytype/matrix-explode/;anytypeParameter");

    // style=matrix, explode=false, type=AnyType(undefined)
    CALL_TEST_OPERATION(matrixNotExplodeAnytype, undefinedJson, "/v2/path/anytype/matrix-not-explode/;anytypeParameter");

    // style=matrix, explode=true, type=AnyType(empty)
    CALL_TEST_OPERATION(matrixExplodeAnytype, emptyJson, "/v2/path/anytype/matrix-explode/;anytypeParameter");

    // style=matrix, explode=false, type=AnyType(empty)
    CALL_TEST_OPERATION(matrixNotExplodeAnytype, emptyJson, "/v2/path/anytype/matrix-not-explode/;anytypeParameter");

    // undefined case for form with empty object
    // style=form, explode=false, type=object
    CALL_TEST_POST_OPERATION(formNotExplodeObject, TestObject(),
                             "/v2/query/object/form-not-explode/formNotExplodeObject?objectParameter=");

    // undefined case for form with empty object
    // style=form, explode=true, type=empty object
    CALL_TEST_POST_OPERATION(formExplodeObject, TestObject(),
                             "/v2/query/object/form-explode/formExplodeObject?objectParameter=");

    // style=form, explode=true, type=empty array
    CALL_TEST_POST_OPERATION(formExplodeArray, QList<int>(),
                             "/v2/query/array/form-explode/formExplodeArray?arrayParameter=");

    // style=form, explode=false, type=empty array
    CALL_TEST_POST_OPERATION(formNotExplodeArray, QList<int>(),
                             "/v2/query/array/form-not-explode/formNotExplodeArray?arrayParameter=");

    // Despite the fact that https://spec.openapis.org/oas/v3.1.1.html#style-values
    // defines the undefined column for null/emptied parameters,
    // servers decline empty LABEL or SIMPLE operations like
    // "/path/string/label-not-explode/." or "/path/string/simple-explode/"
    // and return a code 404
    //
    // style=label, explode=true, type=string
    CALL_NOT_FOUND_TEST_OPERATION(labelExplodeString, emptyLine);

    // style=label, explode=false, type=string
    CALL_NOT_FOUND_TEST_OPERATION(labelNotExplodeString, emptyLine);

    // style=simple, explode=true, type=string
    CALL_NOT_FOUND_TEST_OPERATION(simpleExplodeString, emptyLine);

    // style=simple, explode=false, type=string
    CALL_NOT_FOUND_TEST_OPERATION(simpleNotExplodeString, emptyLine);

    // style=simple, explode=true, type=AnyType(null)
    CALL_NOT_FOUND_TEST_OPERATION(simpleExplodeAnytype, nullJson);

    // style=simple, explode=false, type=AnyType(null)
    CALL_NOT_FOUND_TEST_OPERATION(simpleNotExplodeAnytype, nullJson);

    // style=simple, explode=true, type=AnyType(undefinedJson)
    CALL_NOT_FOUND_TEST_OPERATION(simpleExplodeAnytype, undefinedJson);

    // style=simple, explode=false, type=AnyType(undefinedJson)
    CALL_NOT_FOUND_TEST_OPERATION(simpleNotExplodeAnytype, undefinedJson);

    // style=simple, explode=true, type=AnyType(emptyJson)
    CALL_NOT_FOUND_TEST_OPERATION(simpleExplodeAnytype, emptyJson);

    // style=simple, explode=false, type=AnyType(emptyJson)
    CALL_NOT_FOUND_TEST_OPERATION(simpleNotExplodeAnytype, emptyJson);

    // style=label, explode=true, type=AnyType(undefined)
    CALL_NOT_FOUND_TEST_OPERATION(labelExplodeAnytype, undefinedJson);

    // style=label, explode=false, type=AnyType(undefined)
    CALL_NOT_FOUND_TEST_OPERATION(labelNotExplodeAnytype, undefinedJson);

    // style=label, explode=true, type=AnyType(undefined)
    CALL_NOT_FOUND_TEST_OPERATION(labelExplodeAnytype, nullJson);

    // style=label, explode=false, type=AnyType(undefined)
    CALL_NOT_FOUND_TEST_OPERATION(labelNotExplodeAnytype, nullJson);

    // style=label, explode=true, type=AnyType(emptyJson)
    CALL_NOT_FOUND_TEST_OPERATION(labelExplodeAnytype, emptyJson);

    // style=label, explode=false, type=AnyType(emptyJson)
    CALL_NOT_FOUND_TEST_OPERATION(labelNotExplodeAnytype, emptyJson);
}

void OperationParameters::severalQueryParametersPerOPeration()
{
    QString aParam("First param");
    QString bParam("second param");
    qint32 cParam = -3499;
    QString expectedResult;
    bool done = false;

    connect(this, &OperationParameters::formExplodeStringOptionsFinished,
            this, [&](const QString &summary) {
                done = true;
                QCOMPARE(getStatusString(summary), expectedResult);
            });
    connect(this, &OperationParameters::formExplodeStringOptionsErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &) {
                done = false;
            });

    expectedResult = "/v2/query/strings/form-explode/formExplodeStringOptions?stringParameterA=First%20param&stringParameterB=second%20param&stringParameterC=-3499";
    formExplodeStringOptions(bParam, OptionalParameter<QString>(aParam),
                             OptionalParameter<qint32>(cParam));
    QCOMPARE(m_manager->m_operationPath, expectedResult);
    QTRY_COMPARE_EQ(done, true);

    // NOTE: Empty optional parameters are being
    // excluded from the resulting URL!
    done = false;
    expectedResult = "/v2/query/strings/form-explode/formExplodeStringOptions?stringParameterA=First%20param&stringParameterB=second%20param";
    formExplodeStringOptions(bParam, aParam);
    QCOMPARE(m_manager->m_operationPath, expectedResult);
    QTRY_COMPARE_EQ(done, true);

    done = false;
    expectedResult = "/v2/query/strings/form-explode/formExplodeStringOptions?stringParameterB=second%20param";
    formExplodeStringOptions(bParam);
    QCOMPARE(m_manager->m_operationPath, expectedResult);
    QTRY_COMPARE_EQ(done, true);

    // NOTE: QtOpenAPI::OptionalParameter<T>() is equal to EMPTY optional parameter
    // The value will be excluded from the url
    done = false;
    expectedResult = "/v2/query/strings/form-explode/formExplodeStringOptions?stringParameterB=";
    formExplodeStringOptions("");
    QCOMPARE(m_manager->m_operationPath, expectedResult);
    QTRY_COMPARE_EQ(done, true);

    done = false;
    expectedResult = "/v2/query/strings/form-explode/formExplodeStringOptions?stringParameterB=";
    formExplodeStringOptions("", OptionalParameter<QString>());
    QCOMPARE(m_manager->m_operationPath, expectedResult);
    QTRY_COMPARE_EQ(done, true);

    done = false;
    expectedResult = "/v2/query/strings/form-explode/formExplodeStringOptions?stringParameterB=";
    formExplodeStringOptions("", OptionalParameter<QString>(), OptionalParameter<qint32>());
    QCOMPARE(m_manager->m_operationPath, expectedResult);
    QTRY_COMPARE_EQ(done, true);

    // NOTE: NULL != Empty! optional(Null) is being serialized like
    // 'undefined' column here: https://spec.openapis.org/oas/v3.1.1.html#style-examples
    done = false;
    expectedResult = "/v2/query/strings/form-explode/formExplodeStringOptions?stringParameterA=&stringParameterB=";
    formExplodeStringOptions("", OptionalParameter<QString>(OptionalParameter<QString>::IsNull));
    QCOMPARE(m_manager->m_operationPath, expectedResult);
    QTRY_COMPARE_EQ(done, true);

    done = false;
    expectedResult = "/v2/query/strings/form-explode/formExplodeStringOptions?stringParameterA=&stringParameterB=&stringParameterC=";
    formExplodeStringOptions("", OptionalParameter<QString>(OptionalParameter<QString>::IsNull), OptionalParameter<qint32>(OptionalParameter<qint32>::IsNull));
    QCOMPARE(m_manager->m_operationPath, expectedResult);
    QTRY_COMPARE_EQ(done, true);

    done = false;
    expectedResult = "/v2/query/strings/form-explode/formExplodeStringOptions?stringParameterA=&stringParameterB=";
    formExplodeStringOptions("", OptionalParameter<QString>(OptionalParameter<QString>::IsNull), OptionalParameter<qint32>());
    QCOMPARE(m_manager->m_operationPath, expectedResult);
    QTRY_COMPARE_EQ(done, true);

    // NOTE: Empty string != empty parameter. It is being serialized like
    // 'undefined' column here: https://spec.openapis.org/oas/v3.1.1.html#style-examples
    done = false;
    expectedResult = "/v2/query/strings/form-explode/formExplodeStringOptions?stringParameterA=&stringParameterB=&stringParameterC=";
    formExplodeStringOptions("", OptionalParameter<QString>(""), OptionalParameter<qint32>(OptionalParameter<qint32>::IsNull));
    QCOMPARE(m_manager->m_operationPath, expectedResult);
    QTRY_COMPARE_EQ(done, true);

    done = false;
    expectedResult = "/v2/query/strings/form-explode/formExplodeStringOptions?stringParameterA=%20end%21&stringParameterB=The&stringParameterC=100";
    formExplodeStringOptions("The", OptionalParameter<QString>(" end!"), OptionalParameter<qint32>(100));
    QCOMPARE(m_manager->m_operationPath, expectedResult);
    QTRY_COMPARE_EQ(done, true);

    done = false;
    TestObject testObject;
    testObject.setName("John");
    testObject.setStatus("Sleepy");
    testObject.setAge(12);
    formExplodeDifferentOptions(50, OptionalParameter<TestObject>(testObject),
                                this, [&](const QRestReply &reply, const QString &summary) {
                                    done = reply.isSuccess();
                                    QCOMPARE(getStatusString(summary), "/v2/query/strings/form-explode/formExplodeDifferentOptions?age=12&name=John&status=Sleepy&stringParameterB=50");
                                });

    QTRY_COMPARE_EQ(done, true);

    done = false;
    formExplodeDifferentOptions(50, OptionalParameter<TestObject>(),
                                this, [&](const QRestReply &reply, const QString &summary) {
                                    done = reply.isSuccess();
                                    QCOMPARE(getStatusString(summary), "/v2/query/strings/form-explode/formExplodeDifferentOptions?stringParameterB=50");
                                });
    QTRY_COMPARE_EQ(done, true);

    // NOTE: Parameter is not nullable in yaml file, so it's being excluded
    // from the serialization, even if Null is passed as a 2d argument.
    done = false;
    formExplodeDifferentOptions(50, OptionalParameter<TestObject>(OptionalParameter<TestObject>::IsNull), this,
                                [&](const QRestReply &reply, const QString &summary) {
                                    done = reply.isSuccess();
                                    QCOMPARE(getStatusString(summary), "/v2/query/strings/form-explode/formExplodeDifferentOptions?stringParameterB=50");
                                });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::pathAndQueryParameters()
{
    QString expectedResult;
    QString stringParam("str * param");
    QList<int> arrayParam({22, -150, 0});

    expectedResult = "/v2/path/string/matrix-explode/;stringParameter=str%20%2A%20param/query/array/spaceDelimited-not-explode?arrayParameter=22%20-150%200"_L1;
    CALL_TEST_OPERATION_MULTI_PARAMS(queryAndPathParams, expectedResult, stringParam, arrayParam);

    // style=form, explode=true, type=string : Invalid style for path parameters => use simple style
    // style=label, explode=true, type=array : Invalid style for query parameters => use form style
#ifdef Q_OS_WIN
    const char *pathWarningMsg = "'form' style is invalid for path parameters.\r\nAllowed styles "
                                 "are: 'matrix', 'label' and 'simple'.\r\nFalling back to the "
                                 "default style 'simple'.";
    const char *queryWarningMsg = "'label' style is invalid for query parameters.\r\nAllowed "
                                  "styles are: 'form', 'spaceDelimited', 'pipeDelimited' and "
                                  "'deepObject'.\r\nFalling back to the default style 'form'.";
#else
    const char *pathWarningMsg = "'form' style is invalid for path parameters.\nAllowed styles are:"
                                 " 'matrix', 'label' and 'simple'.\nFalling back to the default "
                                 "style 'simple'.";
    const char *queryWarningMsg = "'label' style is invalid for query parameters.\nAllowed styles "
                                  "are: 'form', 'spaceDelimited', 'pipeDelimited' and 'deepObject'."
                                  "\nFalling back to the default style 'form'.";
#endif
    QTest::ignoreMessage(QtWarningMsg, pathWarningMsg);
    QTest::ignoreMessage(QtWarningMsg, queryWarningMsg);
    expectedResult = "/v2/path/string/invalid-form-explode/str%20%2A%20param/query/array/invalid-label-explode?arrayParameter=22&arrayParameter=-150&arrayParameter=0"_L1;
    CALL_TEST_OPERATION_MULTI_PARAMS(invalidStylesQueryPathParams, expectedResult, stringParam,
                                     arrayParam);
}

/**
 * Serialization of Headers is a tricky thing in the referenced SPEC OpenApi 3.1.1
 * See: https://spec.openapis.org/oas/v3.1.1.html#appendix-d-serializing-headers-and-cookies
 *
 * Historically, many custom HTTP headers use semicolon-delimited key-value pairs, e.g.:
 * Content-Type: application/json; charset=utf-8
 * It means that a custom header with multiple values could look like:
 * X-Custom-Header: id=123; role=admin
 *
 * But rfc7230 defines that if the header with the same field name appears
 * multiple times (e.g. X-Custom-Header), the server, client, or proxy can combine
 * them into one line, separated by commas.
 * See: https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.2
 * According to the RFC, when multiple headers have the same field name,
 * their values are combined into a single header. For example:
 * X-Custom-Header: value1
 * X-Custom-Header: value2
 * Should be presented as: X-Custom-Header: value1, value2
 *
 * The newer RFC 9110 makes no assumption that comma-separated values are not valid, but!
 * In short it says: ONLY the headers that are explicitly defined as LIST-BASED
 * can be merged using a comma as a separator.
 *
 * The OpenApi 3.1.1 defines that header can contain multiple values, separated by commas,
 * by using style="simple" for operation parameter headers.
 * Another useful reference link:
 * https://www.speakeasy.com/openapi/requests/parameters/header-parameters
 *
 * What does it mean for us?
 * Our purpose is to use the Openapi 3.1.1 SPEC as the reference.
 * So, we are likely somehow compliant to the rfc7230 now.
 * NOTE: Servers that are generated by OpenAPI generators correctly parse these headers.
 *
 * If users want to use ";" as a seperator between different values of the same header,
 * they can apply some tricks, like following *.yaml example:
 *
 * parameters:
 *   - name: X-User-Context
 *     in: header
 *     required: true
 *     description: "Structured user context in key=value pairs.
 *                   Fields: id, role, source.
 *                   Format: `id=123; role=admin; source=web`
 *                   This header is a single string value, not a list.
 *                   Parsers must not split on commas."
 *     schema:
 *       type: string
 *       example: id=123; role=admin; source=web
 *
 * Another way is using content + media type for custom header definition.
 *
 * NOTE: OpenAPI does not say anything about percent-encoding for header,
 * though it doesn't mean it is safe to use special characters, like
 * gen-delims / sub-delims in the header.
 * There should be some additional agreements between client/server
 * parts in case of special symbols usage.
 */
void OperationParameters::headerAnyTypeParameters_data()
{
    TestObject obj;
    obj.setName("Super Puper *+,;=!$&'()");
    obj.setStatus("Awake!");

    QTest::addColumn<QJsonValue>("jsonValue");
    QTest::addColumn<QString>("expectedExplodeResult");
    QTest::addColumn<QString>("expectedNotExplodeResult");
    QTest::newRow("QJsonValue(string)")
        << QJsonValue("John Doe") << QString("John Doe") << QString("John Doe");
    QTest::newRow("QJsonValue(int)")
        << QJsonValue(100) << QString("100") << QString("100");
    QTest::newRow("QJsonValue(array)")
        << QJsonValue({ 1, 2.2, QString("Strange *+,;=!$&'()") })
        << QString("1,2.2,Strange *+,;=!$&'()") << QString("1,2.2,Strange *+,;=!$&'()");
    QTest::newRow("QJsonValue(object)")
        << QJsonValue(obj.asJsonObject())
        << QString("name=Super Puper *+,;=!$&'(),status=Awake!")
        << QString("name,Super Puper *+,;=!$&'(),status,Awake!");
    QTest::newRow("QJsonValue(Null)")
        << QJsonValue(QJsonValue::Null) << QString() << QString();
    QTest::newRow("QJsonValue(Undefined)")
        << QJsonValue(QJsonValue::Undefined) << QString() << QString();
    QTest::newRow("QJsonValue()") << QJsonValue() << QString() << QString();
}

void OperationParameters::headerAnyTypeParameters()
{
    QFETCH(QJsonValue, jsonValue);
    QFETCH(QString, expectedExplodeResult);
    QFETCH(QString, expectedNotExplodeResult);
    bool done = false;
    headerSimpleExplodeAnytype(jsonValue,
                               OptionalParameter<QString>("I am a QUERY string"),
                               this, [&](const QRestReply &reply, const QString &summary) {
                                   done = reply.isSuccess();
                                   QCOMPARE(getHeaderValue(summary, "Any-Type-Parameter"),
                                            expectedExplodeResult);
                               });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    headerSimpleNotExplodeAnytype(jsonValue, OptionalParameter<QString>(),
                                  this, [&](const QRestReply &reply, const QString &summary) {
                                      done = reply.isSuccess();
                                      QCOMPARE(getHeaderValue(summary, "Any-Type-Parameter"),
                                               expectedNotExplodeResult);
                                  });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::headerStringParameters_data()
{
    QTest::addColumn<QString>("stringValue");
    QTest::addColumn<QString>("expectedExplodeResult");
    QTest::addColumn<QString>("expectedNotExplodeResult");

    QTest::newRow("QString with sub-delims")
        << QString("I am a simple string *+,;=!$&'()")
        << QString("I am a simple string *+,;=!$&'()")
        << QString("I am a simple string *+,;=!$&'()");
    QTest::newRow("QString with numbers")
        << QString("1236498910") << QString("1236498910") << QString("1236498910");
    QTest::newRow("Empty string") << QString() << QString() << QString();
}

void OperationParameters::headerStringParameters()
{
    QFETCH(QString, stringValue);
    QFETCH(QString, expectedExplodeResult);
    QFETCH(QString, expectedNotExplodeResult);
    bool done = false;
    headerSimpleExplodeString(stringValue,
                              OptionalParameter<QString>("I am a QUERY string"),
                               this, [&](const QRestReply &reply, const QString &summary) {
                                   done = reply.isSuccess();
                                   QCOMPARE(getHeaderValue(summary, "String-Parameter"),
                                            expectedExplodeResult);
                               });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    headerSimpleNotExplodeString(stringValue, OptionalParameter<QString>(),
                                  this, [&](const QRestReply &reply, const QString &summary) {
                                      done = reply.isSuccess();
                                      QCOMPARE(getHeaderValue(summary, "String-Parameter"),
                                               expectedNotExplodeResult);
                                  });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::headerArrayParameters_data()
{
    QTest::addColumn<QList<qint32>>("arrayValue");
    QTest::addColumn<QString>("expectedExplodeResult");
    QTest::addColumn<QString>("expectedNotExplodeResult");

    QTest::newRow("QList<qint32>{regular numbers}")
        << QList<qint32>({0, 1, -7654, 1987357, 0, 0, 1876, -997675})
        << QString("0,1,-7654,1987357,0,0,1876,-997675")
        << QString("0,1,-7654,1987357,0,0,1876,-997675");
    QTest::newRow("QList<qint32>{limits}")
        << QList<qint32>({std::numeric_limits<qint32>::min(),
                          0, std::numeric_limits<qint32>::max()})
        << QString("-2147483648,0,2147483647") << QString("-2147483648,0,2147483647");
    QTest::newRow("QList<qint32>{empty}")
        << QList<qint32>({}) << QString() << QString();
}

void OperationParameters::headerArrayParameters()
{
    QFETCH(QList<qint32>, arrayValue);
    QFETCH(QString, expectedExplodeResult);
    QFETCH(QString, expectedNotExplodeResult);
    bool done = false;
    headerSimpleExplodeArray(arrayValue,
                             OptionalParameter<QString>("I am a QUERY string"),
                             this, [&](const QRestReply &reply, const QString &summary) {
                                 done = reply.isSuccess();
                                 QCOMPARE(getHeaderValue(summary, "Array-Parameter"),
                                          expectedExplodeResult);
                             });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    headerSimpleNotExplodeArray(OptionalParameter<QList<qint32>>(arrayValue),
                                OptionalParameter<QString>(),
                                this, [&](const QRestReply &reply, const QString &summary) {
                                    done = reply.isSuccess();
                                    QCOMPARE(getHeaderValue(summary, "Array-Parameter"),
                                             expectedNotExplodeResult);
                                });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::headerObjectParameters_data()
{
    TestObject obj1, obj2, obj3;
    obj1.setName("Super Puper *+,;=!$&'()");
    obj1.setStatus("Awake!");
    obj2.setName("Simple123456789");
    obj2.setStatus("Sleeping");

    QTest::addColumn<TestObject>("objectValue");
    QTest::addColumn<QString>("expectedExplodeResult");
    QTest::addColumn<QString>("expectedNotExplodeResult");

    QTest::newRow("Object with sub-delim characters")
        << obj1
        << QString("name=Super Puper *+,;=!$&'(),status=Awake!")
        << QString("name,Super Puper *+,;=!$&'(),status,Awake!");
    QTest::newRow("Object with simple strings")
        << obj2 << QString("name=Simple123456789,status=Sleeping")
        << QString("name,Simple123456789,status,Sleeping");
    QTest::newRow("Empty object") << obj3 << QString() << QString();
}

void OperationParameters::headerObjectParameters()
{
    QFETCH(TestObject, objectValue);
    QFETCH(QString, expectedExplodeResult);
    QFETCH(QString, expectedNotExplodeResult);
    bool done = false;
    headerSimpleExplodeObject(objectValue,
                              OptionalParameter<QString>("I am a QUERY string"),
                              this, [&](const QRestReply &reply, const QString &summary) {
                                  done = reply.isSuccess();
                                  QCOMPARE(getHeaderValue(summary, "Object-Parameter"),
                                           expectedExplodeResult);
                              });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    headerSimpleNotExplodeObject(objectValue, OptionalParameter<QString>(),
                                 this, [&](const QRestReply &reply, const QString &summary) {
                                     done = reply.isSuccess();
                                     QCOMPARE(getHeaderValue(summary, "Object-Parameter"),
                                              expectedNotExplodeResult);
                                 });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::headerMapParameters_data()
{
    QMap<QString, QString> stringMap;
    stringMap["key1"_L1] = QString("Super Puper *+,;=!$&'()");
    stringMap["key2"_L1] = QString("Simple123456789");

    QTest::addColumn<QMap<QString, QString>>("mapValue");
    QTest::addColumn<QString>("expectedExplodeResult");
    QTest::addColumn<QString>("expectedNotExplodeResult");

    QTest::newRow("Non-empty map") << stringMap
                                << QString("key1=Super Puper *+,;=!$&'(),key2=Simple123456789")
                                << QString("key1,Super Puper *+,;=!$&'(),key2,Simple123456789");
    QTest::newRow("Empty map")  << QMap<QString, QString>() << QString() << QString();
}

void OperationParameters::headerMapParameters()
{
    using StringMap = QMap<QString, QString>;
    QFETCH(StringMap, mapValue);
    QFETCH(QString, expectedExplodeResult);
    QFETCH(QString, expectedNotExplodeResult);
    bool done = false;
    headerSimpleExplodeMap(mapValue, OptionalParameter<QString>(""),
                           this, [&](const QRestReply &reply, const QString &summary) {
                               done = reply.isSuccess();
                               QCOMPARE(getHeaderValue(summary, "Map-Parameter"),
                                         expectedExplodeResult);
                           });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    headerSimpleNotExplodeMap(mapValue,
                              OptionalParameter<QString>("I am a QUERY string"),
                              this, [&](const QRestReply &reply, const QString &summary) {
                                  done = reply.isSuccess();
                                  QCOMPARE(getHeaderValue(summary, "Map-Parameter"),
                                           expectedNotExplodeResult);
                              });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::headerAdditionalCases_data()
{
    QTest::addColumn<QString>("stringValue");
    QTest::addColumn<QString>("expectedExplodeResult1");
    QTest::addColumn<QString>("expectedExplodeResult2");

    QTest::newRow("QString(simple string 1)")
        << QString("name=John;status=Sleep;age=34")
        << QString("name=John;status=Sleep;age=34")
        << QString("name=John;status=Sleep;age=34");
    QTest::newRow("QString(simple string 2)")
        << QString() << QString()
        << QString("application/x-www-form-urlencoded");
}

void OperationParameters::headerAdditionalCases()
{
    QFETCH(QString, stringValue);
    QFETCH(QString, expectedExplodeResult1);
    QFETCH(QString, expectedExplodeResult2);
    bool done = false;
    headerSimpleNotExplodeCustom(OptionalParameter<QString>(stringValue), this,
                                 [&](const QRestReply &reply, const QString &summary) {
                                     done = reply.isSuccess();
                                     QCOMPARE(getHeaderValue(summary, "String-Parameter"),
                                              expectedExplodeResult1);
                                 });
    QTRY_COMPARE_EQ(done, true);

    // FIX ME: the name Content-Type should be ignored.
    // See: https://spec.openapis.org/oas/v3.1.1.html#common-fixed-fields
    // TODO: https://bugreports.qt.io/browse/QTBUG-140168
    // Content-Type == "application/x-www-form-urlencoded" for empty string here, because
    // the name is a duplicate of the actual Content-Type, and the actual one
    // cannot be empty in the Qt Network.
    done = false;
    headerSimpleNotExplodeContentType(stringValue,
                                      this, [&](const QRestReply &reply, const QString &summary) {
                                          done = reply.isSuccess();
                                          QCOMPARE(getHeaderValue(summary, "Content-Type"),
                                                   expectedExplodeResult2);
                                      });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::severalHeaderParameters()
{
    bool done = false;
    TestObject obj;
    obj.setName("object-header"_L1);
    obj.setStatus("Awake!"_L1);

    severalHeaderParametersOp(QString("String-header!"_L1), 300, obj,
                              this, [&](const QRestReply &reply, const QString &summary) {
                                  done = reply.isSuccess();
                                  QCOMPARE(getHeaderValue(summary, "String-Parameter"_L1),
                                           "String-header!"_L1);
                                  QCOMPARE(getHeaderValue(summary, "Int-Parameter"_L1), "300"_L1);
                                  QCOMPARE(getHeaderValue(summary, "Object-Parameter"_L1),
                                           "name=object-header,status=Awake!"_L1);
                              });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::headerInvalidStyle()
{
    const QString stringValue = "Just some random string"_L1;
    bool done = false;

#ifdef Q_OS_WIN
    const char *warningMsg = "'label' style is invalid for header parameters.\r\n"
                             "Falling back to the default style 'simple'.";
#else
    const char *warningMsg = "'label' style is invalid for header parameters.\n"
                             "Falling back to the default style 'simple'.";
#endif
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    headerInvalidLabelNotExplodeString(stringValue, this,
        [&](const QRestReply &reply, const QString &summary) {
            done = reply.isSuccess();
            QCOMPARE(getHeaderValue(summary, "String-Parameter"), stringValue);
        });
    QTRY_COMPARE_EQ(done, true);
}

/**
 * Cookies are a part of Operation parameters.
 * Cookies support only style=form. Cookies can be presented with
 * a schema or a content type.
 *
 * See: https://spec.openapis.org/oas/v3.1.1.html#appendix-d-serializing-headers-and-cookies
 * For more see:
 * https://www.speakeasy.com/openapi/requests/parameters/cookie-parameters
 *
 * Cookies are also an interesting topic.
 * The thing is, cookies are not well defined in the OpenAPI 3.1.1,
 * which is our referenced SPEC version.
 *
 * There are 2 cases supported:
 * - normal case: style=form + explode=false;
 * - not well defined case: style=form + explode=true;
 *
 * What does "not well defined case" mean?
 * It means such serialization is not supported by any known frameworks
 * or RFC, except OpenApi 3.1.1 itself.
 *
 * What does RFC say?
 * Historically, Cookies are "name1=value1; name2=value2" pairs separated by semicolons.
 * See Cookie header semantic rules:
 * https://datatracker.ietf.org/doc/html/rfc6265#section-5.2
 *
 * So, each "name=value" pair is being interpreted as a separate cookie.
 * And the delimiter between seperate cookies is ';', not a ','.
 *
 * Servers do understand commas inside a cookie value,
 * as long as they are part of a single cookie's value.
 * For example: "key=value1,text,value2,something" is a valid single cookie.
 *
 * And no, servers do not interpret commas as a separator for multiple
 * "key=value" pairs. It means servers do not interpret the following
 * case as two separate cookies:
 * "name1=value1, name2=value2"
 *
 * If you want to set several separate independent cookies,
 * you need to use semicolons as follows:
 * "name1=value1; name2=value2"
 *
 * Speaking about our current Cookie implementation,
 * it supports OpenApi 3.1.1 as it is.
 *
 * See supported examples with object type:
 * style=form, explode=false looks like: "color=R,100,G,200,B,150"
 * style=form, explode=true looks like: "R=100&G=200&B=150"
 *
 * Problems of explode=true are obvious here:
 * - It looks like separate pairs.
 * - It uses an inappropriate '&' delimiter.
 *
 * Users should decide if they want to use explode=true.
 * It will always mean that the server side should be aware of it somehow
 * by additional implementation agreements.
 *
 * But in general using explode=true is not recommended, though
 * there is no reason to prohibit it entirely.
 */
void OperationParameters::cookieAnyTypeParameters_data()
{
    TestObject obj;
    obj.setName("Super Puper");
    obj.setStatus("Awake!");

    QTest::addColumn<QJsonValue>("jsonValue");
    QTest::addColumn<QString>("expectedExplodeResult");
    QTest::addColumn<QString>("expectedNotExplodeResult");

    QTest::newRow("QJsonValue(string)")
        << QJsonValue("John Doe")
        << QString("anytypeParameter=John Doe")
        << QString("anytypeParameter=John Doe");
    QTest::newRow("QJsonValue(int)")
        << QJsonValue(100)
        << QString("anytypeParameter=100") << QString("anytypeParameter=100");
    QTest::newRow("QJsonValue(array)")
        << QJsonValue({ 1, 2.2, QString("Strange") })
        << QString("anytypeParameter=1&anytypeParameter=2.2&anytypeParameter=Strange")
        << QString("anytypeParameter=1,2.2,Strange");
    QTest::newRow("QJsonValue(object)")
        << QJsonValue(obj.asJsonObject())
        << QString("name=Super Puper&status=Awake!")
        << QString("anytypeParameter=name,Super Puper,status,Awake!");
    QTest::newRow("QJsonValue(Null)")
        << QJsonValue(QJsonValue::Null)
        << QString("anytypeParameter=")
        << QString("anytypeParameter=");
    QTest::newRow("QJsonValue(Undefined)")
        << QJsonValue(QJsonValue::Undefined)
        << QString("anytypeParameter=")
        << QString("anytypeParameter=");
    QTest::newRow("QJsonValue()")
        << QJsonValue()
        << QString("anytypeParameter=")
        << QString("anytypeParameter=");
}

void OperationParameters::cookieAnyTypeParameters()
{
    QFETCH(QJsonValue, jsonValue);
    QFETCH(QString, expectedExplodeResult);
    QFETCH(QString, expectedNotExplodeResult);
    bool done = false;
    // NOTE: if you send cookies from a client to a server, use "Cookie" header name.
    // If you send cookies from the server to the client, then use "Set-Cookie" header name.
    // All cookie related operations use "Cookie" inside, because it's a Client part.
    cookieExplodeAnytype(jsonValue,
                         this, [&](const QRestReply &reply, const QString &summary) {
                             done = reply.isSuccess();
                             QCOMPARE(getHeaderValue(summary, "Cookie"_L1),
                                      expectedExplodeResult);
                             if (jsonValue.type() == QJsonValue::Object) {
                                 // explode=true for objects doesn't work well on server side.
                                 // It is not possible to detect cookie value by Parameter Name
                                 // on server side, because the serialized string doesn't contain it:
                                 // "name=Super Puper&status=Awake!"
                                 // Just be aware.
                                 QCOMPARE(getObjectValue(summary, "error"_L1).toString(),
                                          "http: named cookie not present"_L1);
                             } else {
                                QVERIFY(getObjectValue(summary, "error"_L1).toString().isEmpty());
                             }
                         });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    cookieNotExplodeAnytype(jsonValue,
                            this, [&](const QRestReply &reply, const QString &summary) {
                                done = reply.isSuccess();
                                QCOMPARE(getHeaderValue(summary, "Cookie"_L1),
                                         expectedNotExplodeResult);
                                QVERIFY(getObjectValue(summary, "error"_L1).toString().isEmpty());
                            });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::cookieStringParameters_data()
{
    QTest::addColumn<QString>("stringValue");
    QTest::addColumn<QString>("expectedExplodeResult");
    QTest::addColumn<QString>("expectedNotExplodeResult");

    QTest::newRow("QString with sub-delims")
        << QString("I am a simple string")
        << QString("stringParameter=I am a simple string")
        << QString("stringParameter=I am a simple string");
    QTest::newRow("QString with numbers")
        << QString("1236498910")
        << QString("stringParameter=1236498910")
        << QString("stringParameter=1236498910");
    QTest::newRow("Empty string")
        << QString() << QString("stringParameter=") << QString("stringParameter=");
}

void OperationParameters::cookieStringParameters()
{
    QFETCH(QString, stringValue);
    QFETCH(QString, expectedExplodeResult);
    QFETCH(QString, expectedNotExplodeResult);
    bool done = false;
    cookieExplodeString(stringValue, QString("I am a PATH string"),
                        this, [&](const QRestReply &reply, const QString &summary) {
                            done = reply.isSuccess();
                            QCOMPARE(getHeaderValue(summary, "Cookie"_L1),
                                     expectedExplodeResult);
                            QVERIFY(getObjectValue(summary, "error"_L1).toString().isEmpty());
                        });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    cookieNotExplodeString(QString("I am a PATH string"), stringValue,
                           this, [&](const QRestReply &reply, const QString &summary) {
                               done = reply.isSuccess();
                               QCOMPARE(getHeaderValue(summary, "Cookie"_L1),
                                        expectedNotExplodeResult);
                               QVERIFY(getObjectValue(summary, "error"_L1).toString().isEmpty());
                           });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::cookieArrayParameters_data()
{
    QTest::addColumn<QList<qint32>>("arrayValue");
    QTest::addColumn<QString>("expectedExplodeResult");
    QTest::addColumn<QString>("expectedNotExplodeResult");

    QTest::newRow("QList<qint32>{regular numbers}")
        << QList<qint32>({0, 1, -7654, 1987357, 0, 0, 1876, -997675})
        << QString("arrayParameter=0&arrayParameter=1&arrayParameter=-7654&arrayParameter=1987357&arrayParameter=0&arrayParameter=0&arrayParameter=1876&arrayParameter=-997675")
        << QString("arrayParameter=0,1,-7654,1987357,0,0,1876,-997675");
    QTest::newRow("QList<qint32>{limits}")
        << QList<qint32>({std::numeric_limits<qint32>::min(),
                          0, std::numeric_limits<qint32>::max()})
        << QString("arrayParameter=-2147483648&arrayParameter=0&arrayParameter=2147483647")
        << QString("arrayParameter=-2147483648,0,2147483647");
    QTest::newRow("QList<qint32>{empty}")
        << QList<qint32>({}) << QString("arrayParameter=") << QString("arrayParameter=");
}

void OperationParameters::cookieArrayParameters()
{
    QFETCH(QList<qint32>, arrayValue);
    QFETCH(QString, expectedExplodeResult);
    QFETCH(QString, expectedNotExplodeResult);
    bool done = false;
    cookieExplodeArray(arrayValue,
                       this, [&](const QRestReply &reply, const QString &summary) {
                           done = reply.isSuccess();
                           QCOMPARE(getHeaderValue(summary, "Cookie"_L1),
                                    expectedExplodeResult);
                           QVERIFY(getObjectValue(summary, "error"_L1).toString().isEmpty());
                       });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    cookieNotExplodeArray(arrayValue,
                          this, [&](const QRestReply &reply, const QString &summary) {
                              done = reply.isSuccess();
                              QCOMPARE(getHeaderValue(summary, "Cookie"_L1),
                                       expectedNotExplodeResult);
                              QVERIFY(getObjectValue(summary, "error"_L1).toString().isEmpty());
                          });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::cookieObjectParameters_data()
{
    TestObject obj1, obj2, obj3;
    obj1.setName("Super Puper");
    obj1.setStatus("Awake!");
    obj2.setName("Simple123456789");
    obj2.setStatus("Sleeping");

    QTest::addColumn<TestObject>("objectValue");
    QTest::addColumn<QString>("expectedExplodeResult");
    QTest::addColumn<QString>("expectedNotExplodeResult");

    QTest::newRow("Object with sub-delim characters")
        << obj1
        << QString("name=Super Puper&status=Awake!")
        << QString("objectParameter=name,Super Puper,status,Awake!");
    QTest::newRow("Object with simple strings")
        << obj2
        << QString("name=Simple123456789&status=Sleeping")
        << QString("objectParameter=name,Simple123456789,status,Sleeping");
    QTest::newRow("Empty object") << obj3
                                  << QString("objectParameter=")
                                  << QString("objectParameter=");
}

void OperationParameters::cookieObjectParameters()
{
    QFETCH(TestObject, objectValue);
    QFETCH(QString, expectedExplodeResult);
    QFETCH(QString, expectedNotExplodeResult);
    bool done = false;
    cookieExplodeObject(objectValue,
                        this, [&](const QRestReply &reply, const QString &summary) {
                            done = reply.isSuccess();
                            QCOMPARE(getHeaderValue(summary, "Cookie"_L1),
                                     expectedExplodeResult);
                            // explode=true for objects doesn't work well on server side.
                            // It is not possible to detect cookie value by Parameter Name
                            // on server side, because the serialized string doesn't contain it:
                            // obj1 = "name=Super Puper&status=Awake!"
                            // obj2 = "name=Simple123456789&status=Sleeping".
                            // Just be aware.
                            if (!objectValue.asJsonObject().empty()) {
                                QCOMPARE(getObjectValue(summary, "error"_L1).toString(),
                                         "http: named cookie not present"_L1);
                            } else {
                                // Obj3 is empty, so we send objectParameter=
                                // as described in "undefined" column
                                // see style=form, explode=true
                                // https://spec.openapis.org/oas/v3.1.1.html#style-examples
                                QVERIFY(getObjectValue(summary, "error"_L1).toString().isEmpty());
                            }
                        });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    cookieNotExplodeObject(objectValue,
                           this, [&](const QRestReply &reply, const QString &summary) {
                               done = reply.isSuccess();
                               QCOMPARE(getHeaderValue(summary, "Cookie"_L1),
                                        expectedNotExplodeResult);
                               QVERIFY(getObjectValue(summary, "error"_L1).toString().isEmpty());
                           });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::cookieMapParameters_data()
{
    QMap<QString, QString> stringMap;
    stringMap["key1"_L1] = QString("Super Puper");
    stringMap["key2"_L1] = QString("Simple123456789");

    QTest::addColumn<QMap<QString, QString>>("mapValue");
    QTest::addColumn<QString>("expectedExplodeResult");
    QTest::addColumn<QString>("expectedNotExplodeResult");

    QTest::newRow("Non-empty map")
        << stringMap
        << QString("key1=Super Puper&key2=Simple123456789")
        << QString("mapParameter=key1,Super Puper,key2,Simple123456789");
    QTest::newRow("Empty map")
        << QMap<QString, QString>()
        << QString("mapParameter=") << QString("mapParameter=");
}

void OperationParameters::cookieMapParameters()
{
    using StringMap = QMap<QString, QString>;
    QFETCH(StringMap, mapValue);
    QFETCH(QString, expectedExplodeResult);
    QFETCH(QString, expectedNotExplodeResult);
    bool done = false;
    cookieExplodeStringMap(mapValue,
                           this, [&](const QRestReply &reply, const QString &summary) {
                               done = reply.isSuccess();
                               QCOMPARE(getHeaderValue(summary, "Cookie"_L1),
                                        expectedExplodeResult);
                               // explode=true for objects doesn't work well on server side.
                               // It is not possible to detect cookie value by parameter name
                               // on server side, because the serialized string doesn't contain it:
                               // "key1=Super Puper&key2=Simple123456789". Just be aware.
                               if (!mapValue.empty()) {
                                   QCOMPARE(getObjectValue(summary, "error"_L1).toString(),
                                            "http: named cookie not present"_L1);
                               } else {
                                   // Map is empty, so we send mapParameter=
                                   // as described in "undefined" column
                                   // see style=form, explode=true
                                   // https://spec.openapis.org/oas/v3.1.1.html#style-examples
                                   QVERIFY(getObjectValue(summary, "error"_L1).toString().isEmpty());
                               }
                           });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    cookieNotExplodeStringMap(mapValue,
                              this, [&](const QRestReply &reply, const QString &summary) {
                                  done = reply.isSuccess();
                                  QCOMPARE(getHeaderValue(summary, "Cookie"_L1),
                                           expectedNotExplodeResult);
                                  QVERIFY(getObjectValue(summary, "error"_L1).toString().isEmpty());
                              });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::severalCookies()
{
    QMap<QString, QString> stringMap, emptyMap;
    stringMap["key1"_L1] = QString("Super Puper");
    stringMap["key2"_L1] = QString("Simple123456789");

    bool done = false;
    severalNotExplodeCookies(stringMap, 100,
                             this, [&](const QRestReply &reply, const QString &summary) {
                                 done = reply.isSuccess();
                                 QCOMPARE(getObjectValue(summary, "cookie1"_L1).toString(),
                                          "key1,Super Puper,key2,Simple123456789");
                                 QCOMPARE(getObjectValue(summary, "cookie2"_L1).toString(), "100");
                                 QCOMPARE(getObjectValue(summary, "size"_L1).toInt(), 2);
                             });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    severalNotExplodeCookies(OptionalParameter<QMap<QString, QString>>(emptyMap),
                             OptionalParameter<qint64>(-999999999),
                             this, [&](const QRestReply &reply, const QString &summary) {
                                 done = reply.isSuccess();
                                 QVERIFY(getObjectValue(summary,
                                                        "cookie1"_L1).toString().isEmpty());
                                 QCOMPARE(getObjectValue(summary,
                                                         "cookie2"_L1).toString(), "-999999999");
                                 QCOMPARE(getObjectValue(summary, "size"_L1).toInt(), 2);
                             });
    QTRY_COMPARE_EQ(done, true);

    // Cookies are empty, because the empty optional parameter is not being sent at all
    // so nothing to send back from the server
    done = true;
    severalNotExplodeCookies(OptionalParameter<QMap<QString, QString>>(),
                             OptionalParameter<qint64>(),
                             this, [&](const QRestReply &reply, const QString &) {
                                 done = reply.isSuccess();
                             });
    QTRY_COMPARE_EQ(done, false);

    done = false;
    severalExplodeCookies(stringMap, 100,
                          this, [&](const QRestReply &reply, const QString &summary) {
                              done = reply.isSuccess();
                              // Server doesn't parse objects like
                              // "key1=Super Puper&key2=Simple123456789"
                              // because key=value pairs are being interpreted as a
                              // separate values
                              QCOMPARE(getObjectValue(summary, "cookie1"_L1).toString(), "");
                              // Primitive types are OK, the serialization is the same
                              // like explode=false for primitives
                              QCOMPARE(getObjectValue(summary, "cookie2"_L1).toString(), "100");
                              QCOMPARE(getObjectValue(summary, "size"_L1).toInt(), 2);
                          });
    QTRY_COMPARE_EQ(done, true);

    done = false;
    severalExplodeCookies(OptionalParameter<QMap<QString, QString>>(emptyMap),
                          OptionalParameter<qint64>(-999999999),
                          this, [&](const QRestReply &reply, const QString &summary) {
                              done = reply.isSuccess();
                              QCOMPARE(getObjectValue(summary, "cookie1"_L1).toString(), "");
                              QCOMPARE(getObjectValue(summary, "cookie2"_L1).toString(),
                                       "-999999999");
                              QCOMPARE(getObjectValue(summary, "size"_L1).toInt(), 2);
                          });
    QTRY_COMPARE_EQ(done, true);

    // Cookies are empty, because the empty optional parameter is not being sent at all
    // so nothing to send back from the server
    done = true;
    severalExplodeCookies(OptionalParameter<QMap<QString, QString>>(),
                          OptionalParameter<qint64>(),
                          this, [&](const QRestReply &reply, const QString &) {
                              done = reply.isSuccess();
                          });
    QTRY_COMPARE_EQ(done, false);
}

void OperationParameters::cookieInvalidStyle()
{
    const QString stringValue = "Just some random string"_L1;
    const QString expectedCookie = "stringParameter=Just some random string"_L1;
    bool done = false;

#ifdef Q_OS_WIN
    const char *warningMsg = "'simple' style is invalid for cookie parameters.\r\n"
                             "Falling back to the default style 'form'.";
#else
    const char *warningMsg = "'simple' style is invalid for cookie parameters.\n"
                             "Falling back to the default style 'form'.";
#endif
    QTest::ignoreMessage(QtWarningMsg, warningMsg);
    cookieNotExplodeStringInvalidStyle(stringValue, this,
        [&](const QRestReply &reply, const QString &summary) {
            done = reply.isSuccess();
            QCOMPARE(getHeaderValue(summary, "Cookie"_L1), expectedCookie);
            QVERIFY(getObjectValue(summary, "error"_L1).toString().isEmpty());
        });
    QTRY_COMPARE_EQ(done, true);
}

void OperationParameters::cleanupTestCase()
{
    if (serverProcess.state() == QProcess::ProcessState::Running) {
        serverProcess.kill();
        serverProcess.waitForFinished();
    }
}

} // QtOpenAPI

QTEST_MAIN(QtOpenAPI::OperationParameters)
#include "tst_operationparameters.moc"
