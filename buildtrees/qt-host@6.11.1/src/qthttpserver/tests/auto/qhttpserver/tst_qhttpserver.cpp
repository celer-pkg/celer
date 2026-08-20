// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtHttpServer/qhttpserver.h>
#include <QtHttpServer/qhttpserverrequest.h>
#include <QtHttpServer/qhttpserverrouterrule.h>

#include <QtTest/qtest.h>
#include <QtTest/qsignalspy.h>

#if QT_CONFIG(concurrent)
#include <QtConcurrent/qtconcurrentrun.h>
#endif

#include <QtCore/qurl.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qsemaphore.h>
#include <QtCore/qset.h>
#include <QtCore/qtimer.h>

#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qtcpserver.h>

#if QT_CONFIG(ssl)
#include <QtNetwork/qsslconfiguration.h>
#include <QtNetwork/qsslsocket.h>
#include <QtNetwork/qsslkey.h>
#include <QtNetwork/qsslserver.h>
#endif

#if QT_CONFIG(localserver)
#include <qrandom.h>
#include <QtNetwork/qlocalserver.h>
#include <QtNetwork/qlocalsocket.h>
#endif

#include <QtTest/private/qtesthelpers_p.h>

#include <array>

QT_BEGIN_NAMESPACE

#if QT_CONFIG(ssl)

static const char g_privateKey[] = R"(-----BEGIN RSA PRIVATE KEY-----
MIIJKAIBAAKCAgEAvdrtZtVquwiG12+vd3OjRVibdK2Ob73DOOWgb5rIgQ+B2Uzc
OFa0xsiRyc/bam9CEEqgn5YHSn95LJHvN3dbsA8vrFqIXTkisFAuHJqsmsYZbAIi
CX8t1tlcUmQsJmjZ1IKhk37lgGMKkc28Oh/CHbTrhJZWdQyoBbNb8KeqSHkePYu0
/BMtO/lrkXJjV6BXgK8BgOqJfOqrGCsBvW+sqZz9D51ZBBVb3YCrBZP20NVA5xZU
qOFwS3jn+9hO1XlJcwiFA3VzU7uSVt2zjzhX0iHw6GOVbjR4IStqH/y0oa9R9mQa
0hmzQ7LcV9NighX5kM8PsgT9i6Xhv2nmsjpPreLYgXoXqpDRrL0PR0CSts2ucRdf
hMhY8ViNoarZ12Z2CTaNxiHPGzNYNJPaQG40o3LEbQ3GP7igZ8go/ffSV+kZJS5j
uAHCsUvNUA4gvFfVXLxzoG6qewLXSCXoqDyJ9T7g4L81W19hsBxVp8gDqVAiBnpg
+GTLaC69WOm9OMXEROTOlin7gxlQ0pZO+2/M3uFFo/hXlIH/Mb5NPKlpNBqgLpqI
wtGMugt7Dx9JoMwWvEBlzMgeycYmNXwSHsdQ5kxFS5uYuZEri62Xrk/WWlev/PDC
RdcSUhrjVSNotFQveGKSwC5z2FOAIZioA0mPxsBixSaQY8fhiaC5ydUw4F0CAwEA
AQKCAgB5M4AG/Aus5x6d/hC4YzxCEvT7IakisLQmaIFpfhiuO6YbgTO9S60Qkg5w
FZ/vbKNyHxI3juGMr6A90dQzRqFj3e4DS7BuQwFgKW+mlx/Flt231AzCn0w2MoD7
oDOHObyGK/bWYFZHBfNDbWHSgV+88zi/ZfI/uxqwuPXixkaxCZFCnSOnIN7pwKrp
KWs+D4CNCCwfjprDAlTDkwEDXH2PskbjZwHi13fUCkYjw3f3jYxnehwFzBWSONdw
MYDySwGWzEOOF7bOJ5qeld4BemimH0DaOmi0+A4QrtSLIxp1daUPdIyiwAFvIIoG
D0592WV/CpDshr8OHZHmTscV1J/0OTNa3Pr5K9L24mSIf2Zd85X9nl3qLbYPqdCJ
1lQUYOiPO0us58y6V1vS6CWK1J3fVMCcmIUDHoAelHPKrgU9tHjCTj0Dk3LYz/hm
oK9I4OE0TKfWkUgSogB753sR/0ssnTeIFy9RAEPZXlJ9EGiNU3f8ZnuoAOi6pFWi
OO80K1sAhuDjX67O6OoqFMCWJTd1oXjLqjbLBsVeGH5kiZHZVqdAAtISV7f8jAQR
wEc2OgDJ6e38HYgwtqtR3Vkv7tVXfWx0Z9SYqtJWQv+CAwoPUvD+Bhok4iW2k1U7
Fq4iVHMl1n4ljZBgkHCl9Y8+h1qo5f+PgjsKblaiPS8EUCL8yQKCAQEA9I8/vpsu
8H/je7kLUlikkKiKDydU1tt/QRH33x5ZdCIHkXvGkd5GhGyZ8sngHJkOlteGBcVx
2kZC+4c3bKn5Pke38U+W8Xw2ZUm3zTn11Trlg2EhTdl+UTW/BBFt8o/hHYLW3nuT
y+VO3uZYtghGwYBwAeuYBRYRFpnZS9n0yMOwt9jCbqjSpL+QnY4HFcY3NWBE2MFg
JerVtpSEZFCeYksUU3IOCU0Ol0IjfmMy9XjEkkmeb4E7OFjHH1F7VaHT2ZlhhHzf
TKYvHWotFS621oDl8LBtD/8ZS0cYNpVfoJbKDhNMMAZlGXq6fDwj9d76SU70BMc+
PacThaAAY7ke/wKCAQEAxryPThH3GXvhIoakEtlS+dnNgIsOuGZYQj2CniKjmIv7
D9OGEd7UC+BxDtVMiq4Sq7vYeUcJ1g9EW1hwwjQIswbW5IGoUuUHpBe9jB1h88Cg
uMWGvtNJzZM0t4arlUrouIz8jxE6mcIysvRAIoFT+D8fzITOIVDx7l6qDbT51jbB
d886V1cN8/FdyEa08w+ChkAR/s+57KQMjBsUeAPAMac2ocgYsSE1YoXcMdZYfQfy
QSJZOt0hTYrOUFlrBBmTGRRv/kKbNeDnr2jjWPRzzupuOUejOUki/z2Ts/lY3vtv
8dA1kjwR/kgVXK+xa3LsZsYlu3myEashT+YMj1HcowKCAQEAinoWeSI7yPhRYfwc
egsxW6vjSMNXmbV97+VxukfgFQ8zw+AXRv9aZJ9t6HkAypCsHyN4gwoS9qp0QSKG
cqQoOmi3sg8EBEb2MhI03iMknRGVZff4uLEfgnJxb6dC32cy69frPN0yifCU4UgD
EUfMcML+KUgysyaUlHyW+wk2Pvv3s5IsPiaf56OFCoGiZ2TuW+3f7fBJNg8r5g9g
i8DOfg/POZTKd9/HFETh/i3DbBVvEPpYmQDO/I/gaE5mDM3uPDdKbY+bjTZIVVqK
noTuCLXB/bCYgMdMlkByaG3aUP8w+BlbOZJVasEAmVogbpdMl3f6Wj5LcvOI7U/1
CIKJFwKCAQALXyK8Dt8awDHjrdyZj4Hl9gaCQnK3LnQCZk6hCc5enjPhdfMH9r4f
Z9pQRPg6PzemR/tdBSmU7A63Q1pAYoXU6KFHNfwRsjU7uHgKGmxObElGCVdqd+CT
OMcdcUFEK6MhXD/fV9cIkUohX0SENO4/GC2ToE3DLkSJpTUJz78z+LIdTuhBsyOD
P95j5VfZSJvpXqUo9W3oEoL9SVdkfqJytOS1YSO4jvPlDU/KMj+h9+Buxa5hZeHP
9A9WHae39laqarb1z43eCV54dQH9Rw+RWWyxLl4ymvK7tCRNegkRyUVgis9l7LYC
3NEMGqmGQm8wekoSbiY4SJiBX+J8GO0NAoIBAE5nwz0iU4+ZFbuknqI76MVkL6xC
llcZHCOpZZIpXTZmCqWySQycqFO3U8BxD2DTxsNAKH0YsnaihHyNgp1g5fzFnPb8
HlVuHhCfJN5Ywo1gfCNHaRJIMYgjPAD+ewTDSowbzH2HlpUt5NOQJWuiZfxPDJll
qmRAqZ3fyf8AP7pXxj5p0y8AUPtkmjk7h8hxstbvcmQvtTDzgkqeBYwZhEtGemdY
OCi7UuXYjRwDfnka2nAdB9lv4ExvU5lkrJVZXONYUwToArAxRtdKMqCfl36JILMA
C4+9sOeTo6HtZRvPVNLMX/rkWIv+onFgblfb8guA2wz1JUT00fNxQPt1k8s=
-----END RSA PRIVATE KEY-----)";

