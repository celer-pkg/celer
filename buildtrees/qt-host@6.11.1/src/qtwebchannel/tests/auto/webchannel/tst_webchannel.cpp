// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// Copyright (C) 2019 Menlo Systems GmbH, author Arno Rehn <a.rehn@menlosystems.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "tst_webchannel.h"

#include <QtWebChannel/qwebchannel.h>
#include <QtWebChannel/private/qwebchannel_p.h>
#include <QtWebChannel/private/qmetaobjectpublisher_p.h>

#include <QtTest>
#include <QtTest/private/qpropertytesthelper_p.h>
#ifdef WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE
#include <QJSEngine>
#endif

#include <QPromise>
#include <QTimer>

#ifdef WEBCHANNEL_TESTS_CAN_USE_CONCURRENT
#include <QtConcurrent>
#endif

#include <memory>
#include <optional>
#include <vector>

QT_USE_NAMESPACE

#ifdef WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE
class TestJSEngine;

class TestEngineTransport : public QWebChannelAbstractTransport
{
    Q_OBJECT
public:
    TestEngineTransport(TestJSEngine *);
    void sendMessage(const QJsonObject &message) override;

    Q_INVOKABLE void channelSetupReady();
    Q_INVOKABLE void send(const QByteArray &message);
private:
    TestJSEngine *m_testEngine;
};

class ConsoleLogger : public QObject
{
    Q_OBJECT
public:
    ConsoleLogger(QObject *parent = 0);

    Q_INVOKABLE void log(const QString &text);
    Q_INVOKABLE void error(const QString &text);

    int errorCount() const { return m_errCount; }
    int logCount() const { return m_logCount; }
    QString lastError() const { return m_lastError; }

private:
    int m_errCount;
    int m_logCount;
    QString m_lastError;

};



ConsoleLogger::ConsoleLogger(QObject *parent)
    : QObject(parent)
    , m_errCount(0)
    , m_logCount(0)
{
}

void ConsoleLogger::log(const QString &text)
{
    m_logCount++;
    qDebug("LOG: %s", qPrintable(text));
}

void ConsoleLogger::error(const QString &text)
{
    m_errCount++;
    m_lastError = text;
    qWarning("ERROR: %s", qPrintable(text));
}


// A test JS engine with convenience integration with WebChannel.
class TestJSEngine : public QJSEngine
{
    Q_OBJECT
public:
    TestJSEngine();

    TestEngineTransport *transport() const;
    ConsoleLogger *logger() const;
    void initWebChannelJS();

signals:
    void channelSetupReady(TestEngineTransport *transport);

private:
    TestEngineTransport *m_transport;
    ConsoleLogger *m_logger;
};

TestEngineTransport::TestEngineTransport(TestJSEngine *engine)
    : QWebChannelAbstractTransport(engine)
    , m_testEngine(engine)
{
}

void TestEngineTransport::sendMessage(const QJsonObject &message)
{
    QByteArray json = QJsonDocument(message).toJson(QJsonDocument::Compact);
    QJSValue callback = m_testEngine->evaluate(QStringLiteral("transport.onmessage"));
    Q_ASSERT(callback.isCallable());
    QJSValue arg = m_testEngine->newObject();
    QJSValue data = m_testEngine->evaluate(QString::fromLatin1("JSON.parse('%1');").arg(QString::fromUtf8(json)));
    Q_ASSERT(!data.isError());
    arg.setProperty(QStringLiteral("data"), data);
    QJSValue val = callback.call((QJSValueList() << arg));
    Q_ASSERT(!val.isError());
}

void TestEngineTransport::channelSetupReady()
{
    emit m_testEngine->channelSetupReady(m_testEngine->transport());
}

void TestEngineTransport::send(const QByteArray &message)
{
    QJsonDocument doc(QJsonDocument::fromJson(message));
    emit messageReceived(doc.object(), this);
}


TestJSEngine::TestJSEngine()
    : m_transport(new TestEngineTransport(this))
    , m_logger(new ConsoleLogger(this))
{
    globalObject().setProperty("transport", newQObject(m_transport));
    globalObject().setProperty("console", newQObject(m_logger));

    QString webChannelJSPath(QStringLiteral(":/qtwebchannel/qwebchannel.js"));
    QFile webChannelJS(webChannelJSPath);
    if (!webChannelJS.open(QFile::ReadOnly))
        qFatal("Error opening qwebchannel.js");
    QString source(QString::fromUtf8(webChannelJS.readAll()));
    evaluate(source, webChannelJSPath);
}

TestEngineTransport *TestJSEngine::transport() const
{
    return m_transport;
}

ConsoleLogger *TestJSEngine::logger() const
{
    return m_logger;
}

void TestJSEngine::initWebChannelJS()
{
    globalObject().setProperty(QStringLiteral("channel"), newObject());
    QJSValue channel = evaluate(QStringLiteral("channel = new QWebChannel(transport, function(channel) { transport.channelSetupReady();});"));
    Q_ASSERT(!channel.isError());
}

#endif // WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE

namespace {
QVariantList convert_to_js(const TestStructVector &list)
{
    QVariantList ret;
    ret.reserve(list.size());
    std::transform(list.begin(), list.end(), std::back_inserter(ret), [](const TestStruct &value) -> QVariant {
        QVariantMap map;
        map["foo"] = value.foo;
        map["bar"] = value.bar;
        return map;
    });
    return ret;
}
}

#if QT_CONFIG(future)
QFuture<int> TestObject::futureIntResult() const
{
    return QtFuture::makeReadyValueFuture(42);
}

QFuture<int> TestObject::futureDelayedIntResult() const
{
    QPromise<int> p;
    const auto f = p.future();
    p.start();
    QTimer::singleShot(10, this, [p=std::move(p)]() mutable {
        p.addResult(7);
        p.finish();
    });
    return f;
}

#ifdef WEBCHANNEL_TESTS_CAN_USE_CONCURRENT
QFuture<int> TestObject::futureIntResultFromThread() const
{
    return QtConcurrent::run([] {
        return 1337;
    });
}
#endif

QFuture<void> TestObject::futureVoidResult() const
{
    return QtFuture::makeReadyVoidFuture();
}

QFuture<QString> TestObject::futureStringResult() const
{
    return QtFuture::makeReadyValueFuture<QString>("foo");
}

QFuture<int> TestObject::cancelledFuture() const
{
    QPromise<int> p;
    auto f = p.future();
    p.start();
    f.cancel();
    Q_ASSERT(f.isCanceled());
    return f;
}

QFuture<int> TestObject::failedFuture() const
{
    return QtFuture::makeExceptionalFuture<int>(QException{});
}
#endif

