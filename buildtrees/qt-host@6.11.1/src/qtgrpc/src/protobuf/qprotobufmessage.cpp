// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtProtobuf/qprotobufmessage.h>

#include <QtProtobuf/private/qprotobufmessage_p.h>
#include <QtProtobuf/private/qprotobufpropertyorderingbuilder_p.h>
#include <QtProtobuf/qabstractprotobufserializer.h>
#include <QtProtobuf/qprotobufpropertyordering.h>

#include <QtCore/qassert.h>
#include <QtCore/qmetaobject.h>

#include <QtCore/private/qmetaobjectbuilder_p.h>

#include <string>

QT_BEGIN_NAMESPACE

static std::string nullTerminate(QLatin1StringView l1) noexcept
{
    return l1.isNull() ? std::string{} : std::string{l1.data(), size_t(l1.size())};
}

/*!
    \since 6.11
    \fn template <class To> To qprotobufmessage_cast(QProtobufMessage *from)
    \fn template <class To> To qprotobufmessage_cast(const QProtobufMessage *from)
    \relates QProtobufMessage

    Returns the given \a from object cast to pointer type \c To if the object is
    of type \c To (or a subclass); otherwise returns \nullptr. If \a from is
    \nullptr, this function also returns \nullptr.

    The template argument \c To must be a pointer to a class that inherits from
    QProtobufMessage and is declared with the \l Q_PROTOBUF_OBJECT macro.

    Example:
    \code
    QProtobufMessage *message = new myproject::MySpecificMessage();

    // Cast to its own type (success)
    auto *specific = qprotobufmessage_cast<myproject::MySpecificMessage *>(message);
    // specific is a valid pointer

    // Cast to an unrelated type (failure)
    auto *another = qprotobufmessage_cast<myproject::AnotherMessage *>(message);
    // another is nullptr
    \endcode

    This function behaves similarly to the standard C++ \c dynamic_cast(), with
    the advantages that it doesn't require RTTI support and works across
    dynamic library boundaries.

    \warning If the type pointed to by \c To is not declared with the
    Q_PROTOBUF_OBJECT macro, this function's return value is undefined.
*/

/*!
    \class QProtobufMessage
    \inmodule QtProtobuf

    \brief Base class for all protobuf messages.

    Provides access to the properties of a message, using setProperty()
    and property(), without depending on what the message is.
*/

/*!
    \internal
    Used from generated classes to construct the QProtobufMessage base class.
    Internally the \a metaObject is used to query QMetaProperty
*/
QProtobufMessage::QProtobufMessage(const QMetaObject *metaObject,
                                   const QtProtobufPrivate::QProtobufPropertyOrdering *ordering)
    : d_ptr(new QProtobufMessagePrivate(metaObject, ordering))
{
}

/*!
    \internal
    Allows constructing QProtobufMessage using the private message implementation from the
    derived class.
*/
QProtobufMessage::QProtobufMessage(QProtobufMessagePrivate &dd) : d_ptr(&dd)
{
}

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QProtobufMessagePrivate)

QProtobufMessagePrivate::QProtobufMessagePrivate(const QMetaObject *metaObject,
                                                 const QtProtobufPrivate::QProtobufPropertyOrdering
                                                     *ordering)
    : metaObject(metaObject), ordering(ordering)
{
}

QProtobufMessagePrivate::~QProtobufMessagePrivate()
    = default;

/*!
    \internal
*/
int QProtobufMessagePrivate::propertyIndex(QAnyStringView propertyName) const
{
    return propertyName.visit([this](auto propertyName) {
        if constexpr (std::is_same_v<QStringView, decltype(propertyName)>) {
            return metaObject->indexOfProperty(propertyName.toLatin1().constData());
        } else if constexpr (std::is_same_v<QUtf8StringView, decltype(propertyName)>) {
            return metaObject->indexOfProperty(propertyName.toString().toLatin1().constData());
        } else if constexpr (std::is_same_v<QLatin1StringView, decltype(propertyName)>) {
            return metaObject->indexOfProperty(nullTerminate(propertyName).data());
        }
        return -1;
    });
}

void QProtobufMessagePrivate::storeUnknownEntry(QProtobufMessage *message,
                                                QByteArrayView entry, int fieldNumber)
{
    Q_ASSERT(message);
    message->d_ptr.detach();
    message->d_ptr->unknownEntries[fieldNumber].append(entry.toByteArray());
}

std::optional<QMetaProperty> QProtobufMessagePrivate::metaProperty(QAnyStringView name) const
{
    const int index = propertyIndex(name);
    const QMetaProperty property = metaObject->property(index);
    if (property.isValid())
        return property;
    return std::nullopt;
}

std::optional<QMetaProperty>
QProtobufMessagePrivate::metaProperty(QtProtobufPrivate::QProtobufFieldInfo info) const
{
    const int propertyIndex = info.propertyIndex() + metaObject->propertyOffset();
    const QMetaProperty metaProperty = metaObject->property(propertyIndex);
    if (metaProperty.isValid())
        return metaProperty;
    return std::nullopt;
}

