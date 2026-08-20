// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtProtobuf/qprotobufregistration.h>

#include <QtProtobuf/private/qtprotobuf-config_p.h>
#include <QtProtobuf/private/qprotobufregistration_p.h>
#include <QtProtobuf/qtprotobuftypes.h>

#include <QtCore/qmutex.h>
#include <QtCore/qmetatype.h>
#if !QT_CONFIG(protobuf_unsafe_registry)
#   include <QtCore/qreadwritelock.h>
#endif

#include <mutex>

QT_BEGIN_NAMESPACE

/*!
    \relates QtProtobuf
    \fn template<typename T, QtProtobuf::if_protobuf_message<T> = true> inline void qRegisterProtobufType()

    Registers a Protobuf type \e T.
    This function is normally called by generated code.
*/

/*!
    \relates QtProtobuf
    \fn template<typename K, typename V, QtProtobuf::if_protobuf_map<K, V> = true> inline void qRegisterProtobufMapType();

    Registers a Protobuf map type \c K and \c V.
    \c V must be a QProtobufMessage.
    This function is normally called by generated code.
*/

/*!
    \relates QtProtobuf
    \fn template <typename T, typename std::enable_if_t<std::is_enum<T>::value, bool> = true> inline void qRegisterProtobufEnumType();

    Registers serializers for enumeration type \c T in QtProtobuf global
    serializers registry.

    This function is normally called by generated code.
*/

namespace {
std::vector<QtProtobuf::RegisterFunction> &registerFunctions()
{
    // no need for implicit sharing etc, so stick with std::vector
    static std::vector<QtProtobuf::RegisterFunction> registrationList;
    return registrationList;
}

template<typename T>
void registerBasicConverters()
{
    QMetaType::registerConverter<int32_t, T>(T::fromType);
    QMetaType::registerConverter<T, int32_t>(T::toType);
    QMetaType::registerConverter<int64_t, T>(T::fromType);
    QMetaType::registerConverter<T, int64_t>(T::toType);
    QMetaType::registerConverter<uint32_t, T>(T::fromType);
    QMetaType::registerConverter<T, uint32_t>(T::toType);
    QMetaType::registerConverter<uint64_t, T>(T::fromType);
    QMetaType::registerConverter<T, uint64_t>(T::toType);
    if constexpr (!std::is_same_v<long long, int64_t>) {
        QMetaType::registerConverter<long long, T>(T::fromType);
        QMetaType::registerConverter<T, long long>(T::toType);
        QMetaType::registerConverter<unsigned long long, T>(T::fromType);
        QMetaType::registerConverter<T, unsigned long long>(T::toType);
    }
    QMetaType::registerConverter<double, T>(T::fromType);
    QMetaType::registerConverter<T, double>(T::toType);
    QMetaType::registerConverter<T, QString>(T::toString);
}

static void qRegisterBaseTypes()
{
    [[maybe_unused]] // definitely unused
    static bool registered = [] {
        qRegisterMetaType<QtProtobuf::int32>();
        qRegisterMetaType<QtProtobuf::int64>();
        qRegisterMetaType<QtProtobuf::uint32>();
        qRegisterMetaType<QtProtobuf::uint64>();
        qRegisterMetaType<QtProtobuf::sint32>();
        qRegisterMetaType<QtProtobuf::sint64>();
        qRegisterMetaType<QtProtobuf::fixed32>();
        qRegisterMetaType<QtProtobuf::fixed64>();
        qRegisterMetaType<QtProtobuf::sfixed32>();
        qRegisterMetaType<QtProtobuf::sfixed64>();
        qRegisterMetaType<QtProtobuf::boolean>();

        qRegisterMetaType<QtProtobuf::int32List>();
        qRegisterMetaType<QtProtobuf::int64List>();
        qRegisterMetaType<QtProtobuf::uint32List>();
        qRegisterMetaType<QtProtobuf::uint64List>();
        qRegisterMetaType<QtProtobuf::sint32List>();
        qRegisterMetaType<QtProtobuf::sint64List>();
        qRegisterMetaType<QtProtobuf::fixed32List>();
        qRegisterMetaType<QtProtobuf::fixed64List>();
        qRegisterMetaType<QtProtobuf::sfixed32List>();
        qRegisterMetaType<QtProtobuf::sfixed64List>();

        qRegisterMetaType<QtProtobuf::doubleList>();
        qRegisterMetaType<QtProtobuf::floatList>();
        qRegisterMetaType<QtProtobuf::boolList>();

        registerBasicConverters<QtProtobuf::int32>();
        registerBasicConverters<QtProtobuf::int64>();
        registerBasicConverters<QtProtobuf::sfixed32>();
        registerBasicConverters<QtProtobuf::sfixed64>();
        registerBasicConverters<QtProtobuf::fixed32>();
        registerBasicConverters<QtProtobuf::fixed64>();
        return true;
    }();
}

/*
    \internal
    \brief The HandlersRegistry is a container to store mapping between metatype
    identifier and serialization handlers.
*/
struct HandlersRegistry
{
    void registerHandler(QMetaType type, QtProtobufPrivate::Serializer serializer,
                         QtProtobufPrivate::Deserializer deserializer)
    {
#if !QT_CONFIG(protobuf_unsafe_registry)
        QWriteLocker locker(&m_lock);
#endif
        m_registry[type] = { serializer, deserializer };
    }

