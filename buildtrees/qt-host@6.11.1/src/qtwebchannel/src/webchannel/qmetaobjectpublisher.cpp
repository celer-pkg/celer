// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// Copyright (C) 2019 Menlo Systems GmbH, author Arno Rehn <a.rehn@menlosystems.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qmetaobjectpublisher_p.h"
#include "qwebchannel.h"
#include "qwebchannel_p.h"
#include "qwebchannelabstracttransport.h"

#include <QEvent>
#if QT_CONFIG(future)
#include <QFuture>
#endif
#include <QJsonDocument>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#ifndef QT_NO_JSVALUE
#include <QJSValue>
#endif
#include <QUuid>

#include <QtCore/private/qmetaobject_p.h>

QT_BEGIN_NAMESPACE

namespace {

// FIXME: QFlags don't have the QMetaType::IsEnumeration flag set, although they have a QMetaEnum entry in the QMetaObject.
// They only way to detect registered QFlags types is to find the named entry in the QMetaObject's enumerator list.
// Ideally, this would be fixed in QMetaType.
bool isQFlagsType(uint id)
{
    QMetaType type(id);

    // Short-circuit to avoid more expensive operations
    QMetaType::TypeFlags flags = type.flags();
    if (flags.testFlag(QMetaType::PointerToQObject) || flags.testFlag(QMetaType::IsEnumeration)
            || flags.testFlag(QMetaType::SharedPointerToQObject) || flags.testFlag(QMetaType::WeakPointerToQObject)
            || flags.testFlag(QMetaType::TrackingPointerToQObject) || flags.testFlag(QMetaType::IsGadget))
    {
        return false;
    }

    const QMetaObject *mo = type.metaObject();
    if (!mo) {
        return false;
    }

    QByteArray name = type.name();
    name = name.mid(name.lastIndexOf(":") + 1);
    return mo->indexOfEnumerator(name.constData()) > -1;
}

// Common scores for overload resolution
enum OverloadScore {
    PerfectMatchScore = 0,
    VariantScore = 1,
    NumberBaseScore = 2,
    GenericConversionScore = 100,
    IncompatibleScore = 10000,
};

// Scores the conversion of a double to a number-like user type. Better matches
// for a JS 'number' get a lower score.
int doubleToNumberConversionScore(int userType)
{
    switch (userType) {
    case QMetaType::Bool:
        return NumberBaseScore + 7;
    case QMetaType::Char:
    case QMetaType::SChar:
    case QMetaType::UChar:
        return NumberBaseScore + 6;
    case QMetaType::Short:
    case QMetaType::UShort:
        return NumberBaseScore + 5;
    case QMetaType::Int:
    case QMetaType::UInt:
        return NumberBaseScore + 4;
    case QMetaType::Long:
    case QMetaType::ULong:
        return NumberBaseScore + 3;
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
        return NumberBaseScore + 2;
    case QMetaType::Float:
        return NumberBaseScore + 1;
    case QMetaType::Double:
        return NumberBaseScore;
    default:
        break;
    }

    if (QMetaType(userType).flags() & QMetaType::IsEnumeration)
        return doubleToNumberConversionScore(QMetaType::Int);

    return IncompatibleScore;
}

// Keeps track of the badness of a QMetaMethod candidate for overload resolution
struct OverloadResolutionCandidate
{
    OverloadResolutionCandidate(const QMetaMethod &method = QMetaMethod(), int badness = PerfectMatchScore)
        : method(method), badness(badness)
    {}

    QMetaMethod method;
    int badness;

    bool operator<(const OverloadResolutionCandidate &other) const { return badness < other.badness; }
};

MessageType toType(const QJsonValue &value)
{
    int i = value.toInt(-1);
    if (i >= TYPES_FIRST_VALUE && i <= TYPES_LAST_VALUE) {
        return static_cast<MessageType>(i);
    } else {
        return TypeInvalid;
    }
}

const QString KEY_SIGNALS = QStringLiteral("signals");
const QString KEY_METHODS = QStringLiteral("methods");
const QString KEY_PROPERTIES = QStringLiteral("properties");
const QString KEY_ENUMS = QStringLiteral("enums");
const QString KEY_QOBJECT = QStringLiteral("__QObject*__");
const QString KEY_ID = QStringLiteral("id");
const QString KEY_DATA = QStringLiteral("data");
const QString KEY_OBJECT = QStringLiteral("object");
const QString KEY_DESTROYED = QStringLiteral("destroyed");
const QString KEY_SIGNAL = QStringLiteral("signal");
const QString KEY_TYPE = QStringLiteral("type");
const QString KEY_METHOD = QStringLiteral("method");
const QString KEY_ARGS = QStringLiteral("args");
const QString KEY_PROPERTY = QStringLiteral("property");
const QString KEY_VALUE = QStringLiteral("value");

QJsonObject createResponse(const QJsonValue &id, const QJsonValue &data)
{
    QJsonObject response;
    response[KEY_TYPE] = TypeResponse;
    response[KEY_ID] = id;
    response[KEY_DATA] = data;
    return response;
}

#if QT_CONFIG(future)
QMetaType resultTypeOfQFuture(QByteArrayView typeName)
{
    if (!typeName.startsWith("QFuture<") || !typeName.endsWith('>'))
        return {};

    return QMetaType::fromName(typeName.sliced(8, typeName.size() - 9));
}

template<typename Func>
void attachContinuationToFutureInVariant(const QVariant &result, QPointer<QObject> contextObject,
                                         Func continuation)
{
    Q_ASSERT(result.canConvert<QFuture<void>>());

    // QMetaObject::invokeMethod() indirection to work around an issue with passing
    // a context object to QFuture::then(). See below.
    const auto safeContinuation = [contextObject, continuation=std::move(continuation)]
            (const QVariant &result)
    {
        if (!contextObject)
            return;

        QMetaObject::invokeMethod(contextObject.get(), [continuation, result] {
            continuation(result);
        });
    };

    auto f = result.value<QFuture<void>>();

    // Be explicit about what we capture so that we don't accidentally run into
    // threading issues.
    f.then([resultType=resultTypeOfQFuture(result.typeName()), f, continuation=safeContinuation]
    {
        if (!resultType.isValid() || resultType == QMetaType::fromType<void>()) {
            continuation(QVariant{});
            return;
        }

        auto iface = QFutureInterfaceBase::get(f);
        // If we pass a context object to f.then() and the future originates in a
        // different thread, this assertions fails. Why?
        // For the time being, work around that with QMetaObject::invokeMethod()
        // in safeSendResponse().
        Q_ASSERT(iface.resultCount() > 0);

        QMutexLocker<QMutex> locker(&iface.mutex());
        if (iface.resultStoreBase().resultAt(0).isVector()) {
            locker.unlock();
            // This won't work because we cannot generically get a QList<T> into
            // a QVariant with T only known at runtime.
            // TBH, I don't know how to trigger this.
            qWarning() << "Result lists in a QFuture return value are not supported!";
            continuation(QVariant{});
            return;
        }

        // pointer<void>() wouldn't compile because of the isVector-codepath
        // using QList<T> in that method. We're not taking that path anyway (see the
        // above check), so we can use char instead to not break strict aliasing
        // requirements.
        const auto data = iface.resultStoreBase().resultAt(0).pointer<char>();
        locker.unlock();

        const QVariant result(resultType, data);
        continuation(result);
    }).onCanceled([continuation=safeContinuation] {
        // Will catch both failure and cancellation.
        // Maybe send something more meaningful?
        continuation(QVariant{});
    });
}
#endif

}