static const char g_certificate[] = R"(-----BEGIN CERTIFICATE-----
MIIFszCCA5ugAwIBAgIUfpP54qSLfus/pFUIBDizbnrDjE4wDQYJKoZIhvcNAQEL
BQAwaDELMAkGA1UEBhMCRlIxDzANBgNVBAgMBkZyYW5jZTERMA8GA1UEBwwIR3Jl
bm9ibGUxFjAUBgNVBAoMDVF0Q29udHJpYnV0b3IxHTAbBgNVBAMMFHFodHRwc3Nl
cnZlcnRlc3QuY29tMCAXDTIyMDIwNzE0MzE0NVoYDzIyNjgwNzA3MTQzMTQ1WjBo
MQswCQYDVQQGEwJGUjEPMA0GA1UECAwGRnJhbmNlMREwDwYDVQQHDAhHcmVub2Js
ZTEWMBQGA1UECgwNUXRDb250cmlidXRvcjEdMBsGA1UEAwwUcWh0dHBzc2VydmVy
dGVzdC5jb20wggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQC92u1m1Wq7
CIbXb693c6NFWJt0rY5vvcM45aBvmsiBD4HZTNw4VrTGyJHJz9tqb0IQSqCflgdK
f3kske83d1uwDy+sWohdOSKwUC4cmqyaxhlsAiIJfy3W2VxSZCwmaNnUgqGTfuWA
YwqRzbw6H8IdtOuEllZ1DKgFs1vwp6pIeR49i7T8Ey07+WuRcmNXoFeArwGA6ol8
6qsYKwG9b6ypnP0PnVkEFVvdgKsFk/bQ1UDnFlSo4XBLeOf72E7VeUlzCIUDdXNT
u5JW3bOPOFfSIfDoY5VuNHghK2of/LShr1H2ZBrSGbNDstxX02KCFfmQzw+yBP2L
peG/aeayOk+t4tiBeheqkNGsvQ9HQJK2za5xF1+EyFjxWI2hqtnXZnYJNo3GIc8b
M1g0k9pAbjSjcsRtDcY/uKBnyCj999JX6RklLmO4AcKxS81QDiC8V9VcvHOgbqp7
AtdIJeioPIn1PuDgvzVbX2GwHFWnyAOpUCIGemD4ZMtoLr1Y6b04xcRE5M6WKfuD
GVDSlk77b8ze4UWj+FeUgf8xvk08qWk0GqAumojC0Yy6C3sPH0mgzBa8QGXMyB7J
xiY1fBIex1DmTEVLm5i5kSuLrZeuT9ZaV6/88MJF1xJSGuNVI2i0VC94YpLALnPY
U4AhmKgDSY/GwGLFJpBjx+GJoLnJ1TDgXQIDAQABo1MwUTAdBgNVHQ4EFgQUK7Un
0JA3DBUVhclrm6pIZsO60U4wHwYDVR0jBBgwFoAUK7Un0JA3DBUVhclrm6pIZsO6
0U4wDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAgEAuvSFAgfgurDT
/dbXuJ0O+FuGg4NOTNRil5ji3DnMzReIHpSiKiXu76PHHRFnlosvfAFOHlpYntun
LhbUAxim/iIgWZR33uzvqXMXBORZ0zffjy2SjGCW8ZJYyTmg9c0tc0jEjv7owtlU
m6tUXMOs9U0CzvEKLt0K0dMALaLkXtscuzEWA4PHVvnvTu0Wyjj/8n+DgYzY09kC
YF0lJfcG6bddDgspmYyFpULeGGP7+qwgGh4cVBtY5I4Htr3p7hDo6UGDF6AsMQZF
1CAEgBVRbJgI2GTnptpm9k3EFKwQ81z5O+NnP3ZsuuZ3CEVaPHyQf/POLAIhmZLt
0vS9qoRiS4uMUJDXz2kJFBOFHki073eMvHiKtlpYOlJXMQ4MkHCydjeeuhHcgUCq
ZDWuQMmq/8tMwf4YtvxYtXzAMVW9dM8BgWu2G8/JwPMGUGhLfKkHmc8dlQzGDe/W
K/uVHlJZNF4Y0eXVlq9DUhpvKOjGc8A208wQlsTUgPxljgJ2+4F3D+t0luc3h65m
25iw8eRGuYDoCQLG7u7MI0g8A0H+0h9Xrt8PQql86vmQhmTUhKfedVGOo2t2Bcfn
ignL7f4e1m2jh0oWTLhuP1hnVFN4KAKpVIJXhbEkH59cLCN6ARXiEHCM9rmK5Rgk
NQZlAZc2w1Ha9lqisaWWpt42QVhQM64=
-----END CERTIFICATE-----)";

#endif // QT_CONFIG(ssl)

using namespace Qt::StringLiterals;
using namespace std::chrono_literals;

using RouterHandler = std::function<void(const QRegularExpressionMatch &,
                                         const QHttpServerRequest &, QHttpServerResponder &)>;

class QueryRequireRouterRule : public QHttpServerRouterRule
{
public:
    QueryRequireRouterRule(const QString &pathPattern,
                           const QObject *context,
                           RouterHandler &&routerHandler)
        : QHttpServerRouterRule(pathPattern, context, std::move(routerHandler))
    {
    }

    void setKey(const QString &key) { m_queryKey = key; }

    bool matches(const QHttpServerRequest &request, QRegularExpressionMatch *match) const override
    {
        if (QHttpServerRouterRule::matches(request, match)) {
            if (request.query().hasQueryItem(m_queryKey))
                return true;
        }

        return false;
    }

private:
    QString m_queryKey;
};

class ReplyObject : public QObject
{
    Q_OBJECT
public:

    void replyRespReq(QHttpServerResponder &resp,
               const QHttpServerRequest &req)
    {
        resp.write(req.body(), "text/plain"_ba);
    }

    void replyReqResp(const QHttpServerRequest &req,
                QHttpServerResponder &resp)
    {
        resp.write(req.body(), "text/plain"_ba);
    }

    QString replyNoArgs()
    {
        return "ReplyAsHtml::replyNoArgs";
    }

    void replyRespReqConst(QHttpServerResponder &resp,
               const QHttpServerRequest &req) const
    {
        resp.write(req.body(), "text/plain"_ba);
    }

    void replyReqRespConst(const QHttpServerRequest &req,
                QHttpServerResponder &resp) const
    {
        resp.write(req.body(), "text/plain"_ba);
    }

    QString replyNoArgsConst() const
    {
        return "ReplyAsHtml::replyNoArgs";
    }
};

class tst_QHttpServer final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase_data();
    void initTestCase();
    void init();
    void routeGet_data();
    void routeGet();
    void routeKeepAlive();
    void routePost_data();
    void routePost();
    void routeDelete_data();
    void routeDelete();
    void routeExtraHeaders();
    void getLongChunks();
    void invalidRouterArguments();
    void checkRouteLambdaCapture();
    void afterRequest();
    void disconnectedInEventLoop();
    void multipleRequests();
    void pipelinedRequests();
    void missingHandler();
    void pipelinedFutureRequests();
    void requestNotOverwritten();
    void parallelFutureRequests();
    void slowReader();
    void concurrentRequestBack();
    void concurrentMultipart();
    void multipleResponses();
    void contextObjectInOtherThreadWarning();
    void keepAliveTimeout();
    void maximumUrlSizeAllowed();
    void maximumTotalHeaderSizeAllowed();
    void maximumHeaderFieldSizeAllowed();
    void maximumNumberOfHeaderFieldsAllowed();
    void maximumBodySizeAllowed();
    void writeSequentialDevice();
    void writeMuchToSequentialDevice();
    void writeFromEmptySequentialDevice();
    void concurrentRequestsToSequentialDevice();
    void timeoutConnection();
    void useCanceledResponders();

#if QT_CONFIG(localserver)
    void localSocket();
#endif

private:
    void checkReply(QNetworkReply *reply, const QString &response);

private:
    QHttpServer httpserver;
    QString clearUrlBase;
    QString sslUrlBase;
    std::array<QNetworkAccessManager, 10> networkAccessManagers;
    QNetworkAccessManager &networkAccessManager = networkAccessManagers[0];
    ReplyObject replyObject;
    std::vector<std::unique_ptr<QHttpServerResponder>> responders;
    std::optional<QSemaphore> readySem, routeSem;
    std::optional<bool> canceledByClient;
};

struct CustomArg {
    int data = 10;

    CustomArg() {} ;
    CustomArg(const QString &urlArg) : data(urlArg.toInt()) {}
};

static void reqAndRespHandler(QHttpServerResponder &resp, const QHttpServerRequest &req)
{
    resp.write(req.body(), "text/html"_ba);
}

static void respHandler(QHttpServerResponder &responder)
{
    responder.write("test msg", "text/html"_ba);
}

static void respRvalueHandler(QHttpServerResponder &&responder)
{
    responder.write("test msg", "text/html"_ba);
}

class MessageHandler {
public:
    explicit MessageHandler(const char *message) : message(message) {}

    const char *operator()() const {
        return message;
    }
private:
    const char *message;
};

void tst_QHttpServer::initTestCase_data()
{
    QTest::addColumn<bool>("useSsl");
    QTest::addColumn<bool>("useHttp2");

    QTest::newRow("http/1.1") << false << false;

#if QT_CONFIG(ssl)
    if (QSslSocket::supportsSsl()) {
        QTest::newRow("https/1.1") << true << false;
        if (QSslSocket::supportedFeatures().contains(QSsl::SupportedFeature::ServerSideAlpn))
            QTest::newRow("http/2") << true << true;
    }
#endif
}

class SequentialIODevice : public QIODevice
{
    Q_OBJECT

public:
    SequentialIODevice() : SequentialIODevice(QByteArray(), 0, 1, 50ms) { }

    SequentialIODevice(const QByteArray &data, int times, int repetitions,
                       std::chrono::milliseconds readInterval)
        : message(data.repeated(repetitions)), times(times)
    {
        setOpenMode(QIODeviceBase::ReadWrite);
        timer = new QTimer(this);
        timer->callOnTimeout(this, &SequentialIODevice::onTimeout);
        timer->setSingleShot(false);
        timer->setInterval(readInterval);
        timer->start();
    }

    bool isSequential() const override { return true; }
    qint64 bytesAvailable() const override { return buffer.size() - readPos; }
    qint64 bytesToWrite() const override { return 0; }
    qint64 pos() const override { return readPos; }

    bool seek(qint64 pos) override
    {
        Q_UNUSED(pos);
        return false; // No random accesss on sequential devices
    }

    qint64 readData(char *data, qint64 maxSize) override
    {
        qint64 length = qMin(maxSize, buffer.size() - readPos);
        if (length == 0)
            return finishedReading ? -1 : 0;
        memcpy(data, buffer.constData() + readPos, length);
        readPos += length;

        if (readPos == buffer.size()) {
            readPos = 0;
            buffer.clear();
        }

        return length;
    }

    qint64 writeData(const char *data, qint64 maxSize) override
    {
        buffer.append(data, maxSize);
        emit bytesWritten(maxSize);
        emit readyRead();
        return maxSize;
    }

    void onTimeout()
    {
        if (times > 0)
            write(message);

        if (--times <= 0) {
            timer->stop();
            finishedReading = true;
            emit readChannelFinished();
        }
    }

private:
    QTimer *timer;
    QByteArray message;
    QByteArray buffer;
    qsizetype readPos = 0;
    int times;
    bool finishedReading = false;
};

