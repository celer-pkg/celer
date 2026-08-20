// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtHttpServer/qhttpserverresponder.h>
#include <QtHttpServer/qabstracthttpserver.h>
#include <QtHttpServer/qhttpserverrouter.h>
#include <QtHttpServer/qhttpserverrouterrule.h>

#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qtcpserver.h>

Q_DECLARE_METATYPE(QNetworkAccessManager::Operation);

QT_BEGIN_NAMESPACE

struct HttpServer : QAbstractHttpServer {
    QHttpServerRouter router;

    HttpServer() : router(this) {};

    template<typename ViewHandler>
    void route(const char *path, const QHttpServerRequest::Methods methods, ViewHandler &&viewHandler)
    {
        auto rule = std::make_unique<QHttpServerRouterRule>(
                path, methods, this,
                [this, viewHandler = std::forward<ViewHandler>(viewHandler)](
                        const QRegularExpressionMatch &match, const QHttpServerRequest &,
                        QHttpServerResponder &responder) mutable {
                    auto boundViewHandler = QHttpServerRouterRule::bindCaptured(this, viewHandler, match);
                    boundViewHandler(responder);
                });

        router.addRule<ViewHandler>(std::move(rule));
    }

    template<typename ViewHandler>
    void route(const char *path, ViewHandler &&viewHandler)
    {
        route(path, QHttpServerRequest::Method::AnyKnown, std::forward<ViewHandler>(viewHandler));
    }

    bool handleRequest(const QHttpServerRequest &request, QHttpServerResponder &responder) override
    {
        return router.handleRequest(request, responder);
    }

    void missingHandler(const QHttpServerRequest &, QHttpServerResponder &responder) override
    {
        responder.write(QHttpServerResponder::StatusCode::NotFound);
    }
};

class tst_QHttpServerRouter : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void routerRule_data();
    void routerRule();
    void viewHandlerMemberFunction();
    void viewHandlerNoArg();
    void viewHandlerOneArg();
    void viewHandlerTwoArgs();
    void viewHandlerResponder();
    void viewHandlerRequest();
    void viewHandlerLastTwoSpecials();
    void viewHandlerLastTwoSpecialsValueRValueRef();

private:
    HttpServer httpserver;
    QString urlBase;
};

static void getTest(QHttpServerResponder &responder)
{
    responder.write(QString("get-test").toUtf8(), "text/plain");
}

void tst_QHttpServerRouter::initTestCase()
{
    auto pageHandler = [] (const quint64 &page, QHttpServerResponder &responder) {
        responder.write(QString("page: %1").arg(page).toUtf8(), "text/plain");
    };

    httpserver.route("/page/", pageHandler);

    httpserver.route("/post-only", QHttpServerRequest::Method::Post,
                     [] (QHttpServerResponder &responder) {
        responder.write(QString("post-test").toUtf8(), "text/plain");
    });

    httpserver.route("/get-only", QHttpServerRequest::Method::Get, getTest);

    auto tcpserver = std::make_unique<QTcpServer>();
    QVERIFY2(tcpserver->listen(QHostAddress::Any), "HTTP server listen failed");
    quint16 port = tcpserver->serverPort();
    QVERIFY2(httpserver.bind(tcpserver.get()), "HTTP server bind failed");
    tcpserver.release();
    urlBase = QStringLiteral("http://localhost:%1%2").arg(port);
}