Q_DECLARE_TYPEINFO(OverloadResolutionCandidate, Q_RELOCATABLE_TYPE);

void QWebChannelPropertyChangeNotifier::notify(
        QPropertyObserver *self, QUntypedPropertyData *)
{
    auto This = static_cast<QWebChannelPropertyChangeNotifier*>(self);

    // Use the indirection with Qt::AutoConnection to ensure invocation
    // in the correct thread.
    // Explicitly copy the parameters into the lambda so that this instance can be destroyed after posting a queued
    // invocation. The current design doesn't allow this anyway, but I don't want bad surprises in a possible future
    // commit.
    QMetaObject::invokeMethod(
                This->publisher,
                [publisher=This->publisher, object=This->object, propertyIndex=This->propertyIndex]
    {
        publisher->propertyValueChanged(object, propertyIndex);
    }, Qt::AutoConnection);
}

QMetaObjectPublisher::QMetaObjectPublisher(QWebChannel *webChannel)
    : QObject(webChannel),
      webChannel(webChannel),
      blockUpdatesStatus(false),
      blockUpdatesHandler(blockUpdatesStatus.onValueChanged(
              std::function<void()>([&]() { this->onBlockUpdatesChanged(); }))),
      propertyUpdatesInitialized(false),
      propertyUpdateIntervalTime(50),
      propertyUpdateIntervalHandler(propertyUpdateIntervalTime.onValueChanged(
              std::function<void()>([&]() { this->startPropertyUpdateTimer(true); })))
{
}

QMetaObjectPublisher::~QMetaObjectPublisher()
{

}

void QMetaObjectPublisher::registerObject(const QString &id, QObject *object)
{
    registeredObjects[id] = object;
    registeredObjectIds[object] = id;
    if (propertyUpdatesInitialized) {
        if (!webChannel->d_func()->transports.isEmpty()) {
            qWarning("Registered new object after initialization, existing clients won't be notified!");
            // TODO: send a message to clients that an object was added
        }
        initializePropertyUpdates(object, classInfoForObject(object, nullptr));
    }
}

