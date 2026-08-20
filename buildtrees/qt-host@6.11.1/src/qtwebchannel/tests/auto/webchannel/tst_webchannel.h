// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// Copyright (C) 2019 Menlo Systems GmbH, author Arno Rehn <a.rehn@menlosystems.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TST_WEBCHANNEL_H
#define TST_WEBCHANNEL_H

#include <QObject>
#include <QProperty>
#include <QVariant>
#include <QList>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#if QT_CONFIG(future)
#include <QFuture>
#endif
#include <QtDebug>

#include <QtWebChannel/QWebChannelAbstractTransport>

struct TestStruct
{
    TestStruct(int foo = 0, int bar = 0)
        : foo(foo)
        , bar(bar)
    {}
    int foo;
    int bar;

    operator QString() const {
        return QStringLiteral("TestStruct(foo=%1, bar=%2)").arg(foo).arg(bar);
    }
};
inline bool operator==(const TestStruct &a, const TestStruct &b)
{
    return a.foo == b.foo && a.bar == b.bar;
}
inline QDebug operator<<(QDebug &dbg, const TestStruct &ts)
{
    QDebugStateSaver dbgState(dbg);
    dbg.noquote() << static_cast<QString>(ts);
    return dbg;
}
Q_DECLARE_METATYPE(TestStruct)
using TestStructVector = std::vector<TestStruct>;
Q_DECLARE_METATYPE(TestStructVector)

QT_BEGIN_NAMESPACE

class DummyTransport : public QWebChannelAbstractTransport
{
    Q_OBJECT
public:
    explicit DummyTransport(QObject *parent = nullptr)
        : QWebChannelAbstractTransport(parent)
    {}
    ~DummyTransport() {};

    void emitMessageReceived(const QJsonObject &message)
    {
        emit messageReceived(message, this);
    }

    QList<QJsonObject> messagesSent() const { return mMessagesSent; }

public slots:
    void sendMessage(const QJsonObject &message) override
    {
        mMessagesSent.push_back(message);
    }
private:
    QList<QJsonObject> mMessagesSent;
};

class TestObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Foo foo READ foo CONSTANT)
    Q_PROPERTY(int asdf READ asdf NOTIFY asdfChanged)
    Q_PROPERTY(QString bar READ bar NOTIFY theBarHasChanged)
    Q_PROPERTY(QObject * objectProperty READ objectProperty WRITE setObjectProperty NOTIFY objectPropertyChanged)
    Q_PROPERTY(TestObject * returnedObject READ returnedObject WRITE setReturnedObject NOTIFY returnedObjectChanged)
    Q_PROPERTY(QString prop READ prop WRITE setProp NOTIFY propChanged)
    Q_PROPERTY(QString stringProperty READ readStringProperty WRITE setStringProperty BINDABLE bindableStringProperty)

public:
    explicit TestObject(QObject *parent = nullptr)
        : QObject(parent)
        , mObjectProperty(nullptr)
        , mReturnedObject(nullptr)
    { }

    enum Foo {
        Bar,
        Asdf
    };
    Q_ENUM(Foo)

    enum TestFlag : quint16 {
        FirstFlag = 0x1,
        SecondFlag = 0x2
    };
    Q_DECLARE_FLAGS(TestFlags, TestFlag)
    Q_FLAG(TestFlags)

    Foo foo() const {return Bar;}
    int asdf() const {return 42;}
    QString bar() const {return QString();}

    QObject *objectProperty() const
    {
        return mObjectProperty;
    }

    TestObject *returnedObject() const
    {
        return mReturnedObject;
    }

    QString prop() const
    {
        return mProp;
    }

    QString readStringProperty() const { return mStringProperty; }

    Q_INVOKABLE void method1() {}

#if QT_CONFIG(future)
    Q_INVOKABLE QFuture<int> futureIntResult() const;
    Q_INVOKABLE QFuture<int> futureDelayedIntResult() const;
#ifdef WEBCHANNEL_TESTS_CAN_USE_CONCURRENT
    Q_INVOKABLE QFuture<int> futureIntResultFromThread() const;
#endif
    Q_INVOKABLE QFuture<void> futureVoidResult() const;
    Q_INVOKABLE QFuture<QString> futureStringResult() const;
    Q_INVOKABLE QFuture<int> cancelledFuture() const;
    Q_INVOKABLE QFuture<int> failedFuture() const;
#endif

protected:
    Q_INVOKABLE void method2() {}

private:
    Q_INVOKABLE void method3() {}

signals:
    void sig1();
    void sig2(const QString&);
    void asdfChanged();
    void theBarHasChanged();
    void objectPropertyChanged();
    void returnedObjectChanged();
    void propChanged(const QString&);
    void replay();
    void overloadSignal(int);
    void overloadSignal(float);