TestWebChannel::TestWebChannel(QObject *parent)
    : QObject(parent)
    , m_dummyTransport(new DummyTransport(this))
    , m_lastInt(0)
    , m_lastBool(false)
    , m_lastDouble(0)
{
    qRegisterMetaType<TestStruct>();
    Q_ASSERT(QMetaType::fromType<TestStruct>().isEqualityComparable());
    Q_ASSERT(QMetaType::fromType<TestStruct>().hasRegisteredDebugStreamOperator());

    qRegisterMetaType<TestStructVector>();
    QMetaType::registerConverter<TestStructVector, QVariantList>(convert_to_js);

    QMetaType::registerConverter<TestStruct, QString>();

    QMetaType::registerConverter<TestStruct, QJsonValue>(
        [](const TestStruct &s) {
            return QJsonObject {
                { "__type__", "TestStruct" },
                { "foo", s.foo },
                { "bar", s.bar },
            };
        });

    QMetaType::registerConverter<QJsonValue, TestStruct>(
        [](const QJsonValue &value) -> std::optional<TestStruct> {
            const auto object = value.toObject();
            if (object.value("__type__").toString() != QStringLiteral("TestStruct"))
                return std::nullopt;

            return TestStruct {
                object.value("foo").toInt(),
                object.value("bar").toInt(),
            };
        });
}

TestWebChannel::~TestWebChannel()
{

}

int TestWebChannel::readInt() const
{
    return m_lastInt;
}

void TestWebChannel::setInt(int i)
{
    m_lastInt = i;
    emit lastIntChanged();
}

bool TestWebChannel::readBool() const
{
    return m_lastBool;
}

void TestWebChannel::setBool(bool b)
{
    m_lastBool = b;
    emit lastBoolChanged();
}

double TestWebChannel::readDouble() const
{
    return m_lastDouble;
}

void TestWebChannel::setDouble(double d)
{
    m_lastDouble = d;
    emit lastDoubleChanged();
}

QVariant TestWebChannel::readVariant() const
{
    return m_lastVariant;
}

void TestWebChannel::setVariant(const QVariant &v)
{
    m_lastVariant = v;
    emit lastVariantChanged();
}

QJsonValue TestWebChannel::readJsonValue() const
{
    return m_lastJsonValue;
}

void TestWebChannel::setJsonValue(const QJsonValue& v)
{
    m_lastJsonValue = v;
    emit lastJsonValueChanged();
}

QUrl TestWebChannel::readUrl() const
{
    return m_lastUrl;
}

void TestWebChannel::setUrl(const QUrl& u)
{
    m_lastUrl = u;
}

QJsonObject TestWebChannel::readJsonObject() const
{
    return m_lastJsonObject;
}

void TestWebChannel::setJsonObject(const QJsonObject& v)
{
    m_lastJsonObject = v;
    emit lastJsonObjectChanged();
}

QJsonArray TestWebChannel::readJsonArray() const
{
    return m_lastJsonArray;
}

void TestWebChannel::setJsonArray(const QJsonArray& v)
{
    m_lastJsonArray = v;
    emit lastJsonArrayChanged();
}

int TestWebChannel::readOverload(int i)
{
    return i + 1;
}

QString TestWebChannel::readOverload(const QString &arg)
{
    return arg.toUpper();
}

QString TestWebChannel::readOverload(const QString &arg, int i)
{
    return arg.toUpper() + QString::number(i + 1);
}

void TestWebChannel::testRegisterObjects()
{
    QWebChannel channel;
    QObject plain;

    QHash<QString, QObject*> objects;
    objects[QStringLiteral("plain")] = &plain;
    objects[QStringLiteral("channel")] = &channel;
    objects[QStringLiteral("publisher")] = channel.d_func()->publisher;
    objects[QStringLiteral("test")] = this;

    channel.registerObjects(objects);
}

void TestWebChannel::testDeregisterObjects()
{
    QWebChannel channel;
    TestObject testObject;
    testObject.setObjectName("myTestObject");


    channel.registerObject(testObject.objectName(), &testObject);

    channel.connectTo(m_dummyTransport);
    channel.d_func()->publisher->initializeClient(m_dummyTransport);

    QJsonObject connectMessage =
            QJsonDocument::fromJson(("{\"type\": 7,"
                                    "\"object\": \"myTestObject\","
                                    "\"signal\": " + QString::number(testObject.metaObject()->indexOfSignal("sig1()"))
                                    + "}").toLatin1()).object();
    channel.d_func()->publisher->handleMessage(connectMessage, m_dummyTransport);

    emit testObject.sig1();
    channel.deregisterObject(&testObject);
    emit testObject.sig1();
}

void TestWebChannel::testDeregisterObjectAtStart()
{
    QWebChannel channel;
    QVERIFY(channel.registeredObjects().isEmpty());

    TestObject testObject;
    testObject.setObjectName("myTestObject");

    channel.registerObject(testObject.objectName(), &testObject);
    QCOMPARE(channel.registeredObjects().size(), 1);

    channel.deregisterObject(&testObject);
    QVERIFY(channel.registeredObjects().isEmpty());
}