QJsonObject QMetaObjectPublisher::classInfoForObject(const QObject *object, QWebChannelAbstractTransport *transport)
{
    QJsonObject data;
    if (!object) {
        qWarning("null object given to MetaObjectPublisher - bad API usage?");
        return data;
    }

    QJsonArray qtSignals;
    QJsonArray qtMethods;
    QJsonArray qtProperties;
    QJsonObject qtEnums;

    const QMetaObject *metaObject = object->metaObject();
    QSet<int> notifySignals;
    QSet<QString> identifiers;
    for (int i = 0; i < metaObject->propertyCount(); ++i) {
        const QMetaProperty &prop = metaObject->property(i);
        QJsonArray propertyInfo;
        const QString &propertyName = QString::fromLatin1(prop.name());
        propertyInfo.append(i);
        propertyInfo.append(propertyName);
        identifiers << propertyName;
        QJsonArray signalInfo;
        if (prop.hasNotifySignal()) {
            notifySignals << prop.notifySignalIndex();
            // optimize: compress the common propertyChanged notification names, just send a 1
            const QByteArray &notifySignal = prop.notifySignal().name();
            static const QByteArray changedSuffix = QByteArrayLiteral("Changed");
            if (notifySignal.size() == changedSuffix.size() + propertyName.size() &&
                notifySignal.endsWith(changedSuffix) && notifySignal.startsWith(prop.name()))
            {
                signalInfo.append(1);
            } else {
                signalInfo.append(QString::fromLatin1(notifySignal));
            }
            signalInfo.append(prop.notifySignalIndex());
        } else if (!prop.isConstant() && !prop.isBindable()) {
            qWarning("Property '%s'' of object '%s' has no notify signal, is not bindable and is not constant, "
                     "value updates in HTML will be broken!",
                     prop.name(), object->metaObject()->className());
        }
        propertyInfo.append(signalInfo);
        propertyInfo.append(wrapResult(prop.read(object), transport));
        qtProperties.append(propertyInfo);
    }
    auto addMethod = [&qtSignals, &qtMethods, &identifiers](int i, const QMetaMethod &method, const QByteArray &rawName) {
        //NOTE: the name must be a string, otherwise it will be converted to '{}' in QML
        const auto name = QString::fromLatin1(rawName);
        // only the first method gets called with its name directly
        // others must be called by explicitly passing the method signature
        if (identifiers.contains(name))
            return;
        identifiers << name;
        // send data as array to client with format: [name, index]
        QJsonArray data;
        data.append(name);
        data.append(i);
        if (method.methodType() == QMetaMethod::Signal) {
            qtSignals.append(data);
        } else if (method.access() == QMetaMethod::Public) {
            qtMethods.append(data);
        }
    };
    for (int i = 0; i < metaObject->methodCount(); ++i) {
        if (notifySignals.contains(i)) {
            continue;
        }
        const QMetaMethod &method = metaObject->method(i);
        addMethod(i, method, method.name());
        // for overload resolution also pass full method signature
        addMethod(i, method, method.methodSignature());
    }
    for (int i = 0; i < metaObject->enumeratorCount(); ++i) {
        QMetaEnum enumerator = metaObject->enumerator(i);
        QJsonObject values;
        for (int k = 0; k < enumerator.keyCount(); ++k) {
            values[QString::fromLatin1(enumerator.key(k))] = enumerator.value(k);
        }
        qtEnums[QString::fromLatin1(enumerator.name())] = values;
    }
    data[KEY_SIGNALS] = qtSignals;
    data[KEY_METHODS] = qtMethods;
    data[KEY_PROPERTIES] = qtProperties;
    if (!qtEnums.isEmpty()) {
        data[KEY_ENUMS] = qtEnums;
    }
    return data;
}

void QMetaObjectPublisher::setClientIsIdle(bool isIdle, QWebChannelAbstractTransport *transport)
{
    transportState[transport].clientIsIdle = isIdle;
    if (isIdle)
        sendEnqueuedPropertyUpdates(transport);
}

bool QMetaObjectPublisher::isClientIdle(QWebChannelAbstractTransport *transport)
{
    auto found = transportState.find(transport);
    return found != transportState.end() && found.value().clientIsIdle;
}

QJsonObject QMetaObjectPublisher::initializeClient(QWebChannelAbstractTransport *transport)
{
    QJsonObject objectInfos;
    {
        const QHash<QString, QObject *>::const_iterator end = registeredObjects.constEnd();
        for (QHash<QString, QObject *>::const_iterator it = registeredObjects.constBegin(); it != end; ++it) {
            const QJsonObject &info = classInfoForObject(it.value(), transport);
            if (!propertyUpdatesInitialized) {
                initializePropertyUpdates(it.value(), info);
            }
            objectInfos[it.key()] = info;
        }
    }
    propertyUpdatesInitialized = true;
    return objectInfos;
}

void QMetaObjectPublisher::initializePropertyUpdates(QObject *const object, const QJsonObject &objectInfo)
{
    auto *metaObject = object->metaObject();
    auto *signalHandler = signalHandlerFor(object);
    for (const auto propertyInfoVar : objectInfo[KEY_PROPERTIES].toArray()) {
        const QJsonArray &propertyInfo = propertyInfoVar.toArray();
        if (propertyInfo.size() < 2) {
            qWarning() << "Invalid property info encountered:" << propertyInfoVar;
            continue;
        }
        const int propertyIndex = propertyInfo.at(0).toInt();
        const QJsonArray &signalData = propertyInfo.at(2).toArray();
        const QMetaProperty metaProp = metaObject->property(propertyIndex);

        if (!signalData.isEmpty()) {
            // Property with a NOTIFY signal
            const int signalIndex = signalData.at(1).toInt();

            QSet<int> &connectedProperties = signalToPropertyMap[object][signalIndex];

            // Only connect for a property update once
            if (connectedProperties.isEmpty()) {
                signalHandler->connectTo(object, signalIndex);
            }

            connectedProperties.insert(propertyIndex);
        } else if (metaProp.isBindable()) {
            const auto [begin, end] = propertyObservers.equal_range(object);
            const auto it = std::find_if(begin, end, [&](auto &n) {
                return n.second.propertyIndex == propertyIndex;
            });
            // Only connect for a property update once
            if (it == end) {
                auto it = propertyObservers.emplace(
                            object, QWebChannelPropertyChangeNotifier{this, object, propertyIndex});
                metaProp.bindable(object).observe(&it->second);
            }
        }
    }

    // also always connect to destroyed signal
    signalHandler->connectTo(object, s_destroyedSignalIndex);
}