void tst_QHttpServerRouter::routerRule_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<int>("code");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("body");
    QTest::addColumn<QNetworkAccessManager::Operation>("replyType");

    QTest::addRow("/page/1")
        << "/page/1"
        << 200
        << "text/plain"
        << "page: 1"
        << QNetworkAccessManager::GetOperation;

    QTest::addRow("/page/-1")
        << "/page/-1"
        << 404
        << "application/x-empty"
        << ""
        << QNetworkAccessManager::GetOperation;

    QTest::addRow("/post-only [GET]")
        << "/post-only"
        << 404
        << "application/x-empty"
        << ""
        << QNetworkAccessManager::GetOperation;

    QTest::addRow("/post-only [DELETE]")
        << "/post-only"
        << 404
        << "application/x-empty"
        << ""
        << QNetworkAccessManager::DeleteOperation;

    QTest::addRow("/post-only [POST]")
        << "/post-only"
        << 200
        << "text/plain"
        << "post-test"
        << QNetworkAccessManager::PostOperation;

    QTest::addRow("/get-only [GET]")
        << "/get-only"
        << 200
        << "text/plain"
        << "get-test"
        << QNetworkAccessManager::GetOperation;

    QTest::addRow("/get-only [POST]")
        << "/get-only"
        << 404
        << "application/x-empty"
        << ""
        << QNetworkAccessManager::PostOperation;

    QTest::addRow("/get-only [DELETE]")
        << "/get-only"
        << 404
        << "application/x-empty"
        << ""
        << QNetworkAccessManager::DeleteOperation;
}

void tst_QHttpServerRouter::routerRule()
{
    QFETCH(QString, url);
    QFETCH(int, code);
    QFETCH(QString, type);
    QFETCH(QString, body);
    QFETCH(QNetworkAccessManager::Operation, replyType);

    QNetworkAccessManager networkAccessManager;
    QNetworkReply *reply;
    QNetworkRequest request(QUrl(urlBase.arg(url)));

    switch (replyType) {
    case QNetworkAccessManager::GetOperation:
        reply = networkAccessManager.get(request);
        break;
    case QNetworkAccessManager::PostOperation:
        request.setHeader(QNetworkRequest::ContentTypeHeader, type);
        reply = networkAccessManager.post(request, QByteArray("post body"));
        break;
    case QNetworkAccessManager::DeleteOperation:
        reply = networkAccessManager.deleteResource(request);
        break;
    default:
        QFAIL("The replyType does not supported");
    }

    QTRY_VERIFY(reply->isFinished());

    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), code);
    QCOMPARE(reply->header(QNetworkRequest::ContentTypeHeader), type);
    QCOMPARE(reply->readAll(), body);
}

void tst_QHttpServerRouter::viewHandlerMemberFunction()
{
    class ViewClass
    {
    public:
        void viewClassNoArg()
        {
        }

        void viewClassOneArg(const quint64 &)
        {
        }

        QString viewClassReturnString()
        {
            return "";
        }
    };

    using ViewTraitsNoArg = QHttpServerRouterViewTraits<decltype(&ViewClass::viewClassNoArg), true>;
    using ArgsNoArg = typename ViewTraitsNoArg::Arguments;

    static_assert(ArgsNoArg::Count == 0,
                  "viewClassNoArg: Args::Count == 0");
    static_assert(ArgsNoArg::CapturableCount == 0,
                  "viewClassNoArg: Args::CapturableCount == 0");
    static_assert(ArgsNoArg::SpecialsCount == 0, "viewClassNoArg: Args::SpecialsCount == 0");
    static_assert(ArgsNoArg::StaticAssert, "viewClassNoArg: Args::StaticAssert");

    static_assert(std::is_same<ViewTraitsNoArg::BindableType, std::function<void()>>::value,
                  "viewClassNoArg::BindableType");

    using ViewTraitsOneArg = QHttpServerRouterViewTraits<decltype(&ViewClass::viewClassOneArg), true>;
    using ArgsOneArg = typename ViewTraitsOneArg::Arguments;

    static_assert(ArgsOneArg::Count == 1,
                  "viewOneArg: Args::Count == 1");
    static_assert(ArgsOneArg::CapturableCount == 1,
                  "viewOneArg: Args::CapturableCount == 1");
    static_assert(ArgsOneArg::SpecialsCount == 0, "viewOneArg: Args::SpecialsCount == 0");
    static_assert(ArgsOneArg::Last::IsRequest::Value == 0,
                  "viewOneArg: Args::Last::IsRequest::Value == 0");
    static_assert(ArgsOneArg::Last::IsResponder::Value == 0,
                  "viewOneArg: Args::Last::IsResponder::Value == 0");
    static_assert(ArgsOneArg::Last::IsSpecial::Value == 0,
                  "viewOneArg: Args::Last::IsSpecial::Value == 0");
    static_assert(ArgsOneArg::Last::IsSimple::Value,
                  "viewOneArg: Args::Last::IsSimple::Value");
    static_assert(ArgsOneArg::Last::Value,
                  "viewOneArg: Args::Last::Value");
    static_assert(ArgsOneArg::Last::StaticAssert,
                  "viewOneArg: Args::Last::StaticAssert");
    static_assert(std::is_same<ArgsOneArg::Last::Type, const quint64 &>::value,
                  "viewNonArg: std::is_same<Args::Last::Type, const quint64 &>");
    static_assert(ArgsOneArg::Value, "viewOneArg: Args::Value");
    static_assert(ArgsOneArg::StaticAssert, "viewOneArg: Args::StaticAssert");

    using ViewTraitsReturnString = QHttpServerRouterViewTraits<decltype(&ViewClass::viewClassReturnString), true>;

    static_assert(std::is_same<ViewTraitsReturnString::ReturnType, QString>::value,
                  "ArgsReturnString: std::is_same<ViewTraitsReturnString::ReturnType, QString>");
    static_assert(std::is_same<ViewTraitsReturnString::BindableType, std::function<QString()>>::value,
                  "viewClassNoArg::BindableType");

}