void tst_QHttpServer::initTestCase()
{
    httpserver.route("/resp-and-req-from-object", &replyObject, &ReplyObject::replyRespReq);
    httpserver.route("/req-and-resp-from-object", &replyObject, &ReplyObject::replyReqResp);
    httpserver.route("/qstring-from-object", &replyObject, &ReplyObject::replyNoArgs);
    httpserver.route("/resp-and-req-from-const-object", &replyObject, &ReplyObject::replyRespReqConst);
    httpserver.route("/req-and-resp-from-const-object", &replyObject, &ReplyObject::replyReqRespConst);
    httpserver.route("/qstring-from-const-object", &replyObject, &ReplyObject::replyNoArgsConst);

    ReplyObject *tempReplyObject = new ReplyObject();
    httpserver.route("/deleted-route", tempReplyObject, &ReplyObject::replyRespReq);
    delete tempReplyObject;

    httpserver.route("/req-and-resp", this, reqAndRespHandler);

    httpserver.route("/resp-and-req", this,
                     [](QHttpServerResponder &resp,
                        const QHttpServerRequest &req) {
        resp.write(req.body(), "text/html"_ba);
    });

    auto respHandlerPtr = respHandler;
    httpserver.route("/resp", this, respHandlerPtr);

    auto respRvalueHandlerPtr = respRvalueHandler;
    httpserver.route("/resp2", this, respRvalueHandlerPtr);

    auto l = []() -> QString { return "Hello world get"; };

    httpserver.route("/", QHttpServerRequest::Method::Get, this, l);

    httpserver.route("/", QHttpServerRequest::Method::Post, this, MessageHandler("Hello world post"));

    httpserver.route("/post-and-get",
                     QHttpServerRequest::Method::Get | QHttpServerRequest::Method::Post,
                     this,
                     [](const QHttpServerRequest &request) {
                         if (request.method() == QHttpServerRequest::Method::Get)
                             return "Hello world get";
                         else if (request.method() == QHttpServerRequest::Method::Post)
                             return "Hello world post";

                         return "This should not work";
                     });

    httpserver.route(
            "/any", QHttpServerRequest::Method::AnyKnown, this, [](const QHttpServerRequest &request) {
                static const auto metaEnum = QMetaEnum::fromType<QHttpServerRequest::Method>();
                return metaEnum.valueToKey(static_cast<int>(request.method()));
            });

    httpserver.route("/page/", this, [] (const qint32 number) {
        return QString("page: %1").arg(number);
    });

    httpserver.route("/page/<arg>/detail", this, [] (const quint32 number) {
        return QString("page: %1 detail").arg(number);
    });

    httpserver.route("/user/", this, [] (const QString &name) {
        return QString("%1").arg(name);
    });

    httpserver.route("/user/<arg>/", this, [] (const QString &name, const QByteArray &ba) {
        return QString("%1-%2").arg(name).arg(QString::fromLatin1(ba));
    });

    httpserver.route("/test/", this, [] (const QUrl &url) {
        return QString("path: %1").arg(url.path());
    });

    httpserver.route("/api/v", this, [] (const float api) {
        return QString("api %1v").arg(api);
    });

    httpserver.route("/api/v<arg>/user/", this, [] (const float api, const quint64 user) {
        return QString("api %1v, user id - %2").arg(api).arg(user);
    });

    httpserver.route("/api/v<arg>/user/<arg>/settings", this, [] (const float api, const quint64 user,
                                                             const QHttpServerRequest &request) {
        const auto &role = request.query().queryItemValue(QString::fromLatin1("role"));
        const auto &fragment = request.url().fragment();

        return QString("api %1v, user id - %2, set settings role=%3#'%4'")
                   .arg(api).arg(user).arg(role, fragment);
    });

    auto route = httpserver.route<QueryRequireRouterRule>(
            "/custom/",
            this,
            [] (const quint64 num, const QHttpServerRequest &request) {
        return QString("Custom router rule: %1, key=%2")
                    .arg(num)
                    .arg(request.query().queryItemValue("key"));
    });
    QVERIFY(route);
    if (route)
        route->setKey(QLatin1String("key"));

    httpserver.router()->addConverter<CustomArg>("[+-]?\\d+"_L1);
    httpserver.route("/check-custom-type/", this, [] (const CustomArg &customArg) {
        return QString("data = %1").arg(customArg.data);
    });

    httpserver.route("/post-body", QHttpServerRequest::Method::Post, this,
                     [](const QHttpServerRequest &request) { return request.body(); });

    httpserver.route("/file/", this, [] (const QString &file) {
        return QHttpServerResponse::fromFile(QFINDTESTDATA("data/"_L1 + file));
    });

    httpserver.route("/json-object/", this, [] () {
        return QJsonObject{
            {"property", "test"},
            {"value", 1}
        };
    });

    httpserver.route("/json-array/", this, [] () {
        return QJsonArray{
            1, "2",
            QJsonObject{
                {"name", "test"}
            }
        };
    });

    httpserver.route("/data-and-custom-status-code/", this, []() {
        return QHttpServerResponse(QJsonObject{ { "key", "value" } },
                                   QHttpServerResponder::StatusCode::Accepted);
    });

    httpserver.route("/chunked/", this, [](QHttpServerResponder &responder) {
        responder.writeBeginChunked("text/plain", QHttpServerResponder::StatusCode::Ok);
        responder.writeChunk("part 1 of the message, ");
        responder.writeEndChunked("part 2 of the message");
    });

    httpserver.route("/longChunks/", this, [](QHttpServerResponder &responder) {
        responder.writeBeginChunked("text/plain", QHttpServerResponder::StatusCode::Ok);
        constexpr qsizetype chunkLength = 1024 * 1024;
        QByteArray a(chunkLength, 'a');
        QByteArray b(chunkLength, 'b');
        QByteArray c(chunkLength, 'c');
        responder.writeChunk(a);
        responder.writeChunk(b);
        responder.writeEndChunked(c);
    });

    httpserver.route("/extra-headers", this, [] () {
        QHttpServerResponse resp("");
        auto h = resp.headers();

        h.removeAll(QHttpHeaders::WellKnownHeader::ContentType);
        h.append(QHttpHeaders::WellKnownHeader::ContentType, "application/x-empty");

        h.removeAll(QHttpHeaders::WellKnownHeader::Server);
        h.append(QHttpHeaders::WellKnownHeader::Server, "test server");

        resp.setHeaders(std::move(h));
        return resp;
    });

    httpserver.route("/processing", this, [](QHttpServerResponder &responder) {
        responder.sendResponse(QHttpServerResponse(QHttpServerResponder::StatusCode::Processing));
        responder.sendResponse(QHttpServerResponse("done"));
    });

    httpserver.addAfterRequestHandler(this, [] (const QHttpServerRequest &, QHttpServerResponse &) {

    });

#if QT_CONFIG(concurrent)
    httpserver.route("/future/", this, [] (int id) {
        return QtConcurrent::run([id] () -> QHttpServerResponse {
             if (id == 0)
                return QHttpServerResponse::StatusCode::NotFound;

            QTest::qSleep(500);
            return QHttpServerResponse("future is coming");
        });
    });

    httpserver.route("/wait/<arg>", this, [](qsizetype wait) {
        return QtConcurrent::run([wait] () -> QHttpServerResponse {
            QThread::msleep(wait);
            return QString::number(wait);
        });
    });

    httpserver.route("/user/<arg>/delayed/", this, [](const QString &user, unsigned long delayMs) {
        return QtConcurrent::run([user, delayMs]() -> QHttpServerResponse {
            QThread::msleep(delayMs);
            return user;
        });
    });

    httpserver.route("/wait-and-return-body/", [this](const QHttpServerRequest &request) {
        auto body = request.body();
        return QtConcurrent::run([body, this]() {
            Q_ASSERT(readySem);
            readySem->release();
            Q_ASSERT(routeSem);
            routeSem->acquire();
            return QHttpServerResponse("text/plain", body);
        });
    });

    httpserver.addAfterRequestHandler(
            this, [](const QHttpServerRequest &request, QHttpServerResponse &response) {
                if (request.url().path().startsWith("/wait-and-return-body/")) {
                    auto h = response.headers();
                    h.append("X-REQUEST-BODY-SIZE", QString::number(request.body().size()));
                    response.setHeaders(std::move(h));
                }
            });

    httpserver.route("/sleepingfuture/<arg>", this, [](int ms, QHttpServerResponder &&responder) {
        // Use shared ptr to responder to pass it by value instead of moving the responder into
        // the lambda, which would force the lambda to be mutable.
        return QtConcurrent::run(
                [=, r = std::make_shared<QHttpServerResponder>(std::move(responder))]() {
                    QThread::msleep(ms);
                    QHttpHeaders headers;
                    headers.append(QHttpHeaders::WellKnownHeader::ContentType, "text/plain"_L1);
                    r->write(u"%1"_s.arg(ms).toUtf8(), headers);
                });
    });

    httpserver.route(
            "/slowreader/<arg>/<arg>", this,
            [](const QString message, int repeats, QHttpServerResponder &&responder) {
                return QtConcurrent::run(
                        [=, r = std::make_shared<QHttpServerResponder>(std::move(responder))] {
                            using namespace std::chrono_literals;
                            QByteArray msg = message.toUtf8();
                            QHttpHeaders headers;
                            headers.append(QHttpHeaders::WellKnownHeader::ContentType,
                                           "text/plain"_L1);
                            auto reader = new SequentialIODevice(msg, repeats, 1, 100ms);
                            r->write(reader, headers);
                        });
            });

    httpserver.route("/concurrent-request-back", this,
                     [](QHttpServerRequest request, QHttpServerResponder &&responder) {
                         return QtConcurrent::run([request,
                                                   r = std::make_shared<QHttpServerResponder>(
                                                           std::move(responder))] {
                             r->write(request.body(), "text/plain"_ba);
                         });
                     });

httpserver.route("/concurrent-request-back2", this,
                     [](QHttpServerResponder &&responder, QHttpServerRequest request) {
                         return QtConcurrent::run([request,
                                                   r = std::make_shared<QHttpServerResponder>(
                                                           std::move(responder))] {
                             r->write(request.body(), "text/plain"_ba);
                         });
                     });

    httpserver.route("/concurrent-body-back-delay/<arg>", this,
                     [](int ms, QHttpServerRequest request) {
                         return QtConcurrent::run([=]() -> QHttpServerResponse {
                             QThread::msleep(ms);
                             return request.body();
                         });
                     });

    httpserver.route(
            "/concurrent-multipart-back/<arg>/<arg>/<arg>", this,
            [](int times, int ms, QString message, QHttpServerResponder &&responder) {
                return QtConcurrent::run(
                        [=, r = std::make_shared<QHttpServerResponder>(std::move(responder))] {
                            if (times > 0) {
                                QByteArray ba = message.toUtf8();
                                r->writeBeginChunked("text/plain"_ba);
                                for (int i = 1; i < times; ++i) {
                                    r->writeChunk(ba);
                                    QThread::msleep(ms);
                                }
                                r->writeEndChunked(ba);
                            } else {
                                r->write();
                            }
                        });
            });

    httpserver.route("/timeout-connection", this, [this](QHttpServerResponder &&responder) {
        return QtConcurrent::run(
                [this, r = std::make_shared<QHttpServerResponder>(std::move(responder))] {
                    r->writeBeginChunked("text/plain"_ba);
                    for (int i = 1; i < 200; ++i) {
                        if (r->isResponseCanceled()) {
                            canceledByClient.emplace(true);
                            return;
                        }
                        QThread::sleep(10ms);
                    }
                    canceledByClient.emplace(false);
                    r->writeEndChunked("chunk");
                });
    });

    httpserver.route("/add-responder", this, [this](QHttpServerResponder &&responder) {
        responder.writeBeginChunked("text/plain"_ba);
        responder.writeChunk("chunk");
        responders.emplace_back(std::make_unique<QHttpServerResponder>(std::move(responder)));
    });
#endif

#if QT_CONFIG(localserver)
    const auto serverName =
            QStringLiteral("testserver") + QString::number(qApp->applicationPid());

    std::unique_ptr<QLocalServer> localServer = std::make_unique<QLocalServer>();
    if (localServer->listen(serverName))
        httpserver.bind(localServer.release());
    else
        qCritical("Local Socket server listen failed");
#endif

    auto tcpserver = std::make_unique<QTcpServer>();
    QVERIFY2(tcpserver->listen(), "HTTP server listen failed");
    quint16 port = tcpserver->serverPort();
    QVERIFY2(httpserver.bind(tcpserver.get()), "HTTP server bind failed");
    tcpserver.release();

    clearUrlBase = QStringLiteral("http://localhost:%1%2").arg(port);

#if QT_CONFIG(ssl)
    if (QSslSocket::supportsSsl()) {
        auto sslserver = std::make_unique<QSslServer>();
        QSslConfiguration serverConfig = QSslConfiguration::defaultConfiguration();
        serverConfig.setLocalCertificate(QSslCertificate(g_certificate));
        serverConfig.setPrivateKey(QSslKey(g_privateKey, QSsl::Rsa));
        serverConfig.setAllowedNextProtocols({QSslConfiguration::ALPNProtocolHTTP2});
        sslserver->setSslConfiguration(serverConfig);
        QVERIFY2(sslserver->listen(), "HTTPS server listen failed");
        port = sslserver->serverPort();
        QVERIFY2(httpserver.bind(sslserver.get()), "HTTPS server bind failed");
        sslserver.release();

        sslUrlBase = QStringLiteral("https://localhost:%1%2").arg(port);

        const QList<QSslError> expectedSslErrors = {
            QSslError(QSslError::SelfSignedCertificate, QSslCertificate(g_certificate)),
            // Non-OpenSSL backends are not able to report a specific error code
            // for self-signed certificates.
            QSslError(QSslError::CertificateUntrusted, QSslCertificate(g_certificate)),
            QSslError(QSslError::HostNameMismatch, QSslCertificate(g_certificate)),
        };

        for (size_t i = 0; i < networkAccessManagers.size(); ++i) {
            connect(&networkAccessManagers[i], &QNetworkAccessManager::sslErrors, this,
                    [expectedSslErrors](QNetworkReply *reply, const QList<QSslError> &errors) {
                        for (const auto &error : errors) {
                            if (!expectedSslErrors.contains(error)) {
                                qCritical() << "Got unexpected ssl error:" << error
                                            << error.certificate();
                            }
                        }
                        reply->ignoreSslErrors(expectedSslErrors);
                    });
        }
    }
#endif
    httpserver.route("/sequential-iodevice/<arg>/<arg>/<arg>", this,
                     [](QString message, int times, int repeats, QHttpServerResponder &responder) {
                         auto device = new SequentialIODevice(message.toUtf8(), times, repeats,
                                                              50ms);
                         responder.write(device, "text/plain");
                     });

    httpserver.route("/empty-sequential-iodevice/", this, [](QHttpServerResponder &responder) {
        auto device = new SequentialIODevice;
        responder.write(device, "text/plain");
    });

#ifdef COMPILE_ROUTES_THAT_STATIC_ASSERT
    // Every one of the QHttpServer::route() calls below static_assert
    // because they should not be supported. To see the static_assert messages,
    // thereby making compilation fail, define COMPILE_ROUTES_THAT_STATIC_ASSERT.
    // The comments in the lambdas are the first resulting static_assert messages.

    httpserver.route("/neither-responder-nor-response/", this, []() {
        // Handlers without responder argument must have return value
        return;
    });

    httpserver.route("/simple-and-special-switched-order/<arg>", this,
                     [](QHttpServerRequest request, int ms) {
                         // Request or responder must be the last argument
                         Q_UNUSED(ms);
                         Q_UNUSED(request);
                         return QHttpServerResponse::StatusCode::InternalServerError;
                     });

    httpserver.route("/simple-and-special-switched-order2/<arg>/<arg>", this,
                     [](QHttpServerRequest request, int ms, int repeats) {
                         // QHttpServerRequest or QHttpServerResponder can only be one of the two
                         // last arguments
                         Q_UNUSED(ms);
                         Q_UNUSED(repeats);
                         Q_UNUSED(request);
                         return QHttpServerResponse::StatusCode::InternalServerError;
                     });

    httpserver.route("/two-requests/", this,
                     [](QHttpServerRequest request1, QHttpServerRequest request2) {
                         // Cannot have multiple request arguments
                         Q_UNUSED(request1);
                         Q_UNUSED(request2);
                         return QHttpServerResponse::StatusCode::InternalServerError;
                     });

    httpserver.route("/two-responders/", this,
                     [](QHttpServerResponder &responder1, QHttpServerResponder &responder2) {
                         // Cannot have multiple responder arguments
                         Q_UNUSED(responder1);
                         Q_UNUSED(responder2);
                         return QHttpServerResponse::StatusCode::InternalServerError;
                     });

    httpserver.route("/three-specials/", this,
                     [](QHttpServerRequest request1, QHttpServerRequest request2,
                        QHttpServerResponder &&responder) {
                         // QHttpServerRequest or QHttpServerResponder can only be one of the two
                         // last arguments
                         Q_UNUSED(request1);
                         Q_UNUSED(request2);
                         responder.write(QHttpServerResponse::StatusCode::InternalServerError);
                     });

    httpserver.route("/responder-and-response/", this, [](QHttpServerResponder &responder) {
        // Depending on whether future support is enabled:
        // Either: Handlers with responder argument must have void or QFuture<void> return type
        // Or: Handlers with responder argument must have void return type
        Q_UNUSED(responder);
        return QHttpServerResponse::StatusCode::InternalServerError;
    });

#if QT_CONFIG(concurrent)
    httpserver.route("/future-request-const-ref/", this,
                     [](const QHttpServerRequest &request, QHttpServerResponder &&responder) {
                         // Request argument must be captured by value when returning QFuture<void>
                         return QtConcurrent::run([=, r = std::move(responder)]() mutable {
                             Q_UNUSED(request);
                             r.write(QHttpServerResponse::StatusCode::InternalServerError);
                         });
                     });

    httpserver.route("/responder-argument-returning-future-response/", this,
                     [](QHttpServerRequest request, QHttpServerResponder &&responder) {
                         // Handlers with responder argument must have void or QFuture<void> return
                         // type
                         Q_UNUSED(responder);
                         return QtConcurrent::run([request]() -> QHttpServerResponse {
                             Q_UNUSED(request);
                             return QHttpServerResponse::StatusCode::InternalServerError;
                         });
                     });

    httpserver.route("/future-responder-lvalue-ref/", this, [](QHttpServerResponder &responder) {
        // Responder argument must be captured as Rvalue reference when returning QFuture<void>
        return QtConcurrent::run([r = std::move(responder)]() mutable {
            r.write(QHttpServerResponse::StatusCode::InternalServerError);
        });
    });

    httpserver.route("/request-returning-future-void/", this, [](QHttpServerRequest request) {
        return QtConcurrent::run([=]() {
            // Handlers without responder argument must have return value
            Q_UNUSED(request);
            return;
        });
    });
#endif
#endif
}