void QMetaObjectPublisher::sendPendingPropertyUpdates()
{
    if (blockUpdatesStatus) {
        return;
    }

    QJsonArray data;
    QHash<QWebChannelAbstractTransport*, QJsonArray> specificUpdates;

    // convert pending property updates to JSON data
    const PendingPropertyUpdates::const_iterator end = pendingPropertyUpdates.constEnd();
    for (PendingPropertyUpdates::const_iterator it = pendingPropertyUpdates.constBegin(); it != end; ++it) {
        const QObject *object = it.key();
        const QMetaObject *const metaObject = object->metaObject();
        const QString objectId = registeredObjectIds.value(object);
        const SignalToPropertyNameMap &objectsSignalToPropertyMap = signalToPropertyMap.value(object);
        // maps property name to current property value
        QJsonObject properties;
        // maps signal index to list of arguments of the last emit
        QJsonObject sigs;

        const auto indexes = it.value().propertyIndices(objectsSignalToPropertyMap);

        // TODO: can we get rid of the int <-> string conversions here?
        for (const int propertyIndex : indexes) {
            const QMetaProperty &property = metaObject->property(propertyIndex);
            Q_ASSERT(property.isValid());
            properties[QString::number(propertyIndex)] = wrapResult(property.read(object), nullptr, objectId);
        }

        const auto sigMap = it.value().signalMap;
        for (auto sigIt = sigMap.constBegin(); sigIt != sigMap.constEnd(); ++sigIt) {
            sigs[QString::number(sigIt.key())] = QJsonArray::fromVariantList(sigIt.value());
        }

        QJsonObject obj;
        obj[KEY_OBJECT] = objectId;
        obj[KEY_SIGNALS] = sigs;
        obj[KEY_PROPERTIES] = properties;

        // if the object is auto registered, just send the update only to clients which know this object
        if (wrappedObjects.contains(objectId)) {
            for (QWebChannelAbstractTransport *transport : wrappedObjects.value(objectId).transports) {
                QJsonArray &arr = specificUpdates[transport];
                arr.push_back(obj);
            }
        } else {
            data.push_back(obj);
        }
    }

    pendingPropertyUpdates.clear();
    QJsonObject message;
    message[KEY_TYPE] = TypePropertyUpdate;

    // data does not contain specific updates
    if (!data.isEmpty()) {
        message[KEY_DATA] = data;
        enqueueBroadcastMessage(message);
    }

    // send every property update which is not supposed to be broadcasted
    const QHash<QWebChannelAbstractTransport*, QJsonArray>::const_iterator suend = specificUpdates.constEnd();
    for (QHash<QWebChannelAbstractTransport*, QJsonArray>::const_iterator it = specificUpdates.constBegin(); it != suend; ++it) {
        message[KEY_DATA] = it.value();
        enqueueMessage(message, it.key());
    }

    for (auto state = transportState.begin(); state != transportState.end(); ++state)
        sendEnqueuedPropertyUpdates(state.key());
}

QVariant QMetaObjectPublisher::invokeMethod_helper(QObject *const object, const QMetaMethod &method,
                                                   const QJsonArray &args)
{
    // a good value for the number of arguments we'll preallocate in QVLA
    constexpr qsizetype ArgumentCount = 16;

    QVarLengthArray<QVariant, ArgumentCount> variants;
    QVarLengthArray<const char *, ArgumentCount> names(method.parameterCount() + 1);
    QVarLengthArray<void *, ArgumentCount> parameters(names.size());
    QVarLengthArray<const QtPrivate::QMetaTypeInterface *, ArgumentCount> metaTypes(names.size());
    variants.reserve(names.size());
    variants << QVariant();

    // start with the formal parameters
    for (qsizetype i = 0; i < names.size() - 1; ++i) {
        QMetaType mt = method.parameterMetaType(i);
        QVariant &v = variants.emplace_back(toVariant(args.at(i), mt.id()));
        parameters[i + 1] = v.data();
        names[i + 1] = mt.name();
        metaTypes[i + 1] = mt.iface();
    }

    // now, the return type
    QMetaType mt = method.returnMetaType();
    names[0] = mt.name();
    metaTypes[0] = mt.iface();
    if (int id = mt.id(); id != QMetaType::Void) {
        // Only init variant with return type if its not a variant itself,
        // which would lead to nested variants which is not what we want.
        if (id == QMetaType::QVariant) {
            parameters[0] = &variants[0];
        } else {
            variants[0] = QVariant(mt);
            parameters[0] = variants[0].data();
        }
    } else {
        parameters[0] = nullptr;
    }

    // step 3: make the call
    QMetaMethodInvoker::InvokeFailReason r =
            QMetaMethodInvoker::invokeImpl(method, object, Qt::AutoConnection,
                                           parameters.size(), parameters.constData(),
                                           names.constData(), metaTypes.constData());

    if (r == QMetaMethodInvoker::InvokeFailReason::None)
        return variants.first();

    // print warnings for failures to match
    if (int(r) >= int(QMetaMethodInvoker::InvokeFailReason::FormalParameterMismatch)) {
        int n = int(r) - int(QMetaMethodInvoker::InvokeFailReason::FormalParameterMismatch);
        QByteArray callee = object->metaObject()->className() + QByteArrayView("::")
                + method.methodSignature();
        qWarning() << "Cannot convert formal parameter" << n << "from" << names[n + 1]
                   << "in call to" << callee.constData();
    }

    return QJsonValue();
}

QVariant QMetaObjectPublisher::invokeMethod(QObject *const object, const QMetaMethod &method,
                                              const QJsonArray &args)
{
    if (method.name() == QByteArrayLiteral("deleteLater")) {
        // invoke `deleteLater` on wrapped QObject indirectly
        deleteWrappedObject(object);
        return QJsonValue();
    } else if (!method.isValid()) {
        qWarning() << "Cannot invoke invalid method on object" << object << '.';
        return QJsonValue();
    } else if (method.access() != QMetaMethod::Public) {
        qWarning() << "Cannot invoke non-public method" << method.name() << "on object" << object << '.';
        return QJsonValue();
    } else if (method.methodType() != QMetaMethod::Method && method.methodType() != QMetaMethod::Slot) {
        qWarning() << "Cannot invoke non-public method" << method.name() << "on object" << object << '.';
        return QJsonValue();
    } else if (args.size() > method.parameterCount()) {
        qWarning() << "Ignoring additional arguments while invoking method" << method.name() << "on object" << object << ':'
                   << args.size() << "arguments given, but method only takes" << method.parameterCount() << '.';
    }

    return invokeMethod_helper(object, method, args);
}