void tst_QHttpServerRouter::viewHandlerNoArg()
{
    auto viewNonArg = [] () {
    };

    using ViewTraits = QHttpServerRouterViewTraits<decltype(viewNonArg), true>;
    using Args = typename ViewTraits::Arguments;

    static_assert(Args::Count == 0,
                  "viewNonArg: Args::Count == 0");
    static_assert(Args::CapturableCount == 0,
                  "viewNonArg: Args::CapturableCount == 0");
    static_assert(Args::SpecialsCount == 0, "viewNonArg: Args::SpecialsCount == 0");

    static_assert(Args::Value, "viewNonArg: Args::Value");
    static_assert(Args::StaticAssert, "viewNonArg: Args::StaticAssert");

    static_assert(std::is_same<ViewTraits::BindableType, std::function<void()>>::value,
                  "viewClassNoArg::BindableType");

}

void tst_QHttpServerRouter::viewHandlerOneArg()
{
    auto view = [] (const quint64 &) {
    };

    using ViewTraits = QHttpServerRouterViewTraits<decltype(view), true>;
    using Args = typename ViewTraits::Arguments;

    static_assert(Args::Count == 1,
                  "viewOneArg: Args::Count == 1");
    static_assert(Args::CapturableCount == 1,
                  "viewOneArg: Args::CapturableCount == 1");
    static_assert(Args::SpecialsCount == 0, "viewOneArg: Args::SpecialsCount == 0");
    static_assert(Args::Last::IsRequest::Value == 0,
                  "viewOneArg: Args::Last::IsRequest::Value == 0");
    static_assert(Args::Last::IsRequestValue::Value == 0,
                  "viewOneArg: Args::Last::IsRequestValue::Value == 0");
    static_assert(Args::Last::IsRequestCLvalue::Value == 0,
                  "viewOneArg: Args::Last::IsRequestCLvalue::Value == 0");
    static_assert(Args::Last::IsResponder::Value == 0,
                  "viewOneArg: Args::Last::IsResponder::Value == 0");
    static_assert(Args::Last::IsResponderLvalue::Value == 0,
                  "viewOneArg: Args::Last::IsResponderLvalue::Value == 0");
    static_assert(Args::Last::IsResponderRvalue::Value == 0,
                  "viewOneArg: Args::Last::IsResponderRvalue::Value == 0");
    static_assert(Args::Last::IsSpecial::Value == 0,
                  "viewOneArg: Args::Last::IsSpecial::Value == 0");
    static_assert(Args::Last::IsSimple::Value,
                  "viewOneArg: Args::Last::IsSimple::Value");
    static_assert(Args::Last::Value,
                  "viewOneArg: Args::Last::Value");
    static_assert(Args::Last::StaticAssert,
                  "viewOneArg: Args::Last::StaticAssert");
    static_assert(std::is_same<Args::Last::Type, const quint64 &>::value,
                  "viewNonArg: std::is_same<Args::Last::Type, const quint64 &>");
    static_assert(Args::Value, "viewOneArg: Args::Value");
    static_assert(Args::StaticAssert, "viewOneArg: Args::StaticAssert");
}