public slots:
    void slot1() {}
    void slot2(const QString&) {}

    void setReturnedObject(TestObject *obj)
    {
        mReturnedObject = obj;
        emit returnedObjectChanged();
    }

    void setObjectProperty(QObject *object)
    {
        mObjectProperty = object;
        emit objectPropertyChanged();
    }

    void setProp(const QString&prop) {emit propChanged(mProp=prop);}
    void fire() {emit replay();}

    double overload(double d) { return d + 1; }
    int overload(int i) { return i * 2; }
    QObject *overload(QObject *object) { return object; }
    QString overload(const QString &str) { return str.toUpper(); }
    QString overload(const QString &str, int i) { return str.toUpper() + QString::number(i + 1); }
    QString overload(const QJsonArray &v) { return QString::number(v[1].toInt()) + v[0].toString(); }

    void setStringProperty(const QString &v) { mStringProperty = v; }
    QBindable<QString> bindableStringProperty() { return &mStringProperty; }
    QString getStringProperty() const { return mStringProperty; }
    void bindStringPropertyToStringProperty2() { bindableStringProperty().setBinding(Qt::makePropertyBinding(mStringProperty2)); }
    void setStringProperty2(const QString &string) { mStringProperty2 = string; }

protected slots:
    void slot3() {}

private slots:
    void slot4() {}

public:
    QObject *mObjectProperty;
    TestObject *mReturnedObject;
    QString mProp;
    Q_OBJECT_BINDABLE_PROPERTY(TestObject, QString, mStringProperty);
    QProperty<QString> mStringProperty2;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TestObject::TestFlags)

class TestWebChannel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int lastInt READ readInt WRITE setInt NOTIFY lastIntChanged);
    Q_PROPERTY(bool lastBool READ readBool WRITE setBool NOTIFY lastBoolChanged);
    Q_PROPERTY(double lastDouble READ readDouble WRITE setDouble NOTIFY lastDoubleChanged);
    Q_PROPERTY(QVariant lastVariant READ readVariant WRITE setVariant NOTIFY lastVariantChanged);
    Q_PROPERTY(QJsonValue lastJsonValue READ readJsonValue WRITE setJsonValue NOTIFY lastJsonValueChanged);
    Q_PROPERTY(QJsonObject lastJsonObject READ readJsonObject WRITE setJsonObject NOTIFY lastJsonObjectChanged);
    Q_PROPERTY(QJsonArray lastJsonArray READ readJsonArray WRITE setJsonArray NOTIFY lastJsonArrayChanged);
public:
    explicit TestWebChannel(QObject *parent = 0);
    virtual ~TestWebChannel();

public slots:
    int readInt() const;
    void setInt(int i);
    bool readBool() const;
    void setBool(bool b);
    double readDouble() const;
    void setDouble(double d);
    QVariant readVariant() const;
    void setVariant(const QVariant &v);
    QJsonValue readJsonValue() const;
    void setJsonValue(const QJsonValue &v);
    QJsonObject readJsonObject() const;
    void setJsonObject(const QJsonObject &v);
    QJsonArray readJsonArray() const;
    void setJsonArray(const QJsonArray &v);

    QUrl readUrl() const;
    void setUrl(const QUrl &u);

    int readOverload(int i);
    QString readOverload(const QString &arg);
    QString readOverload(const QString &arg, int i);

signals:
    void lastIntChanged();
    void lastBoolChanged();
    void lastDoubleChanged();
    void lastVariantChanged();
    void lastJsonValueChanged();
    void lastJsonObjectChanged();
    void lastJsonArrayChanged();

private slots:
    void testRegisterObjects();
    void testDeregisterObjects();
    void testDeregisterObjectAtStart();
    void testInfoForObject();
    void testInvokeMethodConversion();
    void testFunctionOverloading();
    void testSetPropertyConversion();
    void testInvokeMethodOverloadResolution();
    void testDisconnect();
    void testWrapRegisteredObject();
    void testUnwrapObject();
    void testTransportWrapObjectProperties();
    void testRemoveUnusedTransports();
    void testPassWrappedObjectBack();
    void testWrapValues_data();
    void testWrapValues();
    void testWrapObjectWithMultipleTransports();
    void testJsonToVariant_data();
    void testJsonToVariant();
    void testInfiniteRecursion();
    void testAsyncObject();
    void testQProperty();
    void testPropertyUpdateInterval_data();
    void testPropertyUpdateInterval();
    void testPropertyMultipleTransports();
    void testQPropertyBlockUpdates();
    void testBindings();
    void testDeletionDuringMethodInvocation_data();
    void testDeletionDuringMethodInvocation();

#if QT_CONFIG(future)
    void testAsyncMethodReturningFuture_data();
    void testAsyncMethodReturningFuture();
#endif

    void qtbug46548_overriddenProperties();
    void qtbug62388_wrapObjectMultipleTransports();

private:
    DummyTransport *m_dummyTransport;

    int m_lastInt;
    bool m_lastBool;
    double m_lastDouble;
    QVariant m_lastVariant;
    QJsonValue m_lastJsonValue;
    QJsonObject m_lastJsonObject;
    QJsonArray m_lastJsonArray;
    QUrl m_lastUrl;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(TestObject::Foo)

#endif // TST_WEBCHANNEL_H