QVariant QMetaObjectPublisher::invokeMethod(QObject *const object, const int methodIndex,
                                            const QJsonArray &args)
{
    const QMetaMethod &method = object->metaObject()->method(methodIndex);
    if (!method.isValid()) {
        qWarning() << "Cannot invoke method of unknown index" << methodIndex << "on object"
                   << object << '.';
        return QJsonValue();
    }
    return invokeMethod(object, method, args);
}

QVariant QMetaObjectPublisher::invokeMethod(QObject *const object, const QByteArray &methodName,
                                            const QJsonArray &args)
{
    QList<OverloadResolutionCandidate> candidates;

    const QMetaObject *mo = object->metaObject();
    for (int i = 0; i < mo->methodCount(); ++i) {
        QMetaMethod method = mo->method(i);
        if (method.name() != methodName || method.parameterCount() != args.count()
                || method.access() != QMetaMethod::Public
                || (method.methodType() != QMetaMethod::Method
                        && method.methodType() != QMetaMethod::Slot))
        {
            // Not a candidate
            continue;
        }

        candidates.append({method, methodOverloadBadness(method, args)});
    }

    if (candidates.isEmpty()) {
        qWarning() << "No candidates found for" << methodName << "with" << args.size()
                   << "arguments on object" << object << '.';
        return QJsonValue();
    }

    std::sort(candidates.begin(), candidates.end());
    if (candidates.size() > 1 && candidates[0].badness == candidates[1].badness) {
        qWarning().nospace() << "Ambiguous overloads for method " << methodName << ". Choosing "
                             << candidates.first().method.methodSignature();

    }

    return invokeMethod_helper(object, candidates.first().method, args);
}

void QMetaObjectPublisher::setProperty(QObject *object, const int propertyIndex, const QJsonValue &value)
{
    QMetaProperty property = object->metaObject()->property(propertyIndex);
    if (!property.isValid()) {
        qWarning() << "Cannot set unknown property" << propertyIndex << "of object" << object;
    } else if (!property.write(object, toVariant(value, property.userType()))) {
        qWarning() << "Could not write value " << value << "to property" << property.name() << "of object" << object;
    }
}

void QMetaObjectPublisher::signalEmitted(const QObject *object, const int signalIndex, const QVariantList &arguments)
{
    if (!webChannel || webChannel->d_func()->transports.isEmpty()) {
        if (signalIndex == s_destroyedSignalIndex)
            objectDestroyed(object);
        return;
    }
    if (!signalToPropertyMap.value(object).contains(signalIndex)) {
        QJsonObject message;
        const QString &objectName = registeredObjectIds.value(object);
        Q_ASSERT(!objectName.isEmpty());
        message[KEY_OBJECT] = objectName;
        message[KEY_SIGNAL] = signalIndex;
        if (!arguments.isEmpty()) {
            message[KEY_ARGS] = wrapList(arguments, nullptr, objectName);
        }
        message[KEY_TYPE] = TypeSignal;

        // if the object is wrapped, just send the response to clients which know this object
        if (wrappedObjects.contains(objectName)) {
            for (QWebChannelAbstractTransport *transport : wrappedObjects.value(objectName).transports) {
                transport->sendMessage(message);
            }
        } else {
            broadcastMessage(message);
        }

        if (signalIndex == s_destroyedSignalIndex) {
            objectDestroyed(object);
        }
    } else {
        auto &propertyUpdate = pendingPropertyUpdates[object];
        propertyUpdate.signalMap[signalIndex] = arguments;
        startPropertyUpdateTimer();
    }
}

void QMetaObjectPublisher::propertyValueChanged(const QObject *object, const int propertyIndex)
{
    pendingPropertyUpdates[object].plainProperties.insert(propertyIndex);
    startPropertyUpdateTimer();
}

void QMetaObjectPublisher::startPropertyUpdateTimer(bool forceRestart)
{
    if (blockUpdatesStatus)
        return;

    if (propertyUpdateIntervalTime >= 0) {
        if (forceRestart || !timer.isActive())
            timer.start(propertyUpdateIntervalTime, this);
    } else {
        sendPendingPropertyUpdates();
    }
}

void QMetaObjectPublisher::objectDestroyed(const QObject *object)
{
    const QString &id = registeredObjectIds.take(object);
    Q_ASSERT(!id.isEmpty());
    bool removed = registeredObjects.remove(id)
            || wrappedObjects.remove(id);
    Q_ASSERT(removed);
    Q_UNUSED(removed);

    // only remove from handler when we initialized the property updates
    // cf: https://bugreports.qt.io/browse/QTBUG-60250
    if (propertyUpdatesInitialized) {
        signalHandlerFor(object)->remove(object);
        signalToPropertyMap.remove(object);
    }
    pendingPropertyUpdates.remove(object);
    propertyObservers.erase(object);
}

QObject *QMetaObjectPublisher::unwrapObject(const QString &objectId) const
{
    if (!objectId.isEmpty()) {
        ObjectInfo objectInfo = wrappedObjects.value(objectId);
        if (objectInfo.object)
            return objectInfo.object;
        QObject *object = registeredObjects.value(objectId);
        if (object)
            return object;
    }

    qWarning() << "No wrapped object" << objectId;
    return nullptr;
}