    QtProtobufPrivate::SerializationHandler findHandler(QMetaType type)
    {
#if !QT_CONFIG(protobuf_unsafe_registry)
        QReadLocker locker(&m_lock);
#endif
        auto it = m_registry.constFind(type);
        if (it != m_registry.constEnd())
            return it.value();
        return {};
    }

private:
# if !QT_CONFIG(protobuf_unsafe_registry)
    QReadWriteLock m_lock;
# endif
    QHash<QMetaType, QtProtobufPrivate::SerializationHandler> m_registry;
};
Q_GLOBAL_STATIC(HandlersRegistry, handlersRegistry)


Q_CONSTINIT QBasicMutex registerMutex;
}

void QtProtobufPrivate::registerHandler(QMetaType type, Serializer serializer,
                                        Deserializer deserializer)
{
    handlersRegistry->registerHandler(type, serializer, deserializer);
}

QtProtobufPrivate::SerializationHandler QtProtobufPrivate::findHandler(QMetaType type)
{
    if (!handlersRegistry.exists())
        return {};
    return handlersRegistry->findHandler(type);
}

namespace QtProtobuf {

ProtoTypeRegistrar::ProtoTypeRegistrar(QtProtobuf::RegisterFunction initializer)
{
    std::scoped_lock lock(registerMutex);
    registerFunctions().push_back(initializer);
}

}

/*!
    \relates QtProtobuf
    Calling this function registers all, currently known, protobuf types with
    the serializer registry.

    \note Only since Qt 6.6.3 version you don't have to call this function manually,
    as it is called automatically. For earlier versions it's better to call it
    before serialization/deserialization attempt.
*/
void qRegisterProtobufTypes()
{
    qRegisterBaseTypes();

    std::scoped_lock lock(registerMutex);
    std::vector<QtProtobuf::RegisterFunction> registrationList;
    registrationList.swap(registerFunctions());

    for (QtProtobuf::RegisterFunction registerFunc : registrationList)
        registerFunc();
}

/*!
    \class template <typename T, QtProtobuf::if_protobuf_message<T> = true> class QtProtobufPrivate::ListIterator
    \internal
*/

/*!
    \class template <typename K, typename V, QtProtobuf::if_protobuf_map<K, V> = true> class QtProtobufPrivate::MapIterator
    \internal
*/

/*!
    \struct QtProtobufPrivate::SerializationHandler
    \internal
    \brief SerializationHandlers contains set of objects that are required for
        class serialization/deserialization.
*/

QT_END_NAMESPACE