void tst_QHttpServerRouter::viewHandlerTwoArgs()
{
    auto view = [] (const quint64 &, const QHttpServerResponder &) {
    };

    using ViewTraits = QHttpServerRouterViewTraits<decltype(view), true>;
    using Args = typename ViewTraits::Arguments;

    static_assert(Args::Count == 2,
                  "viewTwoArgs: Args::Count == 2");
    static_assert(Args::CapturableCount == 1,
                  "viewTwoArgs: Args::CapturableCount == 1");
    static_assert(Args::SpecialsCount == 1, "viewTwoArgs: Args::SpecialsCount == 0");

    using Arg0 = typename Args::template Arg<0>;
    static_assert(Arg0::IsRequest::Value == 0,
                  "viewTwoArgs: Args::Arg0::IsRequest::Value == 0");
    static_assert(Arg0::IsRequestCLvalue::Value == 0,
                  "viewTwoArgs: Args::Arg0::IsRequestCLvalue::Value == 0");
    static_assert(Arg0::IsRequestValue::Value == 0,
                  "viewTwoArgs: Args::Arg0::IsRequestValue::Value == 0");
    static_assert(Arg0::IsResponder::Value == 0,
                  "viewTwoArgs: Args::Arg0::IsResponder::Value == 0");
    static_assert(Arg0::IsResponderLvalue::Value == 0,
                  "viewTwoArgs: Args::Arg0::IsResponderLvalue::Value == 0");
    static_assert(Arg0::IsResponderRvalue::Value == 0,
                  "viewTwoArgs: Args::Arg0::IsResponderRvalue::Value == 0");
    static_assert(Arg0::IsSpecial::Value == 0,
                  "viewTwoArgs: Args::Arg0::IsSpecial::Value == 0");
    static_assert(Arg0::IsSimple::Value,
                  "viewTwoArgs: Args::Arg0::IsSimple::Value");
    static_assert(Arg0::Value,
                  "viewTwoArgs: Args::Arg0::Value");
    static_assert(Arg0::StaticAssert,
                  "viewTwoArgs: Args::Arg0::StaticAssert");
    static_assert(std::is_same<Arg0::Type, const quint64 &>::value,
                  "viewNonArg: std::is_same<Args::Arg0::Type, const quint64>");

    using Arg1 = typename Args::template Arg<1>;
    static_assert(Arg1::IsRequest::Value == 0,
                  "viewTwoArgs: Args::Arg1::IsRequest::Value == 0");
    static_assert(Arg1::IsRequestCLvalue::Value == 0,
                  "viewTwoArgs: Args::Arg1::IsRequestCLvalue::Value == 0");
    static_assert(Arg1::IsRequestValue::Value == 0,
                  "viewTwoArgs: Args::Arg1::IsRequestValue::Value == 0");
    static_assert(Arg1::IsResponder::Value == 0,
                  "viewTwoArgs: Args::Arg1::IsResponder::Value == 0");
    static_assert(Arg1::IsResponderLvalue::Value == 0,
                  "viewTwoArgs: Args::Arg1::IsResponderLvalue::Value == 0");
    static_assert(Arg1::IsResponderRvalue::Value == 0,
                  "viewTwoArgs: Args::Arg1::IsResponderRvalue::Value == 0");
    static_assert(Arg1::IsSpecial::Value == 0,
                  "viewTwoArgs: Args::Arg1::IsSpecial::Value == 0");
    static_assert(Arg1::IsSimple::Value == 0,
                  "viewTwoArgs: Args::Arg1::IsSimple::Value == 0");
    static_assert(Arg1::Value == 0,
                  "viewTwoArgs: Args::Arg1::Value");
    // StaticAssert is disabled in tests
    static_assert(Arg1::StaticAssert,
                  "viewOneArg: Args::Arg1::StaticAssert");
    static_assert(std::is_same<Arg1::Type, const QHttpServerResponder &>::value,
                  "viewTwoArgs: std::is_same<Args::Arg1::Type, const QHttpServerResponder &>)");

    static_assert(Args::Value == 0, "viewTwoArgs: Args::Value == 0");
    // StaticAssert is disabled in tests
    static_assert(Args::StaticAssert, "viewTwoArgs: Args::StaticAssert");
}