void TestWebChannel::testInfoForObject()
{
    TestObject obj;
    obj.setObjectName("myTestObject");

    QWebChannel channel;
    const QJsonObject info = channel.d_func()->publisher->classInfoForObject(&obj, m_dummyTransport);

    QCOMPARE(info.keys(), QStringList() << "enums" << "methods" << "properties" << "signals");

    { // enums
        QJsonObject fooEnum;
        fooEnum["Asdf"] = TestObject::Asdf;
        fooEnum["Bar"] = TestObject::Bar;
        QJsonObject testFlags;
        testFlags["FirstFlag"] = static_cast<int>(TestObject::FirstFlag);
        testFlags["SecondFlag"] = static_cast<int>(TestObject::SecondFlag);
        QJsonObject expected;
        expected["Foo"] = fooEnum;
        expected["TestFlags"] = testFlags;
        QCOMPARE(info["enums"].toObject(), expected);
    }

    QJsonArray expected;
    auto addMethod = [&expected, &obj](const QString &name, const char *signature, bool addName = true) {
        const auto index = obj.metaObject()->indexOfMethod(signature);
        QVERIFY2(index != -1, signature);
        if (addName)
            expected.append(QJsonArray{name, index});
        expected.append(QJsonArray{QString::fromUtf8(signature), index});
    };
    { // methods & slots
        expected = {};
        addMethod(QStringLiteral("deleteLater"), "deleteLater()");
        addMethod(QStringLiteral("slot1"), "slot1()");
        addMethod(QStringLiteral("slot2"), "slot2(QString)");
        addMethod(QStringLiteral("setReturnedObject"), "setReturnedObject(TestObject*)");
        addMethod(QStringLiteral("setObjectProperty"), "setObjectProperty(QObject*)");
        addMethod(QStringLiteral("setProp"), "setProp(QString)");
        addMethod(QStringLiteral("fire"), "fire()");
        addMethod(QStringLiteral("overload"), "overload(double)");
        addMethod(QStringLiteral("overload"), "overload(int)", false);
        addMethod(QStringLiteral("overload"), "overload(QObject*)", false);
        addMethod(QStringLiteral("overload"), "overload(QString)", false);
        addMethod(QStringLiteral("overload"), "overload(QString,int)", false);
        addMethod(QStringLiteral("overload"), "overload(QJsonArray)", false);
        addMethod(QStringLiteral("setStringProperty"), "setStringProperty(QString)");
        addMethod(QStringLiteral("bindableStringProperty"), "bindableStringProperty()");
        addMethod(QStringLiteral("getStringProperty"), "getStringProperty()");
        addMethod(QStringLiteral("bindStringPropertyToStringProperty2"), "bindStringPropertyToStringProperty2()");
        addMethod(QStringLiteral("setStringProperty2"), "setStringProperty2(QString)");
        addMethod(QStringLiteral("method1"), "method1()");
#if QT_CONFIG(future)
        addMethod(QStringLiteral("futureIntResult"), "futureIntResult()");
        addMethod(QStringLiteral("futureDelayedIntResult"), "futureDelayedIntResult()");
#ifdef WEBCHANNEL_TESTS_CAN_USE_CONCURRENT
        addMethod(QStringLiteral("futureIntResultFromThread"), "futureIntResultFromThread()");
#endif
        addMethod(QStringLiteral("futureVoidResult"), "futureVoidResult()");
        addMethod(QStringLiteral("futureStringResult"), "futureStringResult()");
        addMethod(QStringLiteral("cancelledFuture"), "cancelledFuture()");
        addMethod(QStringLiteral("failedFuture"), "failedFuture()");
#endif
        QCOMPARE(info["methods"].toArray(), expected);
    }

    { // signals
        expected = {};
        addMethod(QStringLiteral("destroyed"), "destroyed(QObject*)");
        addMethod(QStringLiteral("destroyed"), "destroyed()", false);
        addMethod(QStringLiteral("sig1"), "sig1()");
        addMethod(QStringLiteral("sig2"), "sig2(QString)");
        addMethod(QStringLiteral("replay"), "replay()");
        addMethod(QStringLiteral("overloadSignal"), "overloadSignal(int)");
        addMethod(QStringLiteral("overloadSignal"), "overloadSignal(float)", false);
        QCOMPARE(info["signals"].toArray(), expected);
    }

    { // properties
        QJsonArray expected;
        {
            QJsonArray property;
            property.append(obj.metaObject()->indexOfProperty("objectName"));
            property.append(QStringLiteral("objectName"));
            {
                QJsonArray signal;
                signal.append(1);
                signal.append(obj.metaObject()->indexOfMethod("objectNameChanged(QString)"));
                property.append(signal);
            }
            property.append(obj.objectName());
            expected.append(property);
        }
        {
            QJsonArray property;
            property.append(obj.metaObject()->indexOfProperty("foo"));
            property.append(QStringLiteral("foo"));
            {
                QJsonArray signal;
                property.append(signal);
            }
            property.append(obj.foo());
            expected.append(property);
        }
        {
            QJsonArray property;
            property.append(obj.metaObject()->indexOfProperty("asdf"));
            property.append(QStringLiteral("asdf"));
            {
                QJsonArray signal;
                signal.append(1);
                signal.append(obj.metaObject()->indexOfMethod("asdfChanged()"));
                property.append(signal);
            }
            property.append(obj.asdf());
            expected.append(property);
        }
        {
            QJsonArray property;
            property.append(obj.metaObject()->indexOfProperty("bar"));
            property.append(QStringLiteral("bar"));
            {
                QJsonArray signal;
                signal.append(QStringLiteral("theBarHasChanged"));
                signal.append(obj.metaObject()->indexOfMethod("theBarHasChanged()"));
                property.append(signal);
            }
            property.append(obj.bar());
            expected.append(property);
        }
        {
            QJsonArray property;
            property.append(obj.metaObject()->indexOfProperty("objectProperty"));
            property.append(QStringLiteral("objectProperty"));
            {
                QJsonArray signal;
                signal.append(1);
                signal.append(obj.metaObject()->indexOfMethod("objectPropertyChanged()"));
                property.append(signal);
            }
            property.append(QJsonValue::fromVariant(QVariant::fromValue(obj.objectProperty())));
            expected.append(property);
        }
        {
            QJsonArray property;
            property.append(obj.metaObject()->indexOfProperty("returnedObject"));
            property.append(QStringLiteral("returnedObject"));
            {
                QJsonArray signal;
                signal.append(1);
                signal.append(obj.metaObject()->indexOfMethod("returnedObjectChanged()"));
                property.append(signal);
            }
            property.append(QJsonValue::fromVariant(QVariant::fromValue(obj.returnedObject())));
            expected.append(property);
        }
        {
            QJsonArray property;
            property.append(obj.metaObject()->indexOfProperty("prop"));
            property.append(QStringLiteral("prop"));
            {
                QJsonArray signal;
                signal.append(1);
                signal.append(obj.metaObject()->indexOfMethod("propChanged(QString)"));
                property.append(signal);
            }
            property.append(QJsonValue::fromVariant(QVariant::fromValue(obj.prop())));
            expected.append(property);
        }
        {
            QJsonArray property;
            property.append(obj.metaObject()->indexOfProperty("stringProperty"));
            property.append(QStringLiteral("stringProperty"));
            {
                QJsonArray signal;
                property.append(signal);
            }
            property.append(QJsonValue::fromVariant(QVariant::fromValue(obj.readStringProperty())));
            expected.append(property);
        }
        QCOMPARE(info["properties"].toArray(), expected);
    }
}

void TestWebChannel::testInvokeMethodConversion()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    QJsonArray args;
    args.append(QJsonValue(1000));

    {
        channel.d_func()->publisher->invokeMethod(this, "setInt", args);
        QCOMPARE(m_lastInt, args.at(0).toInt());
        int getterMethod = metaObject()->indexOfMethod("readInt()");
        QVERIFY(getterMethod != -1);
        auto retVal = channel.d_func()->publisher->invokeMethod(this, getterMethod, {});
        QCOMPARE(retVal, args.at(0).toVariant());
    }
    {
        QJsonArray args;
        args.append(QJsonValue(!m_lastBool));
        channel.d_func()->publisher->invokeMethod(this, "setBool", args);
        QCOMPARE(m_lastBool, args.at(0).toBool());
        int getterMethod = metaObject()->indexOfMethod("readBool()");
        QVERIFY(getterMethod != -1);
        auto retVal = channel.d_func()->publisher->invokeMethod(this, getterMethod, {});
        QCOMPARE(retVal, args.at(0).toVariant());
    }
    {
        channel.d_func()->publisher->invokeMethod(this, "setDouble", args);
        QCOMPARE(m_lastDouble, args.at(0).toDouble());
        int getterMethod = metaObject()->indexOfMethod("readDouble()");
        QVERIFY(getterMethod != -1);
        auto retVal = channel.d_func()->publisher->invokeMethod(this, getterMethod, {});
        QCOMPARE(retVal, args.at(0).toVariant());
    }
    {
        channel.d_func()->publisher->invokeMethod(this, "setVariant", args);
        QCOMPARE(m_lastVariant, args.at(0).toVariant());
        int getterMethod = metaObject()->indexOfMethod("readVariant()");
        QVERIFY(getterMethod != -1);
        auto retVal = channel.d_func()->publisher->invokeMethod(this, getterMethod, {});
        QCOMPARE(retVal, args.at(0).toVariant());
    }
    {
        channel.d_func()->publisher->invokeMethod(this, "setJsonValue", args);
        QCOMPARE(m_lastJsonValue, args.at(0));
        int getterMethod = metaObject()->indexOfMethod("readJsonValue()");
        QVERIFY(getterMethod != -1);
        auto retVal = channel.d_func()->publisher->invokeMethod(this, getterMethod, {});
        QCOMPARE(retVal, QVariant(args.at(0)));
    }
    {
        QJsonObject object;
        object["foo"] = QJsonValue(123);
        object["bar"] = QJsonValue(4.2);
        args[0] = object;
        channel.d_func()->publisher->invokeMethod(this, "setJsonObject", args);
        QCOMPARE(m_lastJsonObject, object);
        int getterMethod = metaObject()->indexOfMethod("readJsonObject()");
        QVERIFY(getterMethod != -1);
        auto retVal = channel.d_func()->publisher->invokeMethod(this, getterMethod, {});
        QCOMPARE(retVal, QVariant::fromValue(object));
    }
    {
        QJsonArray array;
        array << QJsonValue(123);
        array <<  QJsonValue(4.2);
        args[0] = array;
        channel.d_func()->publisher->invokeMethod(this, "setJsonArray", args);
        QCOMPARE(m_lastJsonArray, array);
        int getterMethod = metaObject()->indexOfMethod("readJsonArray()");
        QVERIFY(getterMethod != -1);
        auto retVal = channel.d_func()->publisher->invokeMethod(this, getterMethod, {});
        QCOMPARE(retVal, QVariant::fromValue(array));
    }
    {
        args[0] = QJsonValue::fromVariant(QUrl("aviancarrier:/ok"));
        channel.d_func()->publisher->invokeMethod(this, "setUrl", args);
        QVERIFY(m_lastUrl.isValid());
        int getterMethod = metaObject()->indexOfMethod("readUrl()");
        QVERIFY(getterMethod != -1);
        auto retVal = channel.d_func()->publisher->invokeMethod(this, getterMethod, {});
        QCOMPARE(retVal, args.at(0).toVariant().toUrl());
    }
}