void tst_QHttpServer::init()
{
    canceledByClient.reset();
    responders.clear();
#if QT_CONFIG(ssl)
    QFETCH_GLOBAL(const bool, useSsl);
    if (useSsl && QTestPrivate::isSecureTransportBlockingTest())
        QSKIP("SslServer is blocking the test execution while trying to access the login keychain");
#endif // QT_CONFIG(ssl)
    networkAccessManager.clearConnectionCache();
}

void tst_QHttpServer::routeGet_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<int>("code");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("body");

    QTest::addRow("hello world")
        << "/"
        << 200
        << "text/plain"
        << "Hello world get";

    QTest::addRow("test msg")
        << "/resp"
        << 200
        << "text/html"
        << "test msg";

    QTest::addRow("test msg2")
        << "/resp2"
        << 200
        << "text/html"
        << "test msg";

    QTest::addRow("not found")
        << "/not-found"
        << 404
        << "application/x-empty"
        << "";

    QTest::addRow("qstring-from-object")
        << "/qstring-from-object"
        << 200
        << "text/plain"
        << "ReplyAsHtml::replyNoArgs";

    QTest::addRow("qstring-from-const-object")
        << "/qstring-from-const-object"
        << 200
        << "text/plain"
        << "ReplyAsHtml::replyNoArgs";

    QTest::addRow("arg:int")
        << "/page/10"
        << 200
        << "text/plain"
        << "page: 10";

    QTest::addRow("arg:-int")
        << "/page/-10"
        << 200
        << "text/plain"
        << "page: -10";

    QTest::addRow("arg:uint")
        << "/page/10/detail"
        << 200
        << "text/plain"
        << "page: 10 detail";

    QTest::addRow("arg:-uint")
        << "/page/-10/detail"
        << 404
        << "application/x-empty"
        << "";

    QTest::addRow("arg:string")
        << "/user/test"
        << 200
        << "text/plain"
        << "test";

    QTest::addRow("arg:string,!a+.")
        << "/user/test test ,!a+."
        << 200
        << "text/plain"
        << "test test ,!a+.";

    QTest::addRow("arg:string,ba")
        << "/user/james/bond"
        << 200
        << "text/plain"
        << "james-bond";

    QTest::addRow("arg:url")
        << "/test/api/v0/cmds?val=1"
        << 200
        << "text/plain"
        << "path: api/v0/cmds";

    QTest::addRow("arg:float 5.1")
        << "/api/v5.1"
        << 200
        << "text/plain"
        << "api 5.1v";

    QTest::addRow("arg:float 5.")
        << "/api/v5."
        << 200
        << "text/plain"
        << "api 5v";

    QTest::addRow("arg:float 6.0")
        << "/api/v6.0"
        << 200
        << "text/plain"
        << "api 6v";

    QTest::addRow("arg:float,uint")
        << "/api/v5.1/user/10"
        << 200
        << "text/plain"
        << "api 5.1v, user id - 10";

    QTest::addRow("arg:float,uint,query")
        << "/api/v5.2/user/11/settings?role=admin"
        << 200
        << "text/plain"
        << "api 5.2v, user id - 11, set settings role=admin#''";

    // The fragment isn't actually sent via HTTP (it's information for the user agent)
    QTest::addRow("arg:float,uint, query+fragment")
        << "/api/v5.2/user/11/settings?role=admin#tag"
        << 200
        << "text/plain"
        << "api 5.2v, user id - 11, set settings role=admin#''";

    QTest::addRow("custom route rule")
        << "/custom/15"
        << 404
        << "application/x-empty"
        << "";

    QTest::addRow("custom route rule + query")
        << "/custom/10?key=11&g=1"
        << 200
        << "text/plain"
        << "Custom router rule: 10, key=11";

    QTest::addRow("custom route rule + query key req")
        << "/custom/10?g=1&key=12"
        << 200
        << "text/plain"
        << "Custom router rule: 10, key=12";

    QTest::addRow("post-and-get, get")
        << "/post-and-get"
        << 200
        << "text/plain"
        << "Hello world get";

    QTest::addRow("invalid-rule-method, get")
        << "/invalid-rule-method"
        << 404
        << "application/x-empty"
        << "";

    QTest::addRow("check custom type, data=1")
        << "/check-custom-type/1"
        << 200
        << "text/plain"
        << "data = 1";

    QTest::addRow("any, get")
        << "/any"
        << 200
        << "text/plain"
        << "Get";

    QTest::addRow("response from html file")
        << "/file/text.html"
        << 200
        << "text/html"
        << "<html></html>";

    QTest::addRow("response from json file")
        << "/file/application.json"
        << 200
        << "application/json"
        << "{ \"key\": \"value\" }";

    QTest::addRow("json-object")
        << "/json-object/"
        << 200
        << "application/json"
        << "{\"property\":\"test\",\"value\":1}";

    QTest::addRow("json-array")
        << "/json-array/"
        << 200
        << "application/json"
        << "[1,\"2\",{\"name\":\"test\"}]";

    QTest::addRow("data-and-custom-status-code")
            << "/data-and-custom-status-code/"
            << 202
            << "application/json"
            << "{\"key\":\"value\"}";

    QTest::addRow("chunked")
        << "/chunked/"
        << 200
        << "text/plain"
        << "part 1 of the message, part 2 of the message";

