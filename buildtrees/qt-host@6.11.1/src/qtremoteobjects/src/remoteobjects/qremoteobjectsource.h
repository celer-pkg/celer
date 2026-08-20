// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QREMOTEOBJECTSOURCE_H
#define QREMOTEOBJECTSOURCE_H

#include <QtCore/qscopedpointer.h>
#include <QtRemoteObjects/qtremoteobjectglobal.h>
#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

namespace QtPrivate {

//Based on compile time checks for static connect() from qobjectdefs_impl.h
template <class ObjectType, typename Func1, typename Func2>
static inline int qtro_property_index(Func1, Func2, const char *propName)
{
    typedef QtPrivate::FunctionPointer<Func1> Type1;
    typedef QtPrivate::FunctionPointer<Func2> Type2;

    //compilation error if the arguments do not match.
    Q_STATIC_ASSERT_X(int(Type1::ArgumentCount) >= int(Type2::ArgumentCount),
                      "Argument counts are not compatible.");
    Q_STATIC_ASSERT_X((QtPrivate::CheckCompatibleArguments<typename Type1::Arguments, typename Type2::Arguments>::value),
                      "Arguments are not compatible.");
    Q_STATIC_ASSERT_X((QtPrivate::AreArgumentsCompatible<typename Type1::ReturnType, typename Type2::ReturnType>::value),
                      "Return types are not compatible.");
    return ObjectType::staticMetaObject.indexOfProperty(propName);
}

template <class ObjectType, typename Func1, typename Func2>
static inline int qtro_signal_index(Func1 func, Func2, int *count, int const **types)
{
    typedef QtPrivate::FunctionPointer<Func1> Type1;
    typedef QtPrivate::FunctionPointer<Func2> Type2;

    //compilation error if the arguments do not match.
    Q_STATIC_ASSERT_X(int(Type1::ArgumentCount) >= int(Type2::ArgumentCount),
                      "Argument counts are not compatible.");
    Q_STATIC_ASSERT_X((QtPrivate::CheckCompatibleArguments<typename Type1::Arguments, typename Type2::Arguments>::value),
                      "Arguments are not compatible.");
    Q_STATIC_ASSERT_X((QtPrivate::AreArgumentsCompatible<typename Type1::ReturnType, typename Type2::ReturnType>::value),
                      "Return types are not compatible.");
    const QMetaMethod sig = QMetaMethod::fromSignal(func);
    *count = Type2::ArgumentCount;
    *types = QtPrivate::ConnectionTypes<typename Type2::Arguments>::types();
    return sig.methodIndex();
}

template <class ObjectType, typename Func1, typename Func2>
static inline void qtro_method_test(Func1, Func2)
{
    typedef QtPrivate::FunctionPointer<Func1> Type1;
    typedef QtPrivate::FunctionPointer<Func2> Type2;

    //compilation error if the arguments do not match.
    Q_STATIC_ASSERT_X(int(Type1::ArgumentCount) >= int(Type2::ArgumentCount),
                      "Argument counts are not compatible.");
    Q_STATIC_ASSERT_X((QtPrivate::CheckCompatibleArguments<typename Type1::Arguments, typename Type2::Arguments>::value),
                      "Arguments are not compatible.");
    Q_STATIC_ASSERT_X((QtPrivate::AreArgumentsCompatible<typename Type1::ReturnType, typename Type2::ReturnType>::value),
                      "Return types are not compatible.");
}

Q_REMOTEOBJECTS_EXPORT
int qtro_method_index_impl(const QMetaObject *staticMetaObj, const char *className,
                           const char *methodName, int *count, const int **types);

template <class ObjectType, typename Func1, typename Func2>
static inline int qtro_method_index(Func1, Func2, const char *methodName, int *count, int const **types)
{
    typedef QtPrivate::FunctionPointer<Func1> Type1;
    typedef QtPrivate::FunctionPointer<Func2> Type2;

    //compilation error if the arguments do not match.
    Q_STATIC_ASSERT_X(int(Type1::ArgumentCount) >= int(Type2::ArgumentCount),
                      "Argument counts are not compatible.");
    Q_STATIC_ASSERT_X((QtPrivate::CheckCompatibleArguments<typename Type1::Arguments, typename Type2::Arguments>::value),
                      "Arguments are not compatible.");
    Q_STATIC_ASSERT_X((QtPrivate::AreArgumentsCompatible<typename Type1::ReturnType, typename Type2::ReturnType>::value),
                      "Return types are not compatible.");
    *count = Type2::ArgumentCount;
    *types = QtPrivate::ConnectionTypes<typename Type2::Arguments>::types();

    return qtro_method_index_impl(&ObjectType::staticMetaObject,
                                  ObjectType::staticMetaObject.className(), methodName, count,
                                  types);
}

template <class ObjectType>
static inline QByteArray qtro_enum_signature(const char *enumName)
{
    const auto qme = ObjectType::staticMetaObject.enumerator(ObjectType::staticMetaObject.indexOfEnumerator(enumName));
    return QByteArrayLiteral("1::2").replace("1", qme.scope()).replace("2", qme.name());
}

QByteArray qtro_classinfo_signature(const QMetaObject *metaObject);

}

// TODO ModelInfo just needs roles, and no need for SubclassInfo
class QAbstractItemModel;

struct ModelInfo
{
    QAbstractItemModel *ptr;
    QString name;
    QByteArray roles;
};

class Q_REMOTEOBJECTS_EXPORT SourceApiMap
{
protected:
    SourceApiMap() {}
public:
    virtual ~SourceApiMap();
    virtual QString name() const = 0;
    virtual QString typeName() const = 0;
    virtual QByteArray className() const { return typeName().toLatin1().append("Source"); }
    virtual int enumCount() const = 0;
    virtual int propertyCount() const = 0;
    virtual int signalCount() const = 0;
    virtual int methodCount() const = 0;
    virtual int sourceEnumIndex(int index) const = 0;
    virtual int sourcePropertyIndex(int index) const = 0;
    virtual int sourceSignalIndex(int index) const = 0;
    virtual int sourceMethodIndex(int index) const = 0;
    virtual int signalParameterCount(int index) const = 0;
    virtual int signalParameterType(int sigIndex, int paramIndex) const = 0;
    virtual const QByteArray signalSignature(int index) const = 0;
    virtual QByteArrayList signalParameterNames(int index) const = 0;
    virtual int methodParameterCount(int index) const = 0;
    virtual int methodParameterType(int methodIndex, int paramIndex) const = 0;
    virtual const QByteArray methodSignature(int index) const = 0;
    virtual QMetaMethod::MethodType methodType(int index) const = 0;
    virtual const QByteArray typeName(int index) const = 0;
    virtual QByteArrayList methodParameterNames(int index) const = 0;
    virtual int propertyIndexFromSignal(int index) const = 0;
    virtual int propertyRawIndexFromSignal(int index) const = 0;
    virtual QByteArray objectSignature() const = 0;
    virtual bool isDynamic() const { return false; }
    virtual bool isAdapterSignal(int) const { return false; }
    virtual bool isAdapterMethod(int) const { return false; }
    virtual bool isAdapterProperty(int) const { return false; }
    QList<ModelInfo> m_models;
    QList<SourceApiMap *> m_subclasses;
};

QT_END_NAMESPACE

#endif