void TestWebChannel::testFunctionOverloading()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    // all method calls will use the first method's index
    const auto method1 = metaObject()->indexOfMethod("readOverload(int)");
    QVERIFY(method1 != -1);
    const auto method2 = metaObject()->indexOfMethod("readOverload(QString)");
    QVERIFY(method2 != -1);
    QVERIFY(method1 < method2);
    const auto method3 = metaObject()->indexOfMethod("readOverload(QString,int)");
    QVERIFY(method3 != -1);
    QVERIFY(method2 < method3);

    { // int
        const auto retVal = channel.d_func()->publisher->invokeMethod(this, method1, QJsonArray{1000});
        QCOMPARE(retVal.toInt(), 1001);
    }
    { // QString
        const auto retVal = channel.d_func()->publisher->invokeMethod(this, method2, QJsonArray{QStringLiteral("hello world")});
        QCOMPARE(retVal.toString(), QStringLiteral("HELLO WORLD"));
    }
    { // QString, int
        const auto retVal = channel.d_func()->publisher->invokeMethod(this, method3, QJsonArray{QStringLiteral("the answer is "), 41});
        QCOMPARE(retVal.toString(), QStringLiteral("THE ANSWER IS 42"));
    }
}

void TestWebChannel::testSetPropertyConversion()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    {
        int property = metaObject()->indexOfProperty("lastInt");
        QVERIFY(property != -1);
        channel.d_func()->publisher->setProperty(this, property, QJsonValue(42));
        QCOMPARE(m_lastInt, 42);
    }
    {
        int property = metaObject()->indexOfProperty("lastBool");
        QVERIFY(property != -1);
        bool newValue = !m_lastBool;
        channel.d_func()->publisher->setProperty(this, property, QJsonValue(newValue));
        QCOMPARE(m_lastBool, newValue);
    }
    {
        int property = metaObject()->indexOfProperty("lastDouble");
        QVERIFY(property != -1);
        channel.d_func()->publisher->setProperty(this, property, QJsonValue(-4.2));
        QCOMPARE(m_lastDouble, -4.2);
    }
    {
        int property = metaObject()->indexOfProperty("lastVariant");
        QVERIFY(property != -1);
        QVariant variant("foo bar asdf");
        channel.d_func()->publisher->setProperty(this, property, QJsonValue::fromVariant(variant));
        QCOMPARE(m_lastVariant, variant);
    }
    {
        int property = metaObject()->indexOfProperty("lastJsonValue");
        QVERIFY(property != -1);
        QJsonValue value("asdf asdf");
        channel.d_func()->publisher->setProperty(this, property, value);
        QCOMPARE(m_lastJsonValue, value);
    }
    {
        int property = metaObject()->indexOfProperty("lastJsonArray");
        QVERIFY(property != -1);
        QJsonArray array;
        array << QJsonValue(-123);
        array <<  QJsonValue(-42);
        channel.d_func()->publisher->setProperty(this, property, array);
        QCOMPARE(m_lastJsonArray, array);
    }
    {
        int property = metaObject()->indexOfProperty("lastJsonObject");
        QVERIFY(property != -1);
        QJsonObject object;
        object["foo"] = QJsonValue(-123);
        object["bar"] = QJsonValue(-4.2);
        channel.d_func()->publisher->setProperty(this, property, object);
        QCOMPARE(m_lastJsonObject, object);
    }
}

void TestWebChannel::testInvokeMethodOverloadResolution()
{
    QWebChannel channel;
    TestObject testObject;
    TestObject exportedObject;
    channel.registerObject("test", &exportedObject);
    channel.connectTo(m_dummyTransport);

    QVariant result;
    QMetaObjectPublisher *publisher = channel.d_func()->publisher;

    {
        result = publisher->invokeMethod(&testObject, "overload", { 41.0 });
        QVERIFY(result.userType() == QMetaType::Double);
        QCOMPARE(result.toDouble(), 42.0);
    }
    {
        // In JavaScript, there's only 'double', so this should always invoke the 'double' overload
        result = publisher->invokeMethod(&testObject, "overload", { 41 });
        QVERIFY(result.userType() == QMetaType::Double);
        QCOMPARE(result.toDouble(), 42);
    }
    {
        QJsonObject wrappedObject { {"id", "test"} };
        result = publisher->invokeMethod(&testObject, "overload", { wrappedObject });
        QCOMPARE(result.value<TestObject*>(), &exportedObject);
    }
    {
        result = publisher->invokeMethod(&testObject, "overload", { "hello world" });
        QCOMPARE(result.toString(), QStringLiteral("HELLO WORLD"));
    }
    {
        result = publisher->invokeMethod(&testObject, "overload", { "the answer is ", 41 });
        QCOMPARE(result.toString(), QStringLiteral("THE ANSWER IS 42"));
    }
    {
        QJsonArray args;
        args.append(QJsonArray { "foobar", 42 });
        result = publisher->invokeMethod(&testObject, "overload", args);
        QCOMPARE(result.toString(), QStringLiteral("42foobar"));
    }
}

void TestWebChannel::testDisconnect()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);
    channel.disconnectFrom(m_dummyTransport);
    m_dummyTransport->emitMessageReceived(QJsonObject());
}