QVariant QMetaObjectPublisher::unwrapMap(QVariantMap map) const
{
    const auto qobj = map.value(KEY_QOBJECT).toBool();
    const auto id = qobj ? map.value(KEY_ID).toString() : QString();

    if (!id.isEmpty()) // it's probably a QObject
        return QVariant::fromValue(unwrapObject(id));

    // it's probably just a normal JS object, continue searching for objects
    // that look like QObject*
    for (auto &value : map)
        value = unwrapVariant(value);

    return map;
}

QVariant QMetaObjectPublisher::unwrapList(QVariantList list) const
{
    for (auto &value : list)
        value = unwrapVariant(value);

    return list;
}

QVariant QMetaObjectPublisher::unwrapVariant(const QVariant &value) const
{
    switch (value.metaType().id())
    {
        case QMetaType::QVariantList:
            return unwrapList(value.toList());
        case QMetaType::QVariantMap:
            return unwrapMap(value.toMap());
        default:
            break;
    }
    return value;
}

QVariant QMetaObjectPublisher::toVariant(const QJsonValue &value, int targetType) const
{
    QMetaType target(targetType);

    if (target.flags() & QMetaType::PointerToQObject) {
        QObject *unwrappedObject = unwrapObject(value.toObject()[KEY_ID].toString());
        if (unwrappedObject == nullptr)
            qWarning() << "Cannot not convert non-object argument" << value << "to QObject*.";
        return QVariant::fromValue(unwrappedObject);
    } else if (isQFlagsType(targetType)) {
        int flagsValue = value.toInt();
        return QVariant(target, reinterpret_cast<const void*>(&flagsValue));
    }

    QVariant variant = value.toVariant();
    // Try variant conversion to the target type first. If that fails,
    // try conversion over QJsonvalue.
    if (auto converted = variant; converted.convert(target)) {
        variant = std::move(converted);
    } else if (targetType != QMetaType::QVariant) {
        if (QVariant converted = value; converted.convert(target))
            variant = std::move(converted);
        else
            qWarning() << "Could not convert argument" << value << "to target type" << target.name() << '.';
    }
    return unwrapVariant(variant);
}

int QMetaObjectPublisher::conversionScore(const QJsonValue &value, int targetType) const
{
    QMetaType target(targetType);
    if (targetType == QMetaType::QJsonValue) {
        return PerfectMatchScore;
    } else if (targetType == QMetaType::QJsonArray) {
        return value.isArray() ? PerfectMatchScore : IncompatibleScore;
    } else if (targetType == QMetaType::QJsonObject) {
        return value.isObject() ? PerfectMatchScore : IncompatibleScore;
    } else if (target.flags() & QMetaType::PointerToQObject) {
        if (value.isNull())
            return PerfectMatchScore;
        if (!value.isObject())
            return IncompatibleScore;

        QJsonObject object = value.toObject();
        if (object[KEY_ID].isUndefined())
            return IncompatibleScore;

        QObject *unwrappedObject = unwrapObject(object[KEY_ID].toString());
        return unwrappedObject != nullptr ? PerfectMatchScore : IncompatibleScore;
    } else if (targetType == QMetaType::QVariant) {
        return VariantScore;
    }

    // Check if this is a number conversion
    if (value.isDouble()) {
        int score = doubleToNumberConversionScore(targetType);
        if (score != IncompatibleScore) {
            return score;
        }
    }

    QVariant variant = value.toVariant();
    if (variant.userType() == targetType) {
        return PerfectMatchScore;
    } else if (variant.canConvert(target)) {
        return GenericConversionScore;
    }

    return IncompatibleScore;
}

int QMetaObjectPublisher::methodOverloadBadness(const QMetaMethod &method, const QJsonArray &args) const
{
    int badness = PerfectMatchScore;
    for (int i = 0; i < args.size(); ++i) {
        badness += conversionScore(args[i], method.parameterType(i));
    }
    return badness;
}

void QMetaObjectPublisher::transportRemoved(QWebChannelAbstractTransport *transport)
{
    auto it = transportedWrappedObjects.find(transport);
    // It is not allowed to modify a container while iterating over it. So save
    // objects which should be removed and call objectDestroyed() on them later.
    QList<QObject *> objectsForDeletion;
    while (it != transportedWrappedObjects.end() && it.key() == transport) {
        if (wrappedObjects.contains(it.value())) {
            QList<QWebChannelAbstractTransport *> &transports = wrappedObjects[it.value()].transports;
            transports.removeOne(transport);
            if (transports.isEmpty())
                objectsForDeletion.append(wrappedObjects[it.value()].object);
        }

        it++;
    }

    transportedWrappedObjects.remove(transport);

    for (QObject *obj : std::as_const(objectsForDeletion))
        objectDestroyed(obj);
}