#if QT_CONFIG(concurrent)
    QTest::addRow("future")
        << "/future/1"
        << 200
        << "text/plain"
        << "future is coming";

    QTest::addRow("future-not-found")
        << "/future/0"
        << 404
        << "application/x-empty"
        << "";
#endif
}

void tst_QHttpServer::routeGet()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QFETCH(QString, url);
    QFETCH(int, code);
    QFETCH(QString, type);
    QFETCH(QString, body);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    QNetworkRequest request = QNetworkRequest(QUrl{urlBase.arg(url)});
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);

    std::unique_ptr<QNetworkReply> reply(networkAccessManager.get(request));
    QTRY_VERIFY(reply->isFinished());

    const QVariant http2Used = reply->attribute(QNetworkRequest::Http2WasUsedAttribute);
    QVERIFY(http2Used.isValid());
    QCOMPARE(http2Used.toBool(), useHttp2);

    const QVariant sslUsed = reply->attribute(QNetworkRequest::ConnectionEncryptedAttribute);
    QVERIFY(sslUsed.isValid());
    QCOMPARE(sslUsed.toBool(), useSsl);

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), type);
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), code);
    QCOMPARE(reply->readAll().trimmed(), body);
}

void tst_QHttpServer::routeKeepAlive()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    if (useHttp2)
        QSKIP("Keep-alive header is not allowed for HTTP 2");

    httpserver.route("/keep-alive", this,
                    [] (const QHttpServerRequest &req) -> QHttpServerResponse {
        return QString("header: %1, query: %2, body: %3, method: %4")
            .arg(req.value("CustomHeader"),
                 req.url().query(),
                 req.body())
            .arg(static_cast<int>(req.method()));
    });

    QNetworkRequest request(QUrl{urlBase.arg("/keep-alive")});
    request.setRawHeader(QByteArray("Connection"), QByteArray("keep-alive"));
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);

    checkReply(networkAccessManager.get(request),
               QString("header: , query: , body: , method: %1")
                 .arg(static_cast<int>(QHttpServerRequest::Method::Get)));
    if (QTest::currentTestFailed())
        return;

    request.setUrl(QUrl{urlBase.arg("/keep-alive?po=98")});
    request.setRawHeader("CustomHeader", "1");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/html"_ba);

    checkReply(networkAccessManager.post(request, QByteArray("test")),
               QString("header: 1, query: po=98, body: test, method: %1")
                 .arg(static_cast<int>(QHttpServerRequest::Method::Post)));
    if (QTest::currentTestFailed())
        return;

    request = QNetworkRequest(QUrl{urlBase.arg("/keep-alive")});
    request.setRawHeader(QByteArray("Connection"), QByteArray("keep-alive"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/html"_ba);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);

    checkReply(networkAccessManager.post(request, QByteArray("")),
               QString("header: , query: , body: , method: %1")
                 .arg(static_cast<int>(QHttpServerRequest::Method::Post)));
    if (QTest::currentTestFailed())
        return;

    checkReply(networkAccessManager.get(request),
               QString("header: , query: , body: , method: %1")
                 .arg(static_cast<int>(QHttpServerRequest::Method::Get)));
    if (QTest::currentTestFailed())
        return;
}

void tst_QHttpServer::routePost_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<int>("code");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("data");
    QTest::addColumn<QString>("body");

    QTest::addRow("hello world")
        << "/"
        << 200
        << "text/plain"
        << ""
        << "Hello world post";

    QTest::addRow("post-and-get, post")
        << "/post-and-get"
        << 200
        << "text/plain"
        << ""
        << "Hello world post";

    QTest::addRow("any, post")
        << "/any"
        << 200
        << "text/plain"
        << ""
        << "Post";

    QTest::addRow("post-body")
        << "/post-body"
        << 200
        << "text/plain"
        << "some post data"
        << "some post data";

    QString body;
    for (int i = 0; i < 10000; i++)
        body.append(QString::number(i));

    QTest::addRow("post-body - huge body, chunk test")
        << "/post-body"
        << 200
        << "text/plain"
        << body
        << body;

    QTest::addRow("req-and-resp")
        << "/req-and-resp"
        << 200
        << "text/html"
        << "test"
        << "test";

    QTest::addRow("resp-and-req")
        << "/resp-and-req"
        << 200
        << "text/html"
        << "test"
        << "test";

    QTest::addRow("resp-and-req-from-object")
        << "/resp-and-req-from-object"
        << 200
        << "text/plain"
        << "test"
        << "test";

    QTest::addRow("req-and-resp-from-object")
        << "/req-and-resp-from-object"
        << 200
        << "text/plain"
        << "test"
        << "test";

    QTest::addRow("resp-and-req-from-const-object")
        << "/resp-and-req-from-const-object"
        << 200
        << "text/plain"
        << "test"
        << "test";

    QTest::addRow("req-and-resp-from-const-object")
        << "/req-and-resp-from-const-object"
        << 200
        << "text/plain"
        << "test"
        << "test";

    QTest::addRow("deleted-route")
        << "/deleted-route"
        << 404
        << "application/x-empty"
        << ""
        << "";
}


void tst_QHttpServer::routePost()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QFETCH(QString, url);
    QFETCH(int, code);
    QFETCH(QString, type);
    QFETCH(QString, data);
    QFETCH(QString, body);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    QNetworkRequest request(QUrl{urlBase.arg(url)});
    if (data.size())
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/html"_ba);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);

    std::unique_ptr<QNetworkReply> reply(networkAccessManager.post(request, data.toUtf8()));

    QTRY_VERIFY(reply->isFinished());

    const QVariant http2Used = reply->attribute(QNetworkRequest::Http2WasUsedAttribute);
    QVERIFY(http2Used.isValid());
    QCOMPARE(http2Used.toBool(), useHttp2);

    const QVariant sslUsed = reply->attribute(QNetworkRequest::ConnectionEncryptedAttribute);
    QVERIFY(sslUsed.isValid());
    QCOMPARE(sslUsed.toBool(), useSsl);

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), type);
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), code);
    QCOMPARE(reply->readAll(), body);
}

void tst_QHttpServer::routeDelete_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<int>("code");
    QTest::addColumn<QString>("type");

    QTest::addRow("post-and-get, delete")
        << "/post-and-get"
        << 404
        << "application/x-empty";

    QTest::addRow("any, delete")
        << "/any"
        << 200
        << "text/plain";
}