/*!
    Set the property \a propertyName to the value stored in \a value.

    If the \a propertyName isn't a known fields, then the value is ignored, and
    the function returns \c false.

    Returns \c false if it fails to store the \a value on the property.
    Otherwise \c{true}.
*/
bool QProtobufMessage::setProperty(QAnyStringView propertyName, const QVariant &value)
{
    Q_D(QProtobufMessage);

    if (auto mp = d->metaProperty(propertyName))
        return mp->writeOnGadget(this, value);

    return false;
}

/*!
    \overload
    \since 6.6
*/
bool QProtobufMessage::setProperty(QAnyStringView propertyName, QVariant &&value)
{
    Q_D(QProtobufMessage);

    if (auto mp = d->metaProperty(propertyName))
        return mp->writeOnGadget(this, std::move(value));

    return false;
}

/*!
    Get the value of the property \a propertyName.

    If the \a propertyName isn't known then the returned QVariant is invalid.
*/
QVariant QProtobufMessage::property(QAnyStringView propertyName) const
{
    Q_D(const QProtobufMessage);

    if (const auto mp = d->metaProperty(propertyName))
        return mp->readOnGadget(this);
    return {};
}

/*!
    \internal
*/
QProtobufMessage::QProtobufMessage(const QProtobufMessage &other)
    = default;

/*!
    \internal
*/
QProtobufMessage &QProtobufMessage::operator=(const QProtobufMessage &other)
    = default;


/*!
    \internal
*/
QProtobufMessage::~QProtobufMessage()
    = default;

/*!
    \since 6.8
    Returns the pointer to the property ordering of the derived protobuf message.
*/
const QtProtobufPrivate::QProtobufPropertyOrdering *QProtobufMessage::propertyOrdering() const
{
    Q_D(const QProtobufMessage);
    return d->ordering;
}

/*!
    \internal
*/
bool comparesEqual(const QProtobufMessage &lhs, const QProtobufMessage &rhs) noexcept
{
    if (lhs.d_ptr == rhs.d_ptr)
        return true;
    return lhs.d_func()->unknownEntries == rhs.d_func()->unknownEntries;
}

/*!
    \internal
    \since 6.11
    Returns a pointer to the meta-object for this message instance. The
    meta-object provides run-time type information for dynamic operations.

    \sa qprotobufmessage_cast()
*/
const QMetaObject *QProtobufMessage::metaObject() const
{
    Q_D(const QProtobufMessage);
    return d->metaObject;
}

namespace QtProtobufPrivate {
/*!
    \internal
*/
extern QProtobufMessagePointer constructMessageByName(const QString &messageType);
}

/*!
    Constructs QProtobufMessage using \a messageType.
    Returns a pointer to the constructed QProtobufMessage.

    This function attempts to create a message with a type that matches \a messageType.
    If \a messageType is unknown, the function returns \nullptr. If the message
    is not found in the registry, the function returns \nullptr.
    The function caller is given ownership of the constructed message.
*/
QProtobufMessagePointer QProtobufMessage::constructByName(const QString &messageType)
{
    return QtProtobufPrivate::constructMessageByName(messageType);
}

/*!
    \typedef QProtobufMessagePointer
    \relates QProtobufMessage
    \inmodule QtProtobuf

    Synonym for std::unique_ptr<QProtobufMessage, QProtobufMessageDeleter>.
    Use this to manage the lifetime of dynamically allocated QProtobufMessages,
    such as those created by calling QProtobufMessage::constructByName.
*/

/*!
    \class QProtobufMessageDeleter
    \inmodule QtProtobuf
    \brief Calls the destructor of the child class of a QProtobufMessage.

    This class calls the destructor of a protobuf message using the meta-type
    system. It is intended to be used with smart pointers, such as std::unique_ptr.

    \sa QProtobufMessagePointer
*/

/*!
    Destroys the message pointed to by \a ptr.
    This is intended for use with smart pointers.

    \sa QProtobufMessagePointer
*/
void QProtobufMessageDeleter::operator()(QProtobufMessage *ptr) const noexcept
{
    if (!ptr)
        return;
    const QMetaObject *mobj = ptr->d_ptr->metaObject;
    QMetaType type = mobj->metaType();
    type.destroy(ptr);
}

/*!
    Returns the field numbers that were not known to QtProtobuf during
    deserialization.
    \since 6.7
 */
QList<qint32> QProtobufMessage::unknownFieldNumbers() const
{
    Q_D(const QProtobufMessage);
    return d->unknownEntries.keys();
}

/*!
    Returns the unknown \a field values sorted as they were received from the
    wire.
    \since 6.7
*/
QList<QByteArray> QProtobufMessage::unknownFieldData(qint32 field) const
{
    Q_D(const QProtobufMessage);
    return d->unknownEntries.value(field);
}

/*!
    \since 6.8
    Serializes this protobuf message into a QByteArray using \a serializer.

    \sa deserialize()
*/
QByteArray QProtobufMessage::serialize(QAbstractProtobufSerializer *serializer) const
{
    return serializer->serialize(this);
}