// NOTE: transport can be a nullptr
//       in such a case, we need to ensure that the property is registered to
//       the target transports of the parentObjectId
QJsonValue QMetaObjectPublisher::wrapResult(const QVariant &result, QWebChannelAbstractTransport *transport,
                                            const QString &parentObjectId)
{
    if (QObject *object = result.value<QObject *>()) {
        QString id = registeredObjectIds.value(object);

        QJsonObject classInfo;
        if (id.isEmpty()) {
            // neither registered, nor wrapped, do so now
            id = QUuid::createUuid().toString();
            // store ID before the call to classInfoForObject()
            // in case of self-contained objects it avoids
            // infinite loops
            registeredObjectIds[object] = id;

            classInfo = classInfoForObject(object, transport);

            ObjectInfo oi(object);
            if (transport) {
                oi.transports.append(transport);
                transportedWrappedObjects.insert(transport, id);
            } else {
                // use the transports from the parent object
                oi.transports = wrappedObjects.value(parentObjectId).transports;
                // or fallback to all transports if the parent is not wrapped
                if (oi.transports.isEmpty())
                    oi.transports = webChannel->d_func()->transports;

                for (auto transport : std::as_const(oi.transports)) {
                    transportedWrappedObjects.insert(transport, id);
                }
            }
            wrappedObjects.insert(id, oi);

            initializePropertyUpdates(object, classInfo);
        } else {
            auto oi = wrappedObjects.find(id);
            if (oi != wrappedObjects.end() && !oi->isBeingWrapped) {
                Q_ASSERT(object == oi->object);
                // check if this transport is already assigned to the object
                if (transport && !oi->transports.contains(transport)) {
                    oi->transports.append(transport);
                    transportedWrappedObjects.insert(transport, id);
                }
                // QTBUG-84007: Block infinite recursion for self-contained objects
                // which have already been wrapped
                oi->isBeingWrapped = true;
                classInfo = classInfoForObject(object, transport);
                oi->isBeingWrapped = false;
            }
        }

        QJsonObject objectInfo;
        objectInfo[KEY_QOBJECT] = true;
        objectInfo[KEY_ID] = id;
        if (!classInfo.isEmpty())
            objectInfo[KEY_DATA] = classInfo;

        return objectInfo;
    } else if (result.metaType().flags().testFlag(QMetaType::IsEnumeration)) {
        return result.toInt();
    } else if (isQFlagsType(result.userType())) {
        return *reinterpret_cast<const int*>(result.constData());
#ifndef QT_NO_JSVALUE
    } else if (result.canConvert<QJSValue>()) {
        // Workaround for keeping QJSValues from QVariant.
        // Calling QJSValue::toVariant() converts JS-objects/arrays to QVariantMap/List
        // instead of stashing a QJSValue itself into a variant.
        // TODO: Improve QJSValue-QJsonValue conversion in Qt.
        return wrapResult(result.value<QJSValue>().toVariant(), transport, parentObjectId);
#endif
    } else if (result.metaType().id() == QMetaType::QString ||
               result.metaType().id() == QMetaType::QByteArray) {
        // avoid conversion to QVariantList
    } else if (result.canConvert<QVariantList>()) {
        // recurse and potentially wrap contents of the array
        // *don't* use result.toList() as that *only* works for QVariantList and QStringList!
        // Also, don't use QMetaSequence::Iterable (yet), since that seems to trigger QTBUG-42016
        // in certain cases.
        // additionally, when there's a direct converter to QVariantList, use that one via convert
        // but recover when conversion fails and fall back to the .value<QVariantList> conversion
        // see also: https://bugreports.qt.io/browse/QTBUG-80751
        auto list = result;
        if (!list.convert(QMetaType::fromType<QVariantList>()))
            list = result;
        return wrapList(list.value<QVariantList>(), transport);
    } else if (result.canConvert<QVariantMap>()) {
        // recurse and potentially wrap contents of the map
        auto map = result;
        if (!map.convert(QMetaType::fromType<QVariantMap>()))
            map = result;
        return wrapMap(map.value<QVariantMap>(), transport);
    } else if (auto v = result; v.convert(QMetaType::fromType<QJsonValue>())) {
        // Support custom converters to QJsonValue
        return v.value<QJsonValue>();
    }

    return QJsonValue::fromVariant(result);
}

QJsonArray QMetaObjectPublisher::wrapList(const QVariantList &list, QWebChannelAbstractTransport *transport, const QString &parentObjectId)
{
    QJsonArray array;
    for (const QVariant &arg : list) {
        array.append(wrapResult(arg, transport, parentObjectId));
    }
    return array;
}

QJsonObject QMetaObjectPublisher::wrapMap(const QVariantMap &map, QWebChannelAbstractTransport *transport, const QString &parentObjectId)
{
    QJsonObject obj;
    for (QVariantMap::const_iterator i = map.begin(); i != map.end(); i++) {
        obj.insert(i.key(), wrapResult(i.value(), transport, parentObjectId));
    }
    return obj;
}

void QMetaObjectPublisher::deleteWrappedObject(QObject *object) const
{
    if (!wrappedObjects.contains(registeredObjectIds.value(object))) {
        qWarning() << "Not deleting non-wrapped object" << object;
        return;
    }
    object->deleteLater();
}

void QMetaObjectPublisher::broadcastMessage(const QJsonObject &message) const
{
    if (webChannel->d_func()->transports.isEmpty()) {
        return;
    }

    for (QWebChannelAbstractTransport *transport : webChannel->d_func()->transports) {
        transport->sendMessage(message);
    }
}

void QMetaObjectPublisher::enqueueBroadcastMessage(const QJsonObject &message)
{
    if (webChannel->d_func()->transports.isEmpty()) {
        return;
    }

    for (auto *transport : webChannel->d_func()->transports) {
        auto &state = transportState[transport];
        state.queuedMessages.append(message);
    }
}

void QMetaObjectPublisher::enqueueMessage(const QJsonObject &message,
                                          QWebChannelAbstractTransport *transport)
{
    auto &state = transportState[transport];
    state.queuedMessages.append(message);
}