void tst_QHttpServerRouter::viewHandlerResponder()
{
    auto view = [] (QHttpServerResponder &) {
    };

    using ViewTraits = QHttpServerRouterViewTraits<decltype(view), true>;
    using Args = typename ViewTraits::Arguments;

    static_assert(Args::Count == 1,
                  "viewResponder: Args::Count == 1");
    static_assert(Args::CapturableCount == 0,
                  "viewResponder: Args::CapturableCount == 0");
    static_assert(Args::SpecialsCount == 1, "viewResponder: Args::SpecialsCount == 1");
    static_assert(Args::Last::IsRequest::Value == 0,
                  "viewResponder: Args::Last::IsRequest::Value == 0");
    static_assert(Args::Last::IsRequestCLvalue::Value == 0,
                  "viewResponder: Args::Last::IsRequestCLvalue::Value == 0");
    static_assert(Args::Last::IsRequestValue::Value == 0,
                  "viewResponder: Args::Last::IsRequestValue::Value == 0");
    static_assert(Args::Last::IsResponder::Value,
                  "viewResponder: Args::Last::IsResponder::Value");
    static_assert(Args::Last::IsResponderLvalue::Value,
                  "viewResponder: Args::Last::IsResponderLvalue::Value");
    static_assert(Args::Last::IsResponderRvalue::Value == 0,
                  "viewResponder: Args::Last::IsResponderRvalue::Value == 0");
    static_assert(Args::Last::IsSpecial::Value,
                  "viewResponder: Args::Last::IsSpecial::Value");
    static_assert(Args::Last::IsSimple::Value == 0,
                  "viewResponder: Args::Last::IsSimple::Value == 0");
    static_assert(Args::Last::Value,
                  "viewResponder: Args::Last::Value");
    static_assert(Args::Last::StaticAssert,
                  "viewResponder: Args::Last::StaticAssert");
    static_assert(std::is_same<Args::Last::Type, QHttpServerResponder &>::value,
                  "viewNonArg: std::is_same<Args::Last::Type, QHttpServerResponder &>");
    static_assert(Args::Value, "viewResponder: Args::Value");
    static_assert(Args::StaticAssert, "viewResponder: Args::StaticAssert");
}

void tst_QHttpServerRouter::viewHandlerRequest()
{
    auto view = [] (const QHttpServerRequest &) {
    };

    using ViewTraits = QHttpServerRouterViewTraits<decltype(view), true>;
    using Args = typename ViewTraits::Arguments;

    static_assert(Args::Count == 1,
                  "viewResponder: Args::Count == 1");
    static_assert(Args::CapturableCount == 0,
                  "viewResponder: Args::CapturableCount == 0");
    static_assert(Args::SpecialsCount == 1, "viewResponder: Args::SpecialsCount == 1");
    static_assert(Args::Last::IsRequest::Value,
                  "viewResponder: Args::Last::IsRequest::Value");
    static_assert(Args::Last::IsRequestCLvalue::Value,
                  "viewResponder: Args::Last::IsRequestCLvalue::Value");
    static_assert(Args::Last::IsRequestValue::Value == 0,
                  "viewResponder: Args::Last::IsRequestValue::Value == 0");
    static_assert(Args::Last::IsResponder::Value == 0,
                  "viewResponder: Args::Last::IsResponder::Value == 0");
    static_assert(Args::Last::IsResponderLvalue::Value == 0,
                  "viewResponder: Args::Last::IsResponderLvalue::Value == 0");
    static_assert(Args::Last::IsResponderRvalue::Value == 0,
                  "viewResponder: Args::Last::IsResponderRvalue::Value == 0");
    static_assert(Args::Last::IsSpecial::Value,
                  "viewResponder: Args::Last::IsSpecial::Value");
    static_assert(Args::Last::IsSimple::Value == 0,
                  "viewResponder: Args::Last::IsSimple::Value == 0");
    static_assert(Args::Last::Value,
                  "viewResponder: Args::Last::Value");
    static_assert(Args::Last::StaticAssert,
                  "viewResponder: Args::Last::StaticAssert");
    static_assert(std::is_same<Args::Last::Type, const QHttpServerRequest &>::value,
                  "viewNonArg: std::is_same<Args::Last::Type, const QHttpServerRequest &>");
    static_assert(Args::Value, "viewResponder: Args::Value");
    static_assert(Args::StaticAssert, "viewResponder: Args::StaticAssert");
}

