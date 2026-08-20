// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest

import QtWebChannel
import QtWebChannel.Tests
import "qrc:///qtwebchannel/qwebchannel.js" as JSClient

TestCase {
    name: "WebChannel"

    Client {
        id: client
    }

    property var lastMethodArg

    QtObject {
        id: myObj
        property int myProperty: 1

        signal mySignal(var arg, QtObject object)

        function myMethod(arg)
        {
            lastMethodArg = arg;
        }

        WebChannel.id: "myObj"
    }
    QtObject {
        id: myOtherObj
        property var foo: 1
        property var bar: 1
        WebChannel.id: "myOtherObj"
    }
    QtObject {
        id: myValueObj
        property var value: undefined
        WebChannel.id: "myValueObj"
    }
    property var lastFactoryObj
    QtObject{ id: bar; objectName: "bar" }
    QtObject{ id: baz; objectName: "baz" }
    QtObject {
        id: myFactory
        function create(id)
        {
            lastFactoryObj = component.createObject(myFactory, {objectName: id});
            return lastFactoryObj;
        }
        function switchObject() {
            otherObject = myOtherObj;
        }
        property var objectInProperty: QtObject {
            objectName: "foo"
        }
        property var otherObject: myObj
        property var objects: [ bar, baz ];
        WebChannel.id: "myFactory"
    }
    Component {
        id: component
        QtObject {
            property var myProperty : 0
            function myMethod(arg) {
                lastMethodArg = arg;
                return myProperty;
            }
            signal mySignal(var arg1, var arg2)
        }
    }

    TestObject {
        id: testObject
        WebChannel.id: "testObject"
    }

    TestWebChannel {
        id: webChannel
        transports: [client.serverTransport]
        registeredObjects: [myObj, myOtherObj, myValueObj, myFactory, testObject]
    }
    WebChannel {
        id: nullTest
    }

    function initChannel() {
        client.serverTransport.receiveMessage(JSON.stringify({type: JSClient.QWebChannelMessageTypes.idle}));
        webChannel.blockUpdates = true;
        webChannel.blockUpdates = false;
    }

    function init()
    {
        myObj.myProperty = 1
        // immediately send pending updates
        // to avoid property changed signals
        // during run of test case
        initChannel();
        client.cleanup();
    }
    function cleanup() {
        // make tests a bit more strict and predictable
        // by assuming that a test consumes all messages
        initChannel();
        compare(client.clientMessages.length, 0);
    }

    function test_notifyProperty()
    {
        compare(myObj.myProperty, 1);

        var initialValue;
        var changedValue;

        var channel = client.createChannel(function(channel) {
            initialValue = channel.objects.myObj.myProperty;
            channel.objects.myObj.myPropertyChanged.connect(function() {
                changedValue = channel.objects.myObj.myProperty;
            });
            // now trigger a write from the client side
            channel.objects.myObj.myProperty = 3;
        });

        client.awaitInit();
        var msg = client.awaitMessage();

        compare(initialValue, 1);
        compare(myObj.myProperty, 3);

        client.awaitIdle(); // init

        // change property, should be propagated to HTML client and a message be send there
        myObj.myProperty = 2;
        compare(myObj.myProperty, 2);
        client.awaitIdle(); // property update
        compare(changedValue, 2);
        compare(channel.objects.myObj.myProperty, 2)
    }

    function test_bindableProperty()
    {
        compare(testObject.stringProperty, "foo");

        var initialValue;

        var channel = client.createChannel(function(channel) {
            initialValue = channel.objects.testObject.stringProperty;
            // now trigger a write from the client side
            channel.objects.testObject.stringProperty = "bar";
        });

        client.awaitInit();
        var msg = client.awaitMessage();

        compare(initialValue, "foo");
        compare(testObject.stringProperty, "bar");

        client.awaitIdle(); // init

        // Change property, should be propagated to HTML client.
        // This is a bindable property only. Not change signal will be emitted
        // because there is none.
        testObject.stringProperty = "baz";
        compare(testObject.stringProperty, "baz");
        client.awaitIdle(); // property update
        compare(channel.objects.testObject.stringProperty, "baz");
    }

    function test_method()
    {
        var channel = client.createChannel(function (channel) {
            channel.objects.myObj.myMethod("test");
        });

        client.awaitInit();

        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        compare(msg.object, "myObj");
        compare(msg.args, ["test"]);

        compare(lastMethodArg, "test")

        client.awaitIdle();
    }

    function test_signal()
    {
        var signalReceivedArg;
        var signalReceivedObject;
        var channel = client.createChannel(function(channel) {
            channel.objects.myObj.mySignal.connect(function(arg, object) {
                signalReceivedArg = arg;
                signalReceivedObject = object;
            });
        });
        client.awaitInit();

        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.connectToSignal);
        compare(msg.object, "myObj");

        client.awaitIdle(); // initialization

        myObj.mySignal("test", myObj);

        compare(signalReceivedArg, "test");
        compare(signalReceivedObject.__id__, "myObj");

        var newObj = myFactory.create("newObj");
        myObj.mySignal(newObj, newObj);

        compare(signalReceivedArg.objectName, newObj.objectName);
        compare(signalReceivedObject.objectName, newObj.objectName);
    }

    function test_grouping()
    {
        var receivedPropertyUpdates = 0;
        var properties = 0;
        var channel = client.createChannel(function(channel) {
            var originalHandler = channel.handlePropertyUpdate;
            channel.handlePropertyUpdate = function(message) {
                originalHandler(message);
                receivedPropertyUpdates++;
                properties = [channel.objects.myObj.myProperty, channel.objects.myOtherObj.foo, channel.objects.myOtherObj.bar];
            };
        });
        client.awaitInit();
        client.awaitIdle();

        // change properties a lot, we expect this to be grouped into a single update notification
        for (var i = 0; i < 10; ++i) {
            myObj.myProperty = i;
            myOtherObj.foo = i;
            myOtherObj.bar = i;
        }

        client.awaitIdle();
        compare(receivedPropertyUpdates, 1);
        compare(properties, [myObj.myProperty, myOtherObj.foo, myOtherObj.bar]);
        verify(!client.awaitMessage());
    }

    function test_wrapper()
    {
        var signalArgs;
        var testObjBeforeDeletion;
        var testObjAfterDeletion;
        var testObjId;
        var testReturn;
        var channel = client.createChannel(function(channel) {
            channel.objects.myFactory.create("testObj", function(obj) {
                testObjId = obj.__id__;
                compare(channel.objects[testObjId], obj);
                obj.mySignal.connect(function() {
                    signalArgs = arguments;
                    testObjBeforeDeletion = obj;
                    obj.deleteLater();
                    testObjAfterDeletion = obj;
                });
                obj.myProperty = 42;
                obj.myMethod("foobar").then(function(result) {
                    testReturn = result;
                });
            });
        });
        client.awaitInit();
        client.awaitResponse();

        // create testObj
        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        compare(msg.object, "myFactory");
        client.awaitResponse();
        verify(lastFactoryObj);
        compare(lastFactoryObj.objectName, "testObj");
        compare(channel.objects[testObjId].objectName, "testObj");

        // mySignal connection
        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.connectToSignal);
        compare(msg.object, testObjId);

        // set myProperty
        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.setProperty);
        compare(msg.object, testObjId);
        compare(lastFactoryObj.myProperty, 42);

        // call myMethod
        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        compare(msg.object, testObjId);
        compare(msg.args, ["foobar"]);
        client.awaitResponse();
        compare(lastMethodArg, "foobar");

        client.awaitIdle();

        // the server should eventually notify the client about the property update
        client.awaitPropertyUpdate();

        // check that the Promise from myMethod was resolved
        // must happen after waiting for something so the Promise callback
        // can execute
        compare(testReturn, 42);

        client.awaitIdle();

        // property should be wrapped
        compare(channel.objects.myFactory.objectInProperty.objectName, "foo");
        // list property as well
        compare(channel.objects.myFactory.objects.length, 2);
        compare(channel.objects.myFactory.objects[0].objectName, "bar");
        compare(channel.objects.myFactory.objects[1].objectName, "baz");
        // map property as well
        compare(channel.objects.testObject.objectMap.subObject.objectName,
            "embedded");
        // also works with properties that reference other registered objects
        compare(channel.objects.myFactory.otherObject, channel.objects.myObj);

        // change object property
        channel.objects.myFactory.switchObject();
        client.awaitMessage();
        client.awaitResponse();
        client.awaitIdle();
        client.awaitPropertyUpdate();
        compare(channel.objects.myFactory.otherObject, channel.objects.myOtherObj);

        // trigger a signal and ensure it gets transmitted
        lastFactoryObj.mySignal("foobar", 42);
        client.awaitSignal();

        // deleteLater call
        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        compare(msg.object, testObjId);
        client.awaitResponse();

        // now the signalArgs should also be set
        compare(signalArgs, {"0": "foobar", "1": 42});

        // and also a destroyed signal
        client.awaitSignal();

        compare(JSON.stringify(testObjBeforeDeletion), JSON.stringify({}));
        compare(JSON.stringify(testObjAfterDeletion), JSON.stringify({}));
        compare(typeof channel.objects[testObjId], "undefined");
    }

    // test if returned QObjects get inserted into list of
    // objects even if no callback function is set
    function test_wrapper_wrapEveryQObject()
    {
        var testObj;
        var channel = client.createChannel(function(channel) {
            channel.objects.myFactory.create("testObj", function(obj) {
                testObj = obj;
            });
        });
        client.awaitInit();
        client.awaitResponse();

        // call to myFactory.create()
        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        client.awaitResponse();

        client.awaitIdle();

        verify(testObj);
        var testObjId = testObj.__id__;

        testObj.deleteLater();
        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        compare(msg.object, testObjId);
        client.awaitResponse();
        // destroyed signal
        client.awaitSignal();

        compare(lastFactoryObj, null);
        compare(typeof channel.objects[testObjId], "undefined");
    }

    function test_wrapper_propertyUpdateOfWrappedObjects()
    {
        var testObj;
        var testObjId;
        var channel = client.createChannel(function(channel) {
            channel.objects.myFactory.create("testObj", function(obj) {
                testObj = lastFactoryObj;
                testObjId = obj.__id__;
            });
        });
        client.awaitInit();
        client.awaitResponse();

        // call to myFactory.create()
        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        client.awaitResponse();

        client.awaitIdle();

        testObj.myProperty = 42;
        client.awaitPropertyUpdate();
        client.awaitIdle();
        compare(channel.objects[testObjId].myProperty, 42);

        channel.objects[testObjId].deleteLater();
        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        client.awaitResponse();
        // destroyed signal
        client.awaitSignal();
    }

    function test_disconnect()
    {
        var signalArg;
        var channel = client.createChannel(function(channel) {
            channel.objects.myObj.mySignal.connect(function(arg) {
                signalArg = arg;
                channel.objects.myObj.mySignal.disconnect(this);
            });
        });
        client.awaitInit();

        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.connectToSignal);
        compare(msg.object, "myObj");

        client.awaitIdle();

        myObj.mySignal(42, myObj);
        compare(signalArg, 42);

        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.disconnectFromSignal);
        compare(msg.object, "myObj");

        myObj.mySignal(0, myObj);
        compare(signalArg, 42);
    }

    // see also: https://bugreports.qt.io/browse/QTBUG-54074
    function test_signalArgumentTypeConversion()
    {
        var signalArgs = [];
        function logSignalArgs(arg) {
            signalArgs.push(arg);
        }
        var channel = client.createChannel(function(channel) {
            var testObject = channel.objects.testObject;
            testObject.testSignalBool.connect(logSignalArgs);
            testObject.testSignalInt.connect(logSignalArgs);
            testObject.triggerSignals();
        });
        client.awaitInit();

        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.connectToSignal);
        compare(msg.object, "testObject");

        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.connectToSignal);
        compare(msg.object, "testObject");

        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        client.awaitIdle();

        compare(signalArgs, [
            true,
            false,
            42,
            1,
            0
        ]);
    }

    function test_multiConnect()
    {
        var signalArgs = [];
        function logSignalArgs(arg) {
            signalArgs.push(arg);
        }
        var channel = client.createChannel(function(channel) {
            var testObject = channel.objects.testObject;
            testObject.testSignalInt.connect(logSignalArgs);
            testObject.testSignalInt.connect(logSignalArgs);
            testObject.triggerSignals();
        });
        client.awaitInit();

        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.connectToSignal);
        compare(msg.object, "testObject");

        msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.invokeMethod);
        client.awaitIdle();

        compare(signalArgs, [42, 42, 1, 1, 0, 0]);
    }

    function test_connectDuringEmit()
    {
        var cb1 = 0;
        var cb2 = 0;
        var channel = client.createChannel(function(channel) {
            var myObj = channel.objects.myObj;
            myObj.mySignal.connect(function() {
                cb1++;
                myObj.mySignal.connect(function() {
                    cb2++;
                });
            });
        });
        client.awaitInit();

        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.connectToSignal);
        compare(msg.object, "myObj");

        client.awaitIdle();

        myObj.mySignal(42, myObj);

        compare(cb1, 1);
        compare(cb2, 0);
    }

    function test_disconnectDuringEmit()
    {
        var cb1 = 0;
        var cb2 = 0;
        var cb3 = 0;
        var channel = client.createChannel(function(channel) {
            var myObj = channel.objects.myObj;
            var cb1impl = function() {
                cb1++;
            };
            myObj.mySignal.connect(cb1impl);
            myObj.mySignal.connect(function() {
                cb2++;
                myObj.mySignal.disconnect(cb1impl);
            });
            myObj.mySignal.connect(function() {
                cb3++;
            });
        });
        client.awaitInit();

        var msg = client.awaitMessage();
        compare(msg.type, JSClient.QWebChannelMessageTypes.connectToSignal);
        compare(msg.object, "myObj");

        client.awaitIdle();

        myObj.mySignal(42, myObj);

        compare(cb1, 1);
        compare(cb2, 1);
        compare(cb3, 1);
    }

    function test_overloading()
    {
        var signalArgs_implicit = [];
        var signalArgs_explicit1 = [];
        var signalArgs_explicit2 = [];
        var signalArgs_explicit3 = [];
        function logSignalArgs(container) {
            return function(...args) {
                container.push(args);
            };
        }
        var returnValues = [];
        function logReturnValue(value) {
            returnValues.push(value);
        }
        var channel = client.createChannel(function(channel) {
            var testObject = channel.objects.testObject;
            testObject.testOverloadSignal.connect(logSignalArgs(signalArgs_implicit));
            testObject["testOverloadSignal(int)"].connect(logSignalArgs(signalArgs_explicit1));
            testObject["testOverloadSignal(QString)"].connect(logSignalArgs(signalArgs_explicit2));
            testObject["testOverloadSignal(QString,int)"].connect(logSignalArgs(signalArgs_explicit3));
            testObject.testOverload(99, logReturnValue);
            testObject["testOverload(int)"](41, logReturnValue);
            testObject["testOverload(QString)"]("hello world", logReturnValue);
            testObject["testOverload(QString,int)"]("the answer is ", 41, logReturnValue);
        });
        client.awaitInit();

        function awaitMessage(type)
        {
            var msg = client.awaitMessage();
            compare(msg.type, type);
            compare(msg.object, "testObject");
        }

        console.log("sig1");
        awaitMessage(JSClient.QWebChannelMessageTypes.connectToSignal);
        console.log("sig2");
        awaitMessage(JSClient.QWebChannelMessageTypes.connectToSignal);
        console.log("sig3");
        awaitMessage(JSClient.QWebChannelMessageTypes.connectToSignal);

        console.log("method1");
        awaitMessage(JSClient.QWebChannelMessageTypes.invokeMethod);
        console.log("method2");
        awaitMessage(JSClient.QWebChannelMessageTypes.invokeMethod);
        console.log("method3");
        awaitMessage(JSClient.QWebChannelMessageTypes.invokeMethod);
        console.log("method4");
        awaitMessage(JSClient.QWebChannelMessageTypes.invokeMethod);

        client.awaitIdle();

        compare(signalArgs_implicit, [[99], [41]]);
        compare(signalArgs_explicit1, signalArgs_implicit);
        compare(signalArgs_explicit2, [["hello world"]]);
        compare(signalArgs_explicit3, [["the answer is ", 41]]);
        compare(returnValues, [100, 42, "HELLO WORLD", "THE ANSWER IS 42"]);
    }

    function test_variantType()
    {
        var returnValues = [];
        function logReturnValue(value) {
            returnValues.push(value);
        }
        var channel = client.createChannel(function(channel) {
            var testObject = channel.objects.testObject;
            testObject.testVariantType(0.25, logReturnValue);
            testObject.testVariantType("0", logReturnValue);
            testObject.testVariantType(null, logReturnValue);
            testObject.testVariantType(testObject, logReturnValue);
        });
        client.awaitInit();

        function awaitMessage(type)
        {
            var msg = client.awaitMessage();
            compare(msg.type, type);
            compare(msg.object, "testObject");
        }

        console.log("double arg");
        awaitMessage(JSClient.QWebChannelMessageTypes.invokeMethod);
        console.log("string arg");
        awaitMessage(JSClient.QWebChannelMessageTypes.invokeMethod);
        console.log("null arg");
        awaitMessage(JSClient.QWebChannelMessageTypes.invokeMethod);
        console.log("QObject arg");
        awaitMessage(JSClient.QWebChannelMessageTypes.invokeMethod);

        client.awaitIdle();

        // QMetaType::Double: 6, QMetaType::QString: 10, QMetaType::Nullptr: 51,
        // QMetaType::QObjectStar: 39
        compare(returnValues, [6, 10, 51, 39]);
    }

    function test_embeddedQObject()
    {
        var success = false;
        function logReturnValue(value) {
            success = value;
        }
        var channel = client.createChannel(function(channel) {
            var testObject = channel.objects.testObject;
            testObject.testEmbeddedObjects([testObject, { obj: testObject }], logReturnValue);
        });
        client.awaitInit();

        function awaitMessage(type)
        {
            var msg = client.awaitMessage();
            compare(msg.type, type);
            compare(msg.object, "testObject");
        }

        awaitMessage(JSClient.QWebChannelMessageTypes.invokeMethod);

        client.awaitIdle();

        compare(success, true);
    }

    function test_customUpcaseWrapper()
    {
        var channel = client.createChannel(function(channel) {
            channel.objects.testObject.stringProperty = "foo";
        }, function(arg) { return (typeof arg === "string") ? arg.toUpperCase() : undefined });

        client.awaitInit();
        function awaitMessage(type)
        {
            var msg = client.awaitMessage();
            compare(msg.type, type);
            compare(msg.object, "testObject");
        }
        awaitMessage(JSClient.QWebChannelMessageTypes.setProperty);
        compare(testObject.stringProperty, "foo"); // Don't convert in this direction
        client.awaitIdle(); // init

        testObject.stringProperty = "bar";
        compare(testObject.stringProperty, "bar");
        client.awaitIdle(); // property update
        compare(channel.objects.testObject.stringProperty, "BAR"); // Case converted
    }

    function test_dateWrapper()
    {
        var channel = client.createChannel(undefined, "Date");
        client.awaitInit();
        client.awaitIdle();

        var dateString = "2022-01-01T10:00:00Z";
        myValueObj.value = dateString;
        compare(myValueObj.value, dateString);
        client.awaitIdle(); // property update
        var value = channel.objects.myValueObj.value;
        verify(value instanceof Date);
        verify(!isNaN(value));
        compare(value.getUTCFullYear(), 2022);
        compare(value.getUTCMonth(), 0); // 0 = January
        compare(value.getUTCDate(), 1);
        compare(value.getUTCHours(), 10);
        compare(value.getUTCMinutes(), 0);
        compare(value.getUTCSeconds(), 0);

        var invalidDate = "2022-13-31T10:00:00Z"; // Month after december
        myValueObj.value = invalidDate;
        compare(myValueObj.value, invalidDate);
        client.awaitIdle(); // property update
        value = channel.objects.myValueObj.value;
        verify(typeof value === "string"); // Not converted to Date
        compare(value, invalidDate);
    }

    function test_registerNull()
    {
        nullTest.registeredObjects = [null]
        compare(nullTest.registeredObjects, [])
    }

    function test_noContext()
    {
        nullTest.registeredObjects = [testObject.createQObject()]
        compare(nullTest.registeredObjects, [])
    }
}