void TestWebChannel::testWrapRegisteredObject()
{
    QWebChannel channel;
    TestObject obj;
    obj.setObjectName("myTestObject");

    channel.registerObject(obj.objectName(), &obj);
    channel.connectTo(m_dummyTransport);
    channel.d_func()->publisher->initializeClient(m_dummyTransport);

    QJsonObject objectInfo = channel.d_func()->publisher->wrapResult(QVariant::fromValue(&obj), m_dummyTransport).toObject();

    QCOMPARE(2, objectInfo.length());
    QVERIFY(objectInfo.contains("id"));
    QVERIFY(objectInfo.contains("__QObject*__"));
    QVERIFY(objectInfo.value("__QObject*__").isBool() && objectInfo.value("__QObject*__").toBool());

    QString returnedId = objectInfo.value("id").toString();

    QCOMPARE(&obj, channel.d_func()->publisher->registeredObjects.value(obj.objectName()));
    QCOMPARE(obj.objectName(), channel.d_func()->publisher->registeredObjectIds.value(&obj));
    QCOMPARE(obj.objectName(), returnedId);
}

void TestWebChannel::testUnwrapObject()
{
    QWebChannel channel;

    {
        TestObject obj;
        obj.setObjectName("testObject");
        channel.registerObject(obj.objectName(), &obj);
        QObject *unwrapped = channel.d_func()->publisher->unwrapObject(obj.objectName());
        QCOMPARE(unwrapped, &obj);
    }
    {
        TestObject obj;
        QJsonObject objectInfo = channel.d_func()->publisher->wrapResult(QVariant::fromValue(&obj), m_dummyTransport).toObject();
        QObject *unwrapped = channel.d_func()->publisher->unwrapObject(objectInfo["id"].toString());
        QCOMPARE(unwrapped, &obj);
    }
}

void TestWebChannel::testTransportWrapObjectProperties()
{
    QWebChannel channel;

    TestObject obj;
    obj.setObjectName("testObject");
    channel.registerObject(obj.objectName(), &obj);

    DummyTransport *dummyTransport = new DummyTransport(this);
    channel.connectTo(dummyTransport);
    channel.d_func()->publisher->initializeClient(dummyTransport);
    channel.d_func()->publisher->setClientIsIdle(true, dummyTransport);

    QCOMPARE(channel.d_func()->publisher->transportedWrappedObjects.size(), 0);

    QObject objPropObject;
    objPropObject.setObjectName("foobar");

    obj.setObjectProperty(&objPropObject);

    channel.d_func()->publisher->sendPendingPropertyUpdates();

    QCOMPARE(channel.d_func()->publisher->wrappedObjects.size(), 1);
    const QString wrappedObjId = channel.d_func()->publisher->wrappedObjects.keys()[0];

    QCOMPARE(channel.d_func()->publisher->transportedWrappedObjects.size(), 1);
    QCOMPARE(channel.d_func()->publisher->transportedWrappedObjects.keys()[0], dummyTransport);
    QCOMPARE(channel.d_func()->publisher->transportedWrappedObjects.values()[0], wrappedObjId);
}

void TestWebChannel::testRemoveUnusedTransports()
{
    QWebChannel channel;
    DummyTransport *dummyTransport = new DummyTransport(this);
    TestObject obj;

    channel.connectTo(dummyTransport);
    channel.d_func()->publisher->initializeClient(dummyTransport);

    QMetaObjectPublisher *pub = channel.d_func()->publisher;
    pub->wrapResult(QVariant::fromValue(&obj), dummyTransport);

    QCOMPARE(pub->wrappedObjects.size(), 1);
    QCOMPARE(pub->registeredObjectIds.size(), 1);

    channel.disconnectFrom(dummyTransport);
    delete dummyTransport;

    QCOMPARE(pub->wrappedObjects.size(), 0);
    QCOMPARE(pub->registeredObjectIds.size(), 0);
}

void TestWebChannel::testPassWrappedObjectBack()
{
    QWebChannel channel;
    TestObject registeredObj;
    TestObject returnedObjMethod;
    TestObject returnedObjProperty;

    registeredObj.setObjectName("registeredObject");

    channel.registerObject(registeredObj.objectName(), &registeredObj);
    channel.connectTo(m_dummyTransport);
    channel.d_func()->publisher->initializeClient(m_dummyTransport);

    QMetaObjectPublisher *pub = channel.d_func()->publisher;
    QJsonObject returnedObjMethodInfo = pub->wrapResult(QVariant::fromValue(&returnedObjMethod), m_dummyTransport).toObject();
    QJsonObject returnedObjPropertyInfo = pub->wrapResult(QVariant::fromValue(&returnedObjProperty), m_dummyTransport).toObject();

    QJsonArray argsMethod;
    QJsonObject argMethod0;
    argMethod0["id"] = returnedObjMethodInfo["id"];
    argsMethod << argMethod0;
    QJsonObject argProperty;
    argProperty["id"] = returnedObjPropertyInfo["id"];

    pub->invokeMethod(&registeredObj, "setReturnedObject", argsMethod);
    QCOMPARE(registeredObj.mReturnedObject, &returnedObjMethod);
    pub->setProperty(&registeredObj, registeredObj.metaObject()->indexOfProperty("returnedObject"), argProperty);
    QCOMPARE(registeredObj.mReturnedObject, &returnedObjProperty);
}

void TestWebChannel::testWrapValues_data()
{
    QTest::addColumn<QVariant>("variant");
    QTest::addColumn<QJsonValue>("json");

    QTest::addRow("enum") << QVariant::fromValue(TestObject::Asdf)
                          << QJsonValue(static_cast<int>(TestObject::Asdf));

    const TestObject::TestFlags flags = TestObject::FirstFlag | TestObject::SecondFlag;
    QTest::addRow("flags") << QVariant::fromValue(flags)
                           << QJsonValue(static_cast<int>(flags));

    QTest::addRow("list") << QVariant::fromValue(QList<int>{1, 2, 3})
                          << QJsonValue(QJsonArray{1, 2, 3});

    QTest::addRow("customVector") << QVariant::fromValue(TestStructVector{{1, 2}, {3, 4}})
                                  << QJsonValue(QJsonArray({QJsonObject{{"foo", 1}, {"bar", 2}},
                                                            QJsonObject{{"foo", 3}, {"bar", 4}}}));

    QTest::addRow("nullptr") << QVariant::fromValue(nullptr)
                             << QJsonValue();

    QTest::addRow("hash") << QVariant::fromValue(QVariantHash{{"One", 1},
                                                              {"Two", 2}})
                          << QJsonValue(QJsonObject{{"One", 1},
                                                    {"Two", 2}});

    QTest::addRow("url") << QVariant::fromValue(QUrl("aviancarrier:/test"))
                         << QJsonValue(QJsonValue(QString("aviancarrier:/test")));

    QTest::addRow("map") << QVariant::fromValue(QVariantMap{{"One", 1},
                                                            {"Two", 2}})
                         << QJsonValue(QJsonObject{{"One", 1},
                                                   {"Two", 2}});

    QTest::addRow("customType") << QVariant::fromValue(TestStruct{42, 7})
                                << QJsonValue(QJsonObject{{"__type__", "TestStruct"},
                                                          {"foo", 42},
                                                          {"bar", 7}});
}