void tst_QHttpServerRouter::viewHandlerLastTwoSpecials()
{
    auto view = [] (const QHttpServerRequest &, QHttpServerResponder &) {
    };

    using ViewTraits = QHttpServerRouterViewTraits<decltype(view), true>;
    using Args = typename ViewTraits::Arguments;

    static_assert(Args::Count == 2,
                  "viewTwoSpecialArgs: Args::Count == 2");
    static_assert(Args::CapturableCount == 0,
                  "viewTwoSpecialArgs: Args::CapturableCount == 1");
    static_assert(Args::SpecialsCount == 2, "viewTwoSpecialArgs: Args::SpecialsCount == 0");

    using Arg0 = typename Args::template Arg<0>;
    static_assert(Arg0::IsRequest::Value,
                  "viewTwoSpecialArgs: Args::Arg0::IsRequest::Value");
    static_assert(Arg0::IsRequestCLvalue::Value,
                  "viewTwoSpecialArgs: Args::Arg0::IsRequestCLvalue::Value");
    static_assert(Arg0::IsRequestValue::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg0::IsRequestValue::Value == 0");
    static_assert(Arg0::IsResponder::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg0::IsResponder::Value == 0");
    static_assert(Arg0::IsResponderLvalue::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg0::IsResponderLvalue::Value == 0");
    static_assert(Arg0::IsResponderRvalue::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg0::IsResponderRvalue::Value == 0");
    static_assert(Arg0::IsSpecial::Value,
                  "viewTwoSpecialArgs: Args::Arg0::IsSpecial::Value");
    static_assert(Arg0::IsSimple::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg0::IsSimple::Value == 0");
    static_assert(Arg0::Value,
                  "viewTwoSpecialArgs: Args::Arg0::Value");
    // StaticAssert is disabled in tests
    static_assert(Arg0::StaticAssert,
                  "viewTwoSpecialArgs: Args::Arg0::StaticAssert");
    static_assert(std::is_same<Arg0::Type, const QHttpServerRequest &>::value,
                  "viewNonArg: std::is_same<Args::Arg0::Type, const QHttpServerRequest &>");

    using Arg1 = typename Args::template Arg<1>;
    static_assert(Arg1::IsRequest::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg1::IsRequest::Value == 0");
    static_assert(Arg1::IsRequestCLvalue::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg1::IsRequestCLvalue::Value == 0");
    static_assert(Arg1::IsRequestValue::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg1::IsRequestValue::Value == 0");
    static_assert(Arg1::IsResponder::Value,
                  "viewTwoSpecialArgs: Args::Arg1::IsResponder::Value");
    static_assert(Arg1::IsResponderLvalue::Value,
                  "viewTwoSpecialArgs: Args::Arg1::IsResponderLvalue::Value");
    static_assert(Arg1::IsResponderRvalue::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg1::IsResponderRvalue::Value == 0");
    static_assert(Arg1::IsSpecial::Value,
                  "viewTwoSpecialArgs: Args::Arg1::IsSpecial::Value");
    static_assert(Arg1::IsSimple::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg1::IsSimple::Value == 0");
    static_assert(Arg1::Value,
                  "viewTwoSpecialArgs: Args::Arg1::Value");
    static_assert(Arg1::StaticAssert,
                  "viewTwoSpecialArgs: Args::Arg1::StaticAssert");
    static_assert(std::is_same<Arg1::Type, QHttpServerResponder &>::value,
                  "viewTwoSpecialArgs: std::is_same<Args::Arg1::Type, QHttpServerResponder &>");

    static_assert(Args::Value, "viewTwoSpecialArgs: Args::Value");
    // StaticAssert is disabled in tests
    static_assert(Args::StaticAssert, "viewTwoSpecialArgs: Args::StaticAssert");
}

void tst_QHttpServerRouter::viewHandlerLastTwoSpecialsValueRValueRef()
{
    auto view = [] (QHttpServerRequest, QHttpServerResponder &&) {
    };

    using ViewTraits = QHttpServerRouterViewTraits<decltype(view), true>;
    using Args = typename ViewTraits::Arguments;

    static_assert(Args::Count == 2,
                  "viewTwoSpecialArgs: Args::Count == 2");
    static_assert(Args::CapturableCount == 0,
                  "viewTwoSpecialArgs: Args::CapturableCount == 1");
    static_assert(Args::SpecialsCount == 2, "viewTwoSpecialArgs: Args::SpecialsCount == 0");

    using Arg0 = typename Args::template Arg<0>;
    static_assert(Arg0::IsRequest::Value,
                  "viewTwoSpecialArgs: Args::Arg0::IsRequest::Value");
    static_assert(Arg0::IsRequestCLvalue::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg0::IsRequestCLvalue::Value == 0");
    static_assert(Arg0::IsRequestValue::Value,
                  "viewTwoSpecialArgs: Args::Arg0::IsRequestValue::Value");
    static_assert(Arg0::IsResponder::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg0::IsResponder::Value == 0");
    static_assert(Arg0::IsResponderLvalue::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg0::IsResponderLvalue::Value == 0");
    static_assert(Arg0::IsResponderRvalue::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg0::IsResponderRvalue::Value == 0");
    static_assert(Arg0::IsSpecial::Value,
                  "viewTwoSpecialArgs: Args::Arg0::IsSpecial::Value");
    static_assert(Arg0::IsSimple::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg0::IsSimple::Value == 0");
    static_assert(Arg0::Value,
                  "viewTwoSpecialArgs: Args::Arg0::Value");
    // StaticAssert is disabled in tests
    static_assert(Arg0::StaticAssert,
                  "viewTwoSpecialArgs: Args::Arg0::StaticAssert");
    static_assert(std::is_same<Arg0::Type, QHttpServerRequest>::value,
                  "viewNonArg: std::is_same<Args::Arg0::Type, QHttpServerRequest>");

    using Arg1 = typename Args::template Arg<1>;
    static_assert(Arg1::IsRequest::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg1::IsRequest::Value == 0");
    static_assert(Arg1::IsRequestCLvalue::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg1::IsRequestCLvalue::Value == 0");
    static_assert(Arg1::IsRequestValue::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg1::IsRequestValue::Value == 0");
    static_assert(Arg1::IsResponder::Value,
                  "viewTwoSpecialArgs: Args::Arg1::IsResponder::Value");
    static_assert(Arg1::IsResponderLvalue::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg1::IsResponderLvalue::Value == 0");
    static_assert(Arg1::IsResponderRvalue::Value,
                  "viewTwoSpecialArgs: Args::Arg1::IsResponderRvalue::Value");
    static_assert(Arg1::IsSpecial::Value,
                  "viewTwoSpecialArgs: Args::Arg1::IsSpecial::Value");
    static_assert(Arg1::IsSimple::Value == 0,
                  "viewTwoSpecialArgs: Args::Arg1::IsSimple::Value == 0");
    static_assert(Arg1::Value,
                  "viewTwoSpecialArgs: Args::Arg1::Value");
    static_assert(Arg1::StaticAssert,
                  "viewTwoSpecialArgs: Args::Arg1::StaticAssert");
    static_assert(std::is_same<Arg1::Type, QHttpServerResponder &&>::value,
                  "viewTwoSpecialArgs: std::is_same<Args::Arg1::Type, QHttpServerResponder &&>");

    static_assert(Args::Value, "viewTwoSpecialArgs: Args::Value");
    // StaticAssert is disabled in tests
    static_assert(Args::StaticAssert, "viewTwoSpecialArgs: Args::StaticAssert");
}


QT_END_NAMESPACE

QTEST_MAIN(tst_QHttpServerRouter)

#include "tst_qhttpserverrouter.moc"