void QMetaObjectPublisher::sendEnqueuedPropertyUpdates(QWebChannelAbstractTransport *transport)
{
    auto found = transportState.find(transport);
    if (found != transportState.end() && found.value().clientIsIdle
        && !found.value().queuedMessages.isEmpty()) {

        // If the client is connected with an in-process transport, it can
        // happen that a message triggers a subsequent property change. In
        // that case, we need to ensure that the queued messages have already
        // been cleared; otherwise the recursive call will send everythig again.
        // Case in point: The qmlwebchannel tests fail if we don't clear the
        // queued messages before sending them out.
        // For that same reason set the client to "busy" (aka non-idle) just
        // right before sending out the messages; otherwise a potential
        // "Idle" type message will not correctly restore the Idle state.
        const auto messages = std::move(found.value().queuedMessages);
        Q_ASSERT(found.value().queuedMessages.isEmpty());
        found.value().clientIsIdle = false;

        for (const auto &message : messages) {
            transport->sendMessage(message);
        }
    }
}

void QMetaObjectPublisher::handleMessage(const QJsonObject &message, QWebChannelAbstractTransport *transport)
{
    if (!webChannel->d_func()->transports.contains(transport)) {
        qWarning() << "Refusing to handle message of unknown transport:" << transport;
        return;
    }

    if (!message.contains(KEY_TYPE)) {
        qWarning("JSON message object is missing the type property: %s", QJsonDocument(message).toJson().constData());
        return;
    }

    const MessageType type = toType(message.value(KEY_TYPE));
    if (type == TypeIdle) {
        setClientIsIdle(true, transport);
    } else if (type == TypeInit) {
        if (!message.contains(KEY_ID)) {
            qWarning("JSON message object is missing the id property: %s",
                      QJsonDocument(message).toJson().constData());
            return;
        }
        transport->sendMessage(createResponse(message.value(KEY_ID), initializeClient(transport)));
    } else if (type == TypeDebug) {
        static QTextStream out(stdout);
        out << "DEBUG: " << message.value(KEY_DATA).toString() << Qt::endl;
    } else if (message.contains(KEY_OBJECT)) {
        const QString &objectName = message.value(KEY_OBJECT).toString();
        QObject *object = registeredObjects.value(objectName);
        if (!object)
            object = wrappedObjects.value(objectName).object;

        if (!object) {
            qWarning() << "Unknown object encountered" << objectName;
            return;
        }

        if (type == TypeInvokeMethod) {
            if (!message.contains(KEY_ID)) {
                qWarning("JSON message object is missing the id property: %s",
                          QJsonDocument(message).toJson().constData());
                return;
            }

            QPointer<QMetaObjectPublisher> publisherExists(this);
            QPointer<QWebChannelAbstractTransport> transportExists(transport);
            QJsonValue method = message.value(KEY_METHOD);
            QVariant result;

            if (method.isString()) {
                result = invokeMethod(object,
                                      method.toString().toUtf8(),
                                      message.value(KEY_ARGS).toArray());
            } else {
                result = invokeMethod(object,
                                      method.toInt(-1),
                                      message.value(KEY_ARGS).toArray());
            }

            auto sendResponse = [publisherExists, transportExists, id=message.value(KEY_ID)]
                    (const QVariant &result)
            {
                if (!publisherExists || !transportExists)
                    return;

                Q_ASSERT(QThread::currentThread() == publisherExists->thread());

                const auto wrappedResult =
                        publisherExists->wrapResult(result, transportExists.get());
                transportExists->sendMessage(createResponse(id, wrappedResult));
            };

#if QT_CONFIG(future)
            if (result.canConvert<QFuture<void>>()) {
                attachContinuationToFutureInVariant(result, publisherExists.get(), sendResponse);
            } else {
                sendResponse(result);
            }
#else
            sendResponse(result);
#endif
        } else if (type == TypeConnectToSignal) {
            signalHandlerFor(object)->connectTo(object, message.value(KEY_SIGNAL).toInt(-1));
        } else if (type == TypeDisconnectFromSignal) {
            signalHandlerFor(object)->disconnectFrom(object, message.value(KEY_SIGNAL).toInt(-1));
        } else if (type == TypeSetProperty) {
            setProperty(object, message.value(KEY_PROPERTY).toInt(-1),
                        message.value(KEY_VALUE));
        }
    }
}

int QMetaObjectPublisher::propertyUpdateInterval()
{
    return propertyUpdateIntervalTime;
}

void QMetaObjectPublisher::setPropertyUpdateInterval(int ms)
{
    propertyUpdateIntervalTime = ms;
}

void QMetaObjectPublisher::setBlockUpdates(bool block)
{
    blockUpdatesStatus = block;
}

void QMetaObjectPublisher::onBlockUpdatesChanged()
{
    if (!blockUpdatesStatus) {
        startPropertyUpdateTimer();
        sendPendingPropertyUpdates();
    } else if (timer.isActive()) {
        timer.stop();
    }

    emit blockUpdatesChanged(blockUpdatesStatus);
}

bool QMetaObjectPublisher::blockUpdates() const
{
    return blockUpdatesStatus;
}

void QMetaObjectPublisher::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timer.timerId()) {
        if (propertyUpdateIntervalTime <= 0)
            timer.stop();
        sendPendingPropertyUpdates();
    } else {
        QObject::timerEvent(event);
    }
}

SignalHandler<QMetaObjectPublisher> *QMetaObjectPublisher::signalHandlerFor(const QObject *object)
{
    auto thread = object->thread();
    auto it = signalHandlers.find(thread);
    if (it == signalHandlers.end()) {
        it = signalHandlers.emplace(thread, this).first;
        it->second.moveToThread(thread);
    }
    return &it->second;
}

QT_END_NAMESPACE