void TestWebChannel::testWrapValues()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    QFETCH(QVariant, variant);
    QFETCH(QJsonValue, json);

    QJsonValue value = channel.d_func()->publisher->wrapResult(variant, m_dummyTransport);
    QCOMPARE(value, json);
}

void TestWebChannel::testWrapObjectWithMultipleTransports()
{
    QWebChannel channel;
    QMetaObjectPublisher *pub = channel.d_func()->publisher;

    DummyTransport *dummyTransport = new DummyTransport(this);
    DummyTransport *dummyTransport2 = new DummyTransport(this);

    TestObject obj;

    pub->wrapResult(QVariant::fromValue(&obj), dummyTransport);
    pub->wrapResult(QVariant::fromValue(&obj), dummyTransport2);

    QCOMPARE(pub->transportedWrappedObjects.size(), 2);
}

void TestWebChannel::testJsonToVariant_data()
{
    QTest::addColumn<QJsonValue>("json");
    QTest::addColumn<QVariant>("targetVariant");

    QTest::addRow("enum") << QJsonValue(static_cast<int>(TestObject::Asdf))
                          << QVariant::fromValue(TestObject::Asdf);

    const TestObject::TestFlags flags =  TestObject::FirstFlag | TestObject::SecondFlag;
    QTest::addRow("flags") << QJsonValue(static_cast<int>(flags))
                           << QVariant::fromValue(flags);

    QTest::addRow("customType") << QJsonValue(QJsonObject{{"__type__", "TestStruct"},
                                                          {"foo", 42},
                                                          {"bar", 7}})
                                << QVariant::fromValue(TestStruct{42, 7});
}

void TestWebChannel::testJsonToVariant()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    QFETCH(QJsonValue, json);
    QFETCH(QVariant, targetVariant);

    QVariant convertedValue = channel.d_func()->publisher->toVariant(json, targetVariant.userType());
    QCOMPARE(convertedValue, targetVariant);
}

void TestWebChannel::testInfiniteRecursion()
{
    QWebChannel channel;
    TestObject obj;
    obj.setObjectProperty(&obj);
    obj.setObjectName("myTestObject");

    channel.connectTo(m_dummyTransport);
    channel.d_func()->publisher->initializeClient(m_dummyTransport);

    QJsonObject objectInfo = channel.d_func()->publisher->wrapResult(QVariant::fromValue(&obj), m_dummyTransport).toObject();

    // Wrap the result twice to test for QTBUG-84007. A single result wrap will not trigger all recursion paths.
    objectInfo = channel.d_func()->publisher->wrapResult(QVariant::fromValue(&obj), m_dummyTransport).toObject();
}

void TestWebChannel::testAsyncObject()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    QThread thread;
    thread.start();

    TestObject obj;
    obj.moveToThread(&thread);

    QJsonArray args;
    args.append(QJsonValue("message"));

    {
        int received = 0;
        connect(&obj, &TestObject::propChanged, this, [&](const QString &arg) {
            QCOMPARE(arg, args.at(0).toString());
            ++received;
        });
        channel.d_func()->publisher->invokeMethod(&obj, "setProp", args);
        QTRY_COMPARE(received, 1);
    }

    {
        int received = 0;
        auto handler = obj.bindableStringProperty().onValueChanged([&] {
            ++received;
        });
        channel.d_func()->publisher->invokeMethod(&obj, "setStringProperty", args);
        QTRY_COMPARE(received, 1);
    }

    channel.registerObject("myObj", &obj);
    channel.d_func()->publisher->initializeClient(m_dummyTransport);

    QJsonObject connectMessage;
    connectMessage["type"] = 7;
    connectMessage["object"] = "myObj";
    connectMessage["signal"] = obj.metaObject()->indexOfSignal("replay()");
    channel.d_func()->publisher->handleMessage(connectMessage, m_dummyTransport);

    {
        int received = 0;
        connect(&obj, &TestObject::replay, this, [&]() { ++received; });
        QMetaObject::invokeMethod(&obj, "fire");
        QTRY_COMPARE(received, 1);
        channel.deregisterObject(&obj);
        QMetaObject::invokeMethod(&obj, "fire");
        QTRY_COMPARE(received, 2);
    }

    thread.quit();
    thread.wait();
}

void TestWebChannel::testQProperty()
{
    static const int IndexOfStringProperty =
            TestObject::staticMetaObject.indexOfProperty("stringProperty");

    DummyTransport transport;
    QWebChannel channel;
    QMetaObjectPublisher *publisher = channel.d_func()->publisher;

    {
        TestObject testObj;
        testObj.setObjectName("testObject");

        QProperty<QString> obj1("Hello");
        testObj.bindableStringProperty().setBinding([&](){ return obj1.value(); });

        QCOMPARE(obj1.value(), testObj.readStringProperty());

        channel.registerObject(testObj.objectName(), &testObj);
        channel.connectTo(&transport);

        publisher->initializeClient(&transport);
        // One bindable property should result in one observer
        QCOMPARE(publisher->propertyObservers.count(&testObj), 1u);

        QVariant result;
        publisher->setClientIsIdle(true, &transport);
        result = publisher->invokeMethod(&testObj, "getStringProperty", {});
        QCOMPARE(result.toString(), obj1.value());

        obj1 = "world";
        result = publisher->invokeMethod(&testObj, "getStringProperty", {});
        QCOMPARE(result.toString(), obj1.value());

        publisher->sendPendingPropertyUpdates();
        QVERIFY(!transport.messagesSent().isEmpty());
        const QJsonObject updateMessage = transport.messagesSent().last()["data"][0].toObject();
        QCOMPARE(updateMessage["object"], testObj.objectName());
        QCOMPARE(updateMessage["properties"][QString::number(IndexOfStringProperty)], obj1.value());

        publisher->invokeMethod(&testObj, "setStringProperty2", {"Hey"});
        publisher->invokeMethod(&testObj, "bindStringPropertyToStringProperty2", {});
        obj1 = "This should not affect getStringProperty";
        result = publisher->invokeMethod(&testObj, "getStringProperty", {});
        QCOMPARE(result.toString(), "Hey");

        publisher->invokeMethod(&testObj, "setStringProperty2", {"again"});
        result = publisher->invokeMethod(&testObj, "getStringProperty", {});
        QCOMPARE(result.toString(), "again");
    }

    // Ensure that the observer has been removed after the object has been
    // destroyed
    QCOMPARE(publisher->propertyObservers.size(), 0u);
}

void TestWebChannel::testPropertyUpdateInterval_data()
{
    QTest::addColumn<int>("firstUpdateInterval");
    QTest::addColumn<int>("firstUpdateWithin");
    QTest::addColumn<int>("secondUpdateInterval");
    QTest::addColumn<int>("secondUpdateWithin");

    QTest::newRow("1500ms") << 1500 << 2000 << 1500 << 2000;
    QTest::newRow("Next event") << 0 << 0 << 0 << 0;
    QTest::newRow("Immediately") << -1 << 0 << -1 << 0;
    QTest::newRow("1500ms then next event") << 1500 << 2000 << 0 << 0;
    QTest::newRow("Next event then 1500ms") << 0 << 0 << 1500 << 2000;
    QTest::newRow("Immediately the next event") << -1 << 0 << 0 << 0;
    QTest::newRow("Next event then immediately") << 0 << 0 << -1 << 0;
    QTest::newRow("Immediately then 1500ms") << -1 << 0 << 1500 << 2000;
    QTest::newRow("1500ms then immediately") << 1500 << 2000 << -1 << 0;
}