/*!
    \since 6.8
    Deserializes this protobuf message from a QByteArray \a data using
    \a serializer.
    Returns \c true if deserialization was successful, otherwise \c false.

    \sa serialize()
*/
bool QProtobufMessage::deserialize(QAbstractProtobufSerializer *serializer, QByteArrayView data)
{
    return serializer->deserialize(this, data);
}

static bool isProtobufMessage(QMetaType type)
{
    if (const auto *mo = type.metaObject(); mo && mo->inherits(&QProtobufMessage::staticMetaObject))
        return true;
    return false;
}

using StaticMetaCallFn = void (*)(QObject *, QMetaObject::Call, int, void **);
QMetaObject *buildMetaObject(QMetaType key, QMetaType value, StaticMetaCallFn metaCall)
{
    using namespace Qt::StringLiterals;
    QMetaObjectBuilder builder;
    builder.addProperty("key", key.name(), key);
    if (isProtobufMessage(key)) {
        auto propBuilder = builder.addProperty("has_key", "bool", QMetaType(QMetaType::Bool));
        propBuilder.setWritable(false);
    }
    builder.addProperty("value", value.name(), value);
    if (isProtobufMessage(value)) {
        auto propBuilder = builder.addProperty("has_value", "bool", QMetaType(QMetaType::Bool));
        propBuilder.setWritable(false);
    }
    builder.setClassName("QProtobufMapEntry<"_ba + key.name() + ", " + value.name() + '>');
    builder.setSuperClass(&QProtobufMapEntryBase::staticMetaObject);
    builder.setStaticMetacallFunction(metaCall);
    builder.setFlags(MetaObjectFlag::PropertyAccessInStaticMetaCall);
    return builder.toMetaObject();
}

QtProtobufPrivate::FieldFlags getFlagForType(QMetaType type)
{
    using FieldFlag = QtProtobufPrivate::FieldFlag;
    QtProtobufPrivate::FieldFlags flag = {};

    if (isProtobufMessage(type))
        flag |= FieldFlag::Message | FieldFlag::ExplicitPresence;
    if (type.flags() & QMetaType::IsEnumeration)
        flag |= FieldFlag::Enum;
    if (QByteArrayView(type.name()).startsWith("QList<")) // Surely there's a better way
        flag |= FieldFlag::Repeated;
    if (QByteArrayView(type.name()).startsWith("QHash<")) // Surely there's a better way
        flag |= FieldFlag::Map;

    flag |= FieldFlag::Optional; // Hardcoded for MapEntry uses...

    // Need to get this info::
    // NonPacked = 0x1,
    // Oneof = 0x02,
    // Optional = 0x04,
    // Repeated = 0x40,
    // Map = 0x80,

    return flag;
}

QtProtobufPrivate::QProtobufPropertyOrdering::Data *buildMapEntryOrdering(QMetaType key,
                                                                          QMetaType value)
{
    using namespace QtProtobufPrivate;
    QProtobufPropertyOrderingBuilder builder("MapEntry");

    const auto keyFlags = getFlagForType(key);
    builder.addV0Field("key", 1, 0, keyFlags);

    const uint valueIndex = keyFlags.testFlag(FieldFlag::ExplicitPresence) ? 2 : 1;
    builder.addV0Field("value", 2, valueIndex, getFlagForType(value));

    return builder.build();
}

class QProtobufMapEntryBasePrivate : public QProtobufMessagePrivate
{
    Q_DISABLE_COPY_MOVE(QProtobufMapEntryBasePrivate)
public:
    QProtobufMapEntryBasePrivate(QMetaType key, QMetaType value, StaticMetaCallFn metaCall);
    ~QProtobufMapEntryBasePrivate() override;

    QtProtobufPrivate::QProtobufPropertyOrdering::Data *data = nullptr;
    QtProtobufPrivate::QProtobufPropertyOrdering mutableOrdering{};
    QMetaObject *mutableMetaObject = nullptr;
};

QProtobufMapEntryBase::QProtobufMapEntryBase(QMetaType key, QMetaType value,
                                             StaticMetaCallFn metaCall)
    : QProtobufMessage(*new QProtobufMapEntryBasePrivate(key, value, metaCall))
{
}

QProtobufMapEntryBasePrivate::QProtobufMapEntryBasePrivate(QMetaType key, QMetaType value,
                                                           StaticMetaCallFn metaCall)
    : QProtobufMessagePrivate(),
      data(buildMapEntryOrdering(key, value)),
      mutableMetaObject(buildMetaObject(key, value, metaCall))
{
    mutableOrdering.data = data;
    QProtobufMessagePrivate::ordering = &mutableOrdering;
    QProtobufMessagePrivate::metaObject = mutableMetaObject;
}

QProtobufMapEntryBase::~QProtobufMapEntryBase()
    = default;

QProtobufMapEntryBasePrivate::~QProtobufMapEntryBasePrivate()
{
    data->~Data();
    free(data);
    free(mutableMetaObject);
}

QT_END_NAMESPACE

#include "moc_qprotobufmessage.cpp"