void tst_QHttpServer::routeDelete()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QFETCH(QString, url);
    QFETCH(int, code);
    QFETCH(QString, type);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    QNetworkRequest request(QUrl{urlBase.arg(url)});
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);

    std::unique_ptr<QNetworkReply> reply(networkAccessManager.deleteResource(request));

    QTRY_VERIFY(reply->isFinished());

    const QVariant http2Used = reply->attribute(QNetworkRequest::Http2WasUsedAttribute);
    QVERIFY(http2Used.isValid());
    QCOMPARE(http2Used.toBool(), useHttp2);

    const QVariant sslUsed = reply->attribute(QNetworkRequest::ConnectionEncryptedAttribute);
    QVERIFY(sslUsed.isValid());
    QCOMPARE(sslUsed.toBool(), useSsl);

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), type);
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), code);
}

void tst_QHttpServer::routeExtraHeaders()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    QNetworkRequest request(QUrl{urlBase.arg("/extra-headers")});
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);

    std::unique_ptr<QNetworkReply> reply(networkAccessManager.get(request));

    QTRY_VERIFY(reply->isFinished());

    const QVariant http2Used = reply->attribute(QNetworkRequest::Http2WasUsedAttribute);
    QVERIFY(http2Used.isValid());
    QCOMPARE(http2Used.toBool(), useHttp2);

    const QVariant sslUsed = reply->attribute(QNetworkRequest::ConnectionEncryptedAttribute);
    QVERIFY(sslUsed.isValid());
    QCOMPARE(sslUsed.toBool(), useSsl);

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), "application/x-empty");
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);
    QCOMPARE(reply->header(QNetworkRequest::ServerHeader), "test server");
}

void tst_QHttpServer::getLongChunks()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    QNetworkRequest request(QUrl{urlBase.arg("/longChunks/")});
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);

    std::unique_ptr<QNetworkReply> reply(networkAccessManager.get(request));
    QTRY_VERIFY(reply->isFinished());

    const QVariant http2Used = reply->attribute(QNetworkRequest::Http2WasUsedAttribute);
    QVERIFY(http2Used.isValid());
    QCOMPARE(http2Used.toBool(), useHttp2);

    const QVariant sslUsed = reply->attribute(QNetworkRequest::ConnectionEncryptedAttribute);
    QVERIFY(sslUsed.isValid());
    QCOMPARE(sslUsed.toBool(), useSsl);

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), "text/plain");
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);

    QByteArray body(reply->readAll());
    qsizetype offset = 0;
    char parts[] = { 'a', 'b', 'c' };
    constexpr qsizetype chunkLength = 1024 * 1024;
    for (auto part : parts) {
        for (int i = 0; i < chunkLength; ++i)
            QCOMPARE(body[i + offset], part);

        offset += chunkLength;
    }
}

struct CustomType {
    CustomType() {}
    CustomType(const QString &) {}
};

void tst_QHttpServer::invalidRouterArguments()
{
    QTest::ignoreMessage(QtWarningMsg, "Can not find converter for type: QDateTime");
    QCOMPARE(
        httpserver.route("/datetime/", this, [] (const QDateTime &datetime) {
            return QString("datetime: %1").arg(datetime.toString());
        }),
        nullptr);

    QTest::ignoreMessage(QtWarningMsg,
                         "CustomType has not registered a converter to QString. "
                         "Use QHttpServerRouter::addConveter<Type>(converter).");
    QCOMPARE(
        httpserver.route("/implicit-conversion-to-qstring-has-no-registered/",
                         this,
                         [] (const CustomType &) {
            return "";
        }),
        nullptr);
}

void tst_QHttpServer::checkRouteLambdaCapture()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    QNetworkRequest request;
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);

    httpserver.route("/capture-this/", this, [&urlBase] () {
        return urlBase;
    });

    QString msg = urlBase + "/pod";
    httpserver.route("/capture-non-pod-data/", this, [&msg] () {
        return msg;
    });

    request.setUrl(QUrl(urlBase.arg("/capture-this/")));
    checkReply(networkAccessManager.get(request), urlBase);
    if (QTest::currentTestFailed())
        return;

    request.setUrl(QUrl(urlBase.arg("/capture-non-pod-data/")));
    checkReply(networkAccessManager.get(request), msg);
    if (QTest::currentTestFailed())
        return;
}

void tst_QHttpServer::afterRequest()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    QNetworkRequest request(QUrl{urlBase.arg("/test-after-request")});
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);

    httpserver.addAfterRequestHandler(this, [] (const QHttpServerRequest &request,
                                QHttpServerResponse &resp) {
        if (request.url().path() == "/test-after-request") {
            auto h = resp.headers();
            h.removeAll("Arguments-Order-1");
            h.append("Arguments-Order-1", "resp, request");
            resp.setHeaders(std::move(h));
        }
    });

    httpserver.addAfterRequestHandler(this, [] (const QHttpServerRequest &request,
                                QHttpServerResponse &resp) {
        if (request.url().path() == "/test-after-request") {
            auto h = resp.headers();
            h.removeAll("Arguments-Order-2");
            h.append("Arguments-Order-2", "request, resp");
            resp.setHeaders(std::move(h));
        }
    });

    std::unique_ptr<QNetworkReply> reply(networkAccessManager.get(request));

    QTRY_VERIFY(reply->isFinished());

    const QVariant http2Used = reply->attribute(QNetworkRequest::Http2WasUsedAttribute);
    QVERIFY(http2Used.isValid());
    QCOMPARE(http2Used.toBool(), useHttp2);

    const QVariant sslUsed = reply->attribute(QNetworkRequest::ConnectionEncryptedAttribute);
    QVERIFY(sslUsed.isValid());
    QCOMPARE(sslUsed.toBool(), useSsl);

    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), "application/x-empty");
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 404);
    QCOMPARE(reply->rawHeader("Arguments-Order-1"), "resp, request");
    QCOMPARE(reply->rawHeader("Arguments-Order-2"), "request, resp");
}

void tst_QHttpServer::checkReply(QNetworkReply *reply, const QString &response) {
    QVERIFY(reply != nullptr);
    std::unique_ptr<QNetworkReply> replyPtr(reply);
    QTRY_VERIFY(replyPtr->isFinished());

    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);

    const QVariant http2Used = replyPtr->attribute(QNetworkRequest::Http2WasUsedAttribute);
    QVERIFY(http2Used.isValid());
    QCOMPARE(http2Used.toBool(), useHttp2);

    const QVariant sslUsed = replyPtr->attribute(QNetworkRequest::ConnectionEncryptedAttribute);
    QVERIFY(sslUsed.isValid());
    QCOMPARE(sslUsed.toBool(), useSsl);

    QCOMPARE(replyPtr->header(QNetworkRequest::ContentTypeHeader).toByteArray(), "text/plain");
    QCOMPARE(replyPtr->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);
    QCOMPARE(replyPtr->readAll(), response);
};

void tst_QHttpServer::disconnectedInEventLoop()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    QNetworkRequest request(QUrl{urlBase.arg("/event-loop/")});
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);

    httpserver.route("/event-loop/", this, [] () {
        QEventLoop loop;
        QTimer::singleShot(1000, &loop, &QEventLoop::quit);
        loop.exec();
        return QHttpServerResponse::StatusCode::Ok;
    });

    std::unique_ptr<QNetworkReply> reply(networkAccessManager.get(request));
    QTimer::singleShot(500, reply.get(), &QNetworkReply::abort); // cancel connection
    QEventLoop loop;
    connect(reply.get(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
}

void tst_QHttpServer::multipleRequests()
{
    // Test to ensure that the passed lambda is not moved away after the
    // first handled request
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    QNetworkRequest request(QUrl{urlBase.arg("/do-not-move")});
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);

    httpserver.route("/do-not-move", this, [v = std::vector<int>{1, 2, 3}] () {
        return QString::number(v.size());
    });

    checkReply(networkAccessManager.get(request), "3");
    if (QTest::currentTestFailed())
        return;

    checkReply(networkAccessManager.get(request), "3");
    if (QTest::currentTestFailed())
        return;
}

void tst_QHttpServer::pipelinedRequests()
{
    QNetworkReply *replies[10];
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;

    for (std::size_t i = 0; i < std::size(replies); i++) {
        QNetworkRequest req(QUrl(urlBase.arg("/user/") + QString::number(i)));
        req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
        req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);

        replies[i] = networkAccessManager.get(req);
    }

    for (std::size_t i = 0; i < std::size(replies); i++)
        checkReply(replies[i], QString::number(i));
}

class OkResponder : public QObject
{
    Q_OBJECT

public:
    void operator()(const QHttpServerRequest &, QHttpServerResponder &responder)
    {
        responder.write(QHttpServerResponder::StatusCode::Ok);
    }
};

void tst_QHttpServer::missingHandler()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    const QUrl requestUrl(urlBase.arg("/missing"));
    QNetworkRequest request(requestUrl);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);

    std::unique_ptr<QNetworkReply> reply(networkAccessManager.get(request));
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 404);

    {
        auto guard = QScopeGuard([this]() { httpserver.clearMissingHandler(); });

        httpserver.setMissingHandler(
                this, [](const QHttpServerRequest &, QHttpServerResponder &responder) {
                    responder.write(QHttpServerResponder::StatusCode::Ok);
                });

        reply.reset(networkAccessManager.get(request));
        QTRY_VERIFY(reply->isFinished());
        QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);
    }

    reply.reset(networkAccessManager.get(request));
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 404);

    {
        OkResponder responder;
        httpserver.setMissingHandler(&responder, &OkResponder::operator());

        QNetworkRequest request2(requestUrl);
        request2.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
        reply.reset(networkAccessManager.get(QNetworkRequest(requestUrl)));

        QTRY_VERIFY(reply->isFinished());
        QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);
    }

    reply.reset(networkAccessManager.get(request));
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 404);

    const QVariant http2Used = reply->attribute(QNetworkRequest::Http2WasUsedAttribute);
    QVERIFY(http2Used.isValid());
    QCOMPARE(http2Used.toBool(), useHttp2);

    const QVariant sslUsed = reply->attribute(QNetworkRequest::ConnectionEncryptedAttribute);
    QVERIFY(sslUsed.isValid());
    QCOMPARE(sslUsed.toBool(), useSsl);
}