void TestWebChannel::testPropertyUpdateInterval()
{
    DummyTransport transport;
    QWebChannel channel;
    QMetaObjectPublisher *publisher = channel.d_func()->publisher;

    QFETCH(int, firstUpdateInterval);
    QFETCH(int, firstUpdateWithin);

    QProperty<int> propertyUpdateInterval(firstUpdateInterval);
    channel.bindablePropertyUpdateInterval().setBinding(
            Qt::makePropertyBinding(propertyUpdateInterval));
    QCOMPARE(channel.propertyUpdateInterval(), firstUpdateInterval);

    TestObject testObj;
    testObj.setObjectName("testObject");
    channel.registerObject(testObj.objectName(), &testObj);
    channel.connectTo(&transport);
    QProperty<QString> obj1("Hello");
    testObj.bindableStringProperty().setBinding([&]() { return obj1.value(); });
    publisher->initializeClient(&transport);
    publisher->setClientIsIdle(true, &transport);
    QVERIFY(transport.messagesSent().isEmpty());

    // First update
    obj1 = "world";
    if (firstUpdateInterval > 0) {
        QCoreApplication::processEvents();
        QVERIFY(transport.messagesSent().isEmpty());
        QTRY_COMPARE_WITH_TIMEOUT(transport.messagesSent().size(), 1u, firstUpdateWithin);
    } else if (firstUpdateInterval == 0) {
        QCOMPARE(transport.messagesSent().size(), 0u);
        QCoreApplication::processEvents();
        QCOMPARE(transport.messagesSent().size(), 1u);
    } else {
        QCOMPARE(transport.messagesSent().size(), 1u);
    }

    QFETCH(int, secondUpdateInterval);
    QFETCH(int, secondUpdateWithin);
    propertyUpdateInterval = secondUpdateInterval;
    publisher->setClientIsIdle(true, &transport);
    obj1 = "!!!";

    if (secondUpdateInterval > 0) {
        QCoreApplication::processEvents();
        QCOMPARE(transport.messagesSent().size(), 1u);
        QCOMPARE(channel.propertyUpdateInterval(), secondUpdateInterval);
        QVERIFY(publisher->timer.isActive());
        QTRY_COMPARE_WITH_TIMEOUT(transport.messagesSent().size(), 2u, secondUpdateWithin);
    } else if (secondUpdateInterval == 0) {
        QCOMPARE(transport.messagesSent().size(), 1u);
        QCoreApplication::processEvents();
        QCOMPARE(transport.messagesSent().size(), 2u);
    } else {
        QCOMPARE(transport.messagesSent().size(), 2u);
    }
}

void TestWebChannel::testQPropertyBlockUpdates()
{
    DummyTransport transport;
    QWebChannel channel;
    QMetaObjectPublisher *publisher = channel.d_func()->publisher;
    publisher->setPropertyUpdateInterval(0);
    QProperty<bool> blockUpdates(false);
    channel.bindableBlockUpdates().setBinding(Qt::makePropertyBinding(blockUpdates));
    QCOMPARE(channel.blockUpdates(), false);
    bool blockedSignalled(false);
    connect(&channel, &QWebChannel::blockUpdatesChanged, this,
            [&](bool block) { blockedSignalled = block; });

    TestObject testObj;
    testObj.setObjectName("testObject");
    channel.registerObject(testObj.objectName(), &testObj);
    channel.connectTo(&transport);
    QProperty<QString> obj1("Hello");
    testObj.bindableStringProperty().setBinding(Qt::makePropertyBinding(obj1));
    publisher->initializeClient(&transport);
    QVERIFY(transport.messagesSent().isEmpty());

    blockUpdates = true;
    QCOMPARE(blockedSignalled, true);
    publisher->setClientIsIdle(true, &transport);
    obj1 = "world";

    QTest::qWait(0);
    QCOMPARE(transport.messagesSent().size(), 0u);

    blockUpdates = false;
    QCOMPARE(blockedSignalled, false);
    QCOMPARE(transport.messagesSent().size(), 1u);
}

void TestWebChannel::testBindings()
{
    QWebChannel channel;

    QTestPrivate::testReadWritePropertyBasics(channel, true, false, "blockUpdates");
    if (QTest::currentTestFailed()) {
        qDebug("Failed property test for QWebChannel::blockUpdates");
        return;
    }

    QVERIFY(!channel.blockUpdates());

    QTestPrivate::testReadWritePropertyBasics(channel, 100, 200, "propertyUpdateInterval");
    if (QTest::currentTestFailed()) {
        qDebug("Failed property test for QWebChannel::propertyUpdateInterval");
        return;
    }
}

void TestWebChannel::testPropertyMultipleTransports()
{
    DummyTransport transport1;
    DummyTransport transport2;

    QWebChannel channel;
    QMetaObjectPublisher *publisher = channel.d_func()->publisher;

    TestObject testObj;
    testObj.setObjectName("testObject");
    channel.registerObject(testObj.objectName(), &testObj);
    channel.connectTo(&transport1);
    channel.connectTo(&transport2);
    QProperty<QString> obj1("Hello");
    testObj.bindableStringProperty().setBinding([&]() { return obj1.value(); });

    publisher->initializeClient(&transport1);
    publisher->initializeClient(&transport2);
    publisher->setClientIsIdle(true, &transport1);
    QCOMPARE(publisher->isClientIdle(&transport1), true);
    QCOMPARE(publisher->isClientIdle(&transport2), false);
    channel.setPropertyUpdateInterval(1000);
    QVERIFY(transport1.messagesSent().isEmpty());
    QVERIFY(transport2.messagesSent().isEmpty());

    obj1 = "World";
    QTRY_COMPARE_WITH_TIMEOUT(transport1.messagesSent().size(), 1u, 2000);
    QCOMPARE(transport2.messagesSent().size(), 0u);
    publisher->setClientIsIdle(true, &transport2);
    QTRY_COMPARE_WITH_TIMEOUT(transport2.messagesSent().size(), 1u, 2000);
    QCOMPARE(publisher->isClientIdle(&transport1), false);
    QCOMPARE(publisher->isClientIdle(&transport2), false);

    obj1 = "!!!";
    publisher->setClientIsIdle(true, &transport2);
    QCOMPARE(publisher->isClientIdle(&transport2), true);
    QCOMPARE(publisher->isClientIdle(&transport1), false);
    QTRY_COMPARE_WITH_TIMEOUT(transport2.messagesSent().size(), 2u, 2000);
    QCOMPARE(transport1.messagesSent().size(), 1u);
    publisher->setClientIsIdle(true, &transport1);
    QTRY_COMPARE_WITH_TIMEOUT(transport1.messagesSent().size(), 2u, 2000);
    QCOMPARE(publisher->isClientIdle(&transport1), false);
    QCOMPARE(publisher->isClientIdle(&transport2), false);
}