// Test that responses to pipelined requests come in correct order, see also: QTBUG-105202
void tst_QHttpServer::pipelinedFutureRequests()
{
#if QT_CONFIG(concurrent)
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;

    std::array<QNetworkReply *, 10> replies;
    QThreadPool::globalInstance()->setMaxThreadCount(static_cast<int>(replies.size()));

    for (std::size_t i = 0; i < replies.size(); i++) {
        auto delayMs = 1000 / replies.size() * (replies.size() - i);

        QString path = u"/user/%1/delayed/%2"_s.arg(i).arg(delayMs);
        QNetworkRequest req(QUrl(urlBase.arg(path)));
        req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
        req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
        replies[i] = networkAccessManager.get(req);
    }

    for (std::size_t i = 0; i < replies.size(); i++)
        checkReply(replies[i], QString::number(i));
#else
    QSKIP("QtConcurrent is not available, skipping test");
#endif // QT_CONFIG(concurrent)
}

void tst_QHttpServer::requestNotOverwritten()
{
#if QT_CONFIG(concurrent)
    readySem.emplace();
    routeSem.emplace();

    auto cleanup = qScopeGuard([this] {
        readySem.reset();
        routeSem.reset();
    });

    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;

    constexpr qsizetype NumberProcessed = 6;
    std::array<QNetworkReply *, NumberProcessed> replies;
    QThreadPool::globalInstance()->setMaxThreadCount(static_cast<int>(NumberProcessed));

    for (std::size_t i = 0; i < NumberProcessed; i++) {
        auto bodySize = i + 1; // All requests have different body sizes
        QByteArray body(bodySize, 'a');
        QString path = u"/wait-and-return-body/"_s;
        QNetworkRequest req(QUrl(urlBase.arg(path)));
        req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
        req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
        req.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
        replies[i] = networkAccessManager.post(req, body);
    }

    auto waitUntilFuturesStarted = [this, &replies] {
        if (QTest::qWaitFor([this]() { return readySem->available() == NumberProcessed; })) {
            return true;
        } else {
            routeSem->release(NumberProcessed);
            for (std::size_t i = 0; i < NumberProcessed; i++) {
                if (replies[i] != nullptr) {
                    std::unique_ptr<QNetworkReply> replyPtr(replies[i]);
                    replyPtr->abort();
                }
            }
            return false;
        }
    };

    QVERIFY(waitUntilFuturesStarted());
    readySem->acquire(NumberProcessed);
    routeSem->release(NumberProcessed); // Let them complete and execute the afterrequest handlers

    QSet<int> bodySizes;
    for (std::size_t i = 0; i < NumberProcessed; i++) {
        QVERIFY(replies[i] != nullptr);
        std::unique_ptr<QNetworkReply> replyPtr(replies[i]);
        QTRY_VERIFY(replyPtr->isFinished());
        QVERIFY(replyPtr->hasRawHeader("X-REQUEST-BODY-SIZE"));
        QByteArray header = replyPtr->rawHeader("X-REQUEST-BODY-SIZE");
        bool ok;
        int bodySize = header.toInt(&ok);
        QVERIFY(ok);
        bodySizes.insert(bodySize);
    }
    QCOMPARE(bodySizes.size(), NumberProcessed);
    QCOMPARE(readySem->available(), 0);
    QCOMPARE(routeSem->available(), 0);
#else
    QSKIP("QtConcurrent is not available, skipping test");
#endif // QT_CONFIG(concurrent)
}

void tst_QHttpServer::parallelFutureRequests()
{
#if QT_CONFIG(concurrent)
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;

    constexpr qsizetype NumberOfTasks = 10;
    std::array<QNetworkReply *, NumberOfTasks> replies;
    QThreadPool::globalInstance()->setMaxThreadCount(NumberOfTasks);
    QCOMPARE(networkAccessManagers.size(), NumberOfTasks);
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        int delayMs = NumberOfTasks - i;
        QString path = u"/sleepingfuture/%1"_s.arg(delayMs);
        QNetworkRequest req(QUrl(urlBase.arg(path)));
        req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
        req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
        replies[i] = networkAccessManagers[i].get(req);
    }

    for (qint64 i = 0; i < NumberOfTasks; i++) {
        checkReply(replies[i], QString::number(NumberOfTasks - i));
    }
#else
    QSKIP("QtConcurrent is not available, skipping test");
#endif // QT_CONFIG(concurrent)
}

void tst_QHttpServer::slowReader()
{
#if QT_CONFIG(concurrent)
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    constexpr qsizetype NumberOfTasks = 10;
    constexpr qsizetype NumberOfRepeats = 5;
    std::array<QNetworkReply *, NumberOfTasks> replies;
    QThreadPool::globalInstance()->setMaxThreadCount(NumberOfTasks);
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        QString path = u"/slowreader/Message%1/%2"_s.arg(i).arg(NumberOfRepeats);
        QNetworkRequest req(QUrl(urlBase.arg(path)));
        req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
        req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
        replies[i] = networkAccessManager.get(req);
    }

    for (qint64 i = 0; i < NumberOfTasks; i++) {
        checkReply(replies[i], u"Message%1"_s.arg(i).repeated(NumberOfRepeats));
    }
#else
    QSKIP("QtConcurrent is not available, skipping test");
#endif // QT_CONFIG(concurrent)
}

void tst_QHttpServer::concurrentRequestBack()
{
#if QT_CONFIG(concurrent)
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;

    constexpr qsizetype NumberOfTasks = 10;
    std::array<QNetworkReply *, NumberOfTasks> replies;
    QThreadPool::globalInstance()->setMaxThreadCount(NumberOfTasks);
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        auto url = i % 2 == 0 ? "/concurrent-request-back"_L1 : "/concurrent-request-back2"_L1;
        QNetworkRequest req(QUrl(urlBase.arg(url)));
        req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
        req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
        req.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain"_ba);
        QByteArray body = "RepeatMe"_ba.repeated(i + 1);
        replies[i] = networkAccessManager.post(req, body);
    }

    for (qint64 i = 0; i < NumberOfTasks; i++) {
        QByteArray body = "RepeatMe"_ba.repeated(i + 1);
        checkReply(replies[i], QString::fromUtf8(body));
    }
#else
    QSKIP("QtConcurrent is not available, skipping test");
#endif // QT_CONFIG(concurrent)
}

void tst_QHttpServer::concurrentMultipart()
{
#if QT_CONFIG(concurrent)
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    constexpr qsizetype NumberOfTasks = 10;
    std::array<QString, NumberOfTasks> messages = {
        "First", "Second", "Third", "Fourth", "Fifth", "Sixth", "Seventh", "Eight", "Ninth", "Tenth"
    };
    QCOMPARE(messages.size(), NumberOfTasks);
    std::array<QNetworkReply *, NumberOfTasks> replies;
    QThreadPool::globalInstance()->setMaxThreadCount(NumberOfTasks);
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        QString path = u"/concurrent-multipart-back/100/10/%1"_s.arg(messages[i]);
        QNetworkRequest req(QUrl(urlBase.arg(path.toUtf8())));
        req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
        req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
        req.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain"_ba);
        replies[i] = networkAccessManager.get(req);
    }

    for (qsizetype i = 0; i < NumberOfTasks; i++) {
        QString body = messages[i].repeated(100);
        checkReply(replies[i], body);
    }
#else
    QSKIP("QtConcurrent is not available, skipping test");
#endif // QT_CONFIG(concurrent)
}

void tst_QHttpServer::multipleResponses()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    if (useHttp2)
        QSKIP("StatusCode::Processing is not supported for HTTP 2");

    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;

    const QUrl requestUrl(urlBase.arg("/processing"));
    QNetworkRequest req(requestUrl);
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
    auto reply = networkAccessManager.get(req);

    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);
    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), "text/plain");
    QCOMPARE(reply->readAll(), "done");
}

void tst_QHttpServer::contextObjectInOtherThreadWarning()
{
    QThread newThread;
    ReplyObject replyObject;
    replyObject.moveToThread(&newThread);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*context object.*same thread"));
    QCOMPARE(httpserver.route("/otherThread", &replyObject, &ReplyObject::replyReqResp), nullptr);
}

#if QT_CONFIG(localserver)
void tst_QHttpServer::localSocket()
{
    using namespace Qt::StringLiterals;
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    if (useSsl || useHttp2)
        QSKIP("Use only HTTP 1.1 for localSocket");

    QVERIFY(!httpserver.localServers().isEmpty());
    QHttpServerConfiguration configuration;
    qsizetype timeout = configuration.keepAliveTimeout().count();

    for (const auto &localServer : httpserver.localServers()) {
        QLocalSocket socket;
        socket.connectToServer(localServer->fullServerName());
        QVERIFY(socket.waitForConnected(1));

        qApp->processEvents();

        socket.write("GET /resp HTTP/1.1\r\n"
                     "Host: local\r\n"
                     "User-Agent: curl/7.88.1\r\n"
                     "Accept: */*\r\n\r\n");

        const QString expectedString =
                u"HTTP/1.1 200 OK\r\ncontent-type: text/html\r\ncontent-length: 8\r\n"_s
                u"connection: keep-alive\r\nkeep-alive: timeout=%1\r\n\r\ntest msg"_s;
        const QByteArray expectedResult = expectedString.arg(timeout).toUtf8();

        // We need to call process events a couple of times for the write/read to go through
        QTRY_COMPARE_GE(socket.bytesAvailable(), expectedResult.size());
        QCOMPARE(socket.readAll(), expectedResult);
    }
}
#endif

void tst_QHttpServer::keepAliveTimeout()
{
#if QT_CONFIG(concurrent)
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);

    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;

    QHttpServerConfiguration config;
    config.setKeepAliveTimeout(1s);
    httpserver.setConfiguration(config);

    auto cleanup = qScopeGuard([this] {
        QHttpServerConfiguration config;
        httpserver.setConfiguration(config);
    });

    const auto slowWaitTime = QString::number(6000);
    QNetworkRequest reqSlow(QUrl(urlBase.arg(u"/wait/"_s + slowWaitTime)));
    reqSlow.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    reqSlow.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
    QNetworkReply *slowReply = networkAccessManager.get(reqSlow);

    const auto fastWaitTime = QString::number(1000);
    QNetworkRequest reqFast(QUrl(urlBase.arg(u"/wait/"_s + fastWaitTime)));
    reqFast.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    reqFast.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
    QNetworkReply *fastReply = networkAccessManager.get(reqFast);

    QTRY_VERIFY_WITH_TIMEOUT(slowReply->isFinished() && fastReply->isFinished(), 7000);

    checkReply(slowReply, slowWaitTime);
    checkReply(fastReply, fastWaitTime);
#else
    QSKIP("QtConcurrent is not available, skipping test");
#endif // QT_CONFIG(concurrent)
}

void tst_QHttpServer::maximumUrlSizeAllowed()
{
#if QT_CONFIG(concurrent)
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);

    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;

    QHttpServerConfiguration config;
    config.setMaximumUrlSize(30);
    httpserver.setConfiguration(config);

    auto cleanup = qScopeGuard([this] {
        QHttpServerConfiguration config;
        httpserver.setConfiguration(config);
    });

    QNetworkRequest request(QUrl(urlBase.arg(u"/req-and-resp-from-object"_s)));
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    std::unique_ptr<QNetworkReply> reply(networkAccessManager.post(request, "Hey"));

    QTRY_VERIFY(reply->isFinished());
    checkReply(reply.release(), "Hey");

    config.setMaximumUrlSize(3);
    httpserver.setConfiguration(config);

    reply.reset(networkAccessManager.post(request, "Hey"));
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 414);
    QTest::qSleep(2000);

#else
    QSKIP("QtConcurrent is not available, skipping test");
#endif // QT_CONFIG(concurrent)
}

void tst_QHttpServer::maximumTotalHeaderSizeAllowed()
{
#if QT_CONFIG(concurrent)
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);

    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;

    QHttpServerConfiguration config;
    config.setMaximumTotalHeaderSize(500);
    httpserver.setConfiguration(config);

    auto cleanup = qScopeGuard([this] {
        QHttpServerConfiguration config;
        httpserver.setConfiguration(config);
    });

    QNetworkRequest request(QUrl(urlBase.arg(u"/req-and-resp-from-object"_s)));
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    std::unique_ptr<QNetworkReply> reply(networkAccessManager.post(request, "Hey"));

    QTRY_VERIFY(reply->isFinished());
    checkReply(reply.release(), "Hey");

    config.setMaximumTotalHeaderSize(10);
    httpserver.setConfiguration(config);

    reply.reset(networkAccessManager.post(request, "Hey"));
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 431);
    QTest::qSleep(2000);

#else
    QSKIP("QtConcurrent is not available, skipping test");
#endif // QT_CONFIG(concurrent)
}

void tst_QHttpServer::maximumHeaderFieldSizeAllowed()
{
#if QT_CONFIG(concurrent)
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);

    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;

    QHttpServerConfiguration config;
    config.setMaximumHeaderFieldSize(500);
    httpserver.setConfiguration(config);

    auto cleanup = qScopeGuard([this] {
        QHttpServerConfiguration config;
        httpserver.setConfiguration(config);
    });

    QNetworkRequest request(QUrl(urlBase.arg(u"/req-and-resp-from-object"_s)));
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    request.setRawHeader("X-Big-Header", QByteArray("GonnaBeLarge").repeated(10));
    std::unique_ptr<QNetworkReply> reply(networkAccessManager.post(request, "Hey"));

    QTRY_VERIFY(reply->isFinished());
    checkReply(reply.release(), "Hey");

    config.setMaximumHeaderFieldSize(100);
    httpserver.setConfiguration(config);

    reply.reset(networkAccessManager.post(request, "Hey"));
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 431);
    QTest::qSleep(2000);

#else
    QSKIP("QtConcurrent is not available, skipping test");
#endif // QT_CONFIG(concurrent)
}

void tst_QHttpServer::maximumNumberOfHeaderFieldsAllowed()
{
#if QT_CONFIG(concurrent)
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);

    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;

    QHttpServerConfiguration config;
    config.setMaximumHeaderFieldCount(120);
    httpserver.setConfiguration(config);

    auto cleanup = qScopeGuard([this] {
        QHttpServerConfiguration config;
        httpserver.setConfiguration(config);
    });

    QNetworkRequest request(QUrl(urlBase.arg(u"/req-and-resp-from-object"_s)));
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    for (qsizetype i = 0; i < 100; ++i)
        request.setRawHeader(QByteArray("X-Header-").append(QByteArray::number(i)), "Dummydata");
    std::unique_ptr<QNetworkReply> reply(networkAccessManager.post(request, "Hey"));

    QTRY_VERIFY(reply->isFinished());
    checkReply(reply.release(), "Hey");

    config.setMaximumHeaderFieldCount(100);
    httpserver.setConfiguration(config);

    reply.reset(networkAccessManager.post(request, "Hey"));
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 431);
    QTest::qSleep(2000);

#else
    QSKIP("QtConcurrent is not available, skipping test");
#endif // QT_CONFIG(concurrent)
}

void tst_QHttpServer::maximumBodySizeAllowed()
{
#if QT_CONFIG(concurrent)
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);

    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;

    QHttpServerConfiguration config;
    config.setMaximumBodySize(500);
    httpserver.setConfiguration(config);

    auto cleanup = qScopeGuard([this] {
        QHttpServerConfiguration config;
        httpserver.setConfiguration(config);
    });

    QNetworkRequest request(QUrl(urlBase.arg(u"/req-and-resp-from-object"_s)));
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
    std::unique_ptr<QNetworkReply> reply(networkAccessManager.post(request, "Here's the body"));

    QTRY_VERIFY(reply->isFinished());
    checkReply(reply.release(), "Here's the body");

    config.setMaximumBodySize(10);
    httpserver.setConfiguration(config);

    reply.reset(networkAccessManager.post(request, "Here's the body"));
    QTRY_VERIFY(reply->isFinished());
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 413);
    QTest::qSleep(2000);

#else
    QSKIP("QtConcurrent is not available, skipping test");
#endif // QT_CONFIG(concurrent)
}

void tst_QHttpServer::writeSequentialDevice()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);

    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    const QUrl requestUrl(urlBase.arg("/sequential-iodevice/Go/3/1"));
    QNetworkRequest req(requestUrl);
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
    std::unique_ptr<QNetworkReply> reply(networkAccessManager.get(req));

    QSignalSpy spy(reply.get(), &QNetworkReply::finished);
    spy.wait(2s);

    QCOMPARE(spy.count(), 1);
    checkReply(reply.release(), "GoGoGo");
}

void tst_QHttpServer::writeMuchToSequentialDevice()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);

    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    constexpr qsizetype bufferSize = 129 * 1024; // More than the HTTP1 write buffer
    const QUrl requestUrl(
            urlBase.arg(u"/sequential-iodevice/a/2/"_s + QString::number(bufferSize)));
    QNetworkRequest req(requestUrl);
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
    std::unique_ptr<QNetworkReply> reply(networkAccessManager.get(req));

    QSignalSpy spy(reply.get(), &QNetworkReply::finished);
    spy.wait(2s);

    QCOMPARE(spy.count(), 1);
    checkReply(reply.release(), u"a"_s.repeated(bufferSize * 2));
}


void tst_QHttpServer::writeFromEmptySequentialDevice()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);

    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    const QUrl requestUrl(urlBase.arg("/empty-sequential-iodevice/"));
    QNetworkRequest req(requestUrl);
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
    std::unique_ptr<QNetworkReply> reply(networkAccessManager.get(req));

    QSignalSpy spy(reply.get(), &QNetworkReply::finished);
    spy.wait(2s);

    QCOMPARE(spy.count(), 1);
    checkReply(reply.release(), "");
}

void tst_QHttpServer::concurrentRequestsToSequentialDevice()
{
#if QT_CONFIG(concurrent)
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    constexpr qsizetype NumberOfTasks = 10;
    constexpr qsizetype NumberOfRepeats = 3;
    std::array<std::unique_ptr<QNetworkReply>, NumberOfTasks> replies;
    QThreadPool::globalInstance()->setMaxThreadCount(NumberOfTasks);
    for (qsizetype i = 0; i < NumberOfTasks; ++i) {
        QString path =
                u"/sequential-iodevice/%1/%2/1"_s.arg(QChar('a' + (char)i)).arg(NumberOfRepeats);
        QNetworkRequest req(QUrl(urlBase.arg(path)));
        req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
        req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
        replies[i].reset(networkAccessManager.get(req));
    }

    QSignalSpy spy(replies[NumberOfTasks - 1].get(), &QNetworkReply::finished);
    spy.wait(5s); // Wait for reply to last outgoing request

    for (qsizetype i = 0; i < NumberOfTasks; i++) {
        QTRY_VERIFY(replies[i]->isFinished());
        QCOMPARE(replies[i]->readAll(), QString(QChar('a' + (char)i)).repeated(NumberOfRepeats));
    }
#else
    QSKIP("QtConcurrent is not available, skipping test");
#endif // QT_CONFIG(concurrent)
}

void tst_QHttpServer::timeoutConnection()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);
    QVERIFY(!canceledByClient.has_value());

    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    const QUrl requestUrl(urlBase.arg("/timeout-connection"));
    QNetworkRequest req(requestUrl);
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
    req.setTransferTimeout(1s);
    std::unique_ptr<QNetworkReply> reply(networkAccessManager.get(req));

    QSignalSpy spy(reply.get(), &QNetworkReply::finished);
    spy.wait(3s);
    QTRY_VERIFY(canceledByClient.has_value());
    QCOMPARE(spy.count(), 1);
    QVERIFY(reply->isFinished());
    QCOMPARE(reply->error(), QNetworkReply::TimeoutError);
    QVERIFY(canceledByClient.value());
}

void tst_QHttpServer::useCanceledResponders()
{
    QFETCH_GLOBAL(bool, useSsl);
    QFETCH_GLOBAL(bool, useHttp2);

    QString urlBase = useSsl ? sslUrlBase : clearUrlBase;
    const QUrl requestUrl(urlBase.arg("/add-responder"));
    QNetworkRequest req(requestUrl);
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, useHttp2);
    req.setTransferTimeout(1s);
    std::unique_ptr<QNetworkReply> reply1(networkAccessManager.get(req));
    std::unique_ptr<QNetworkReply> reply2(networkAccessManager.get(req));

    QTRY_VERIFY(reply1->isFinished());
    QTRY_VERIFY(reply2->isFinished());

    QCOMPARE(responders.size(), 2);

    for (auto &responder : responders)
        QTRY_VERIFY(responder->isResponseCanceled());

    for (auto &responder : responders)
        responder->writeChunk("chunk");

    for (auto &responder : responders)
        responder->writeEndChunked("end");
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(CustomArg);
Q_DECLARE_METATYPE(CustomType);

QTEST_MAIN(tst_QHttpServer)

#include "tst_qhttpserver.moc"