class FunctionWrapper : public QObject
{
    Q_OBJECT
    std::function<void()> m_fun;
public:
    FunctionWrapper(std::function<void()> fun) : m_fun(std::move(fun)) {}
public slots:
    void invoke()
    {
        m_fun();
    }
};

void TestWebChannel::testDeletionDuringMethodInvocation_data()
{
    QTest::addColumn<bool>("deleteChannel");
    QTest::addColumn<bool>("deleteTransport");
    QTest::newRow("delete neither")   << false << false;
    QTest::newRow("delete channel")   << true  << false;
    QTest::newRow("delete transport") << false << true;
    QTest::newRow("delete both")      << true  << true;
}

void TestWebChannel::testDeletionDuringMethodInvocation()
{
    QFETCH(bool, deleteChannel);
    QFETCH(bool, deleteTransport);

    QScopedPointer<QWebChannel> channel(new QWebChannel);
    QScopedPointer<DummyTransport> transport(new DummyTransport(nullptr));
    FunctionWrapper deleter([&](){
        if (deleteChannel)
            channel.reset();
        if (deleteTransport)
            transport.reset();
    });
    channel->registerObject("deleter", &deleter);
    channel->connectTo(transport.data());

    transport->emitMessageReceived({
        {"type", TypeInvokeMethod},
        {"object", "deleter"},
        {"method", deleter.metaObject()->indexOfMethod("invoke()")},
        {"id", 42}
    });

    QCOMPARE(deleteChannel, !channel);
    QCOMPARE(deleteTransport, !transport);
    if (!deleteTransport)
        QCOMPARE(transport->messagesSent().size(), deleteChannel ? 0 : 1);
}

#if QT_CONFIG(future)
void TestWebChannel::testAsyncMethodReturningFuture_data()
{
    QTest::addColumn<QString>("methodName");
    QTest::addColumn<QJsonValue>("result");

    QTest::addRow("int") << "futureIntResult" << QJsonValue{42};
    QTest::addRow("int-delayed") << "futureDelayedIntResult" << QJsonValue{7};
#ifdef WEBCHANNEL_TESTS_CAN_USE_CONCURRENT
    QTest::addRow("int-thread") << "futureIntResultFromThread" << QJsonValue{1337};
#endif
    QTest::addRow("void") << "futureVoidResult" << QJsonValue{};
    QTest::addRow("QString") << "futureStringResult" << QJsonValue{"foo"};

    QTest::addRow("cancelled") << "cancelledFuture" << QJsonValue{};
    QTest::addRow("failed")    << "failedFuture"    << QJsonValue{};
}

void TestWebChannel::testAsyncMethodReturningFuture()
{
    QFETCH(QString, methodName);
    QFETCH(QJsonValue, result);

    QWebChannel channel;
    TestObject obj;
    channel.registerObject("testObject", &obj);

    DummyTransport transport;
    channel.connectTo(&transport);

    transport.emitMessageReceived({
        {"type", TypeInvokeMethod},
        {"object", "testObject"},
        {"method", methodName},
        {"id", 1}
    });

    QTRY_COMPARE(transport.messagesSent().size(), 1);
    QCOMPARE(transport.messagesSent().first().value("data"), result);
}
#endif

#ifdef WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE

class SubclassedTestObject : public TestObject
{
    Q_OBJECT
    Q_PROPERTY(QString bar READ bar WRITE setBar NOTIFY theBarHasChanged)
public:
    void setBar(const QString &newBar);
signals:
    void theBarHasChanged();
};

void SubclassedTestObject::setBar(const QString &newBar)
{
    if (!newBar.isNull())
        emit theBarHasChanged();
}

class TestSubclassedFunctor {
public:
    TestSubclassedFunctor(TestJSEngine *engine)
        : m_engine(engine)
    {
    }

    void operator()() {
        QCOMPARE(m_engine->logger()->errorCount(), 0);
    }

private:
    TestJSEngine *m_engine;
};
#endif // WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE

void TestWebChannel::qtbug46548_overriddenProperties()
{
#ifndef WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE
    QSKIP("A JS engine is required for this test to make sense.");
#else
    SubclassedTestObject obj;
    obj.setObjectName("subclassedTestObject");

    QWebChannel webChannel;
    webChannel.registerObject(obj.objectName(), &obj);
    TestJSEngine engine;
    webChannel.connectTo(engine.transport());
    QSignalSpy spy(&engine, &TestJSEngine::channelSetupReady);
    connect(&engine, &TestJSEngine::channelSetupReady, TestSubclassedFunctor(&engine));
    engine.initWebChannelJS();
    if (!spy.size())
        spy.wait();
    QCOMPARE(spy.size(), 1);
    QJSValue subclassedTestObject = engine.evaluate("channel.objects[\"subclassedTestObject\"]");
    QVERIFY(subclassedTestObject.isObject());

#endif // WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE
}

void TestWebChannel::qtbug62388_wrapObjectMultipleTransports()
{
    QWebChannel channel;
    TestObject obj;

    auto initTransport = [&channel](QWebChannelAbstractTransport *transport) {
        channel.connectTo(transport);
        channel.d_func()->publisher->initializeClient(transport);
    };
    initTransport(m_dummyTransport);

    auto queryObjectInfo = [&channel](QObject *obj, QWebChannelAbstractTransport *transport) {
        return channel.d_func()->publisher->wrapResult(QVariant::fromValue(obj), transport).toObject();
    };

    auto verifyObjectInfo = [&obj](const QJsonObject &objectInfo) {

        QCOMPARE(objectInfo.length(), 3);
        QVERIFY(objectInfo.contains("id"));
        QVERIFY(objectInfo.contains("__QObject*__"));
        QVERIFY(objectInfo.contains("data"));
        QVERIFY(objectInfo.value("__QObject*__").isBool() && objectInfo.value("__QObject*__").toBool());

        const auto propIndex = obj.metaObject()->indexOfProperty("prop");
        const auto prop = objectInfo["data"].toObject()["properties"].toArray()[propIndex].toArray()[3].toString();
        QCOMPARE(prop, obj.prop());
    };

    const auto objectInfo = queryObjectInfo(&obj, m_dummyTransport);
    verifyObjectInfo(objectInfo);

    const auto id = objectInfo.value("id").toString();

    QCOMPARE(channel.d_func()->publisher->unwrapObject(id), &obj);

    DummyTransport transport;
    initTransport(&transport);
    QCOMPARE(queryObjectInfo(&obj, &transport), objectInfo);

    obj.setProp("asdf");

    const auto objectInfo2 = queryObjectInfo(&obj, m_dummyTransport);
    QVERIFY(objectInfo2 != objectInfo);
    verifyObjectInfo(objectInfo2);

    DummyTransport transport2;
    initTransport(&transport2);
    QCOMPARE(queryObjectInfo(&obj, &transport2), objectInfo2);

    // don't crash when the transports are destroyed
}

QTEST_MAIN(TestWebChannel)

#include "tst_webchannel.moc"
