// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant
#ifndef QV4SCOPEDVALUE_P_H
#define QV4SCOPEDVALUE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qv4engine_p.h"
#include "qv4value_p.h"
#include "qv4property_p.h"
#include "qv4propertykey_p.h"

#ifdef V4_USE_VALGRIND
#include <valgrind/memcheck.h>
#endif

QT_BEGIN_NAMESPACE

#define SAVE_JS_STACK(ctx) Value *__jsStack = ctx->engine->jsStackTop
#define CHECK_JS_STACK(ctx) Q_ASSERT(__jsStack == ctx->engine->jsStackTop)

namespace QV4 {

struct ScopedValue;

inline bool hasExceptionOrIsInterrupted(ExecutionEngine *engine)
{
    return engine->hasException || engine->isInterrupted.loadRelaxed();
}

#define CHECK_EXCEPTION() \
    do { \
        if (hasExceptionOrIsInterrupted(scope.engine)) { \
            return QV4::Encode::undefined(); \
        } \
    } while (false)

#define RETURN_UNDEFINED() \
    return QV4::Encode::undefined()

#define RETURN_RESULT(r) \
    return QV4::Encode(r)

#define THROW_TYPE_ERROR() \
    return scope.engine->throwTypeError()

#define THROW_GENERIC_ERROR(str) \
    return scope.engine->throwError(QString::fromUtf8(str))

struct FunctionPrototype; // Used later for friending in Scope.
struct Scope {
    explicit Scope(ExecutionContext *ctx)
        : engine(ctx->engine())
        , mark(engine->jsStackTop)
    {
    }

    explicit Scope(ExecutionEngine *e)
        : engine(e)
        , mark(engine->jsStackTop)
    {
    }

    explicit Scope(const Managed *m)
        : engine(m->engine())
        , mark(engine->jsStackTop)
    {
    }

    ~Scope() {
#ifndef QT_NO_DEBUG
        Q_ASSERT(engine->jsStackTop >= mark);
//        Q_ASSERT(engine->currentContext < mark);
        memset(mark, 0, (engine->jsStackTop - mark)*sizeof(Value));
#endif
#ifdef V4_USE_VALGRIND
        VALGRIND_MAKE_MEM_UNDEFINED(mark, (engine->jsStackLimit - mark) * sizeof(Value));
#endif
        engine->jsStackTop = mark;
    }

    QML_NEARLY_ALWAYS_INLINE Value *construct(int nValues, const ReturnedValue& initialValue) const
    {
        Value *ptr = alloc(nValues);
        for (int i = 0; i < nValues; ++i)
            ptr[i].setRawValue(initialValue);
        return ptr;
    }

    QML_NEARLY_ALWAYS_INLINE Value *construct(int nValues, const Value &initialValue) const
    {
        Value *ptr = alloc(nValues);
        for (int i = 0; i < nValues; ++i)
            ptr[i] = initialValue;
        return ptr;
    }

    QML_NEARLY_ALWAYS_INLINE Value *construct(int nValues, Heap::Base* initialValue) const
    {
        Value *ptr = alloc(nValues);
        for (int i = 0; i < nValues; ++i)
            ptr[i].setM(initialValue);
        return ptr;
    }

    QML_NEARLY_ALWAYS_INLINE PropertyKey *construct(int nValues, const PropertyKey &initialValue) const
    {
        PropertyKey *ptr = reinterpret_cast<PropertyKey*>(alloc(nValues));
        for (int i = 0; i < nValues; ++i)
            ptr[i] = initialValue;
        return ptr;
    }

    QML_NEARLY_ALWAYS_INLINE Value *constructUndefined(int nValues) const
    {
        return construct(nValues, Value::undefinedValue());
    }

    bool hasException() const {
        return engine->hasException;
    }

    ExecutionEngine *engine;
    Value *mark;

private:
    QML_NEARLY_ALWAYS_INLINE Value *alloc(int nValues) const {
        return engine->jsAlloca(nValues);
    }

    Q_DISABLE_COPY(Scope)

    // The following are friended to allow access to a naked alloc.
    // General usage of alloc is dangerous, and thus generally
    // avoided, but in this case some more complex initialization
    // patterns are used that don't sensibly fit into the various
    // construct methods and further want to avoid the unnecessary
    // writes that initializing the memory with a common value such as
    // undefined would require.
    friend FunctionPrototype;
    template<typename Args>
    friend CallData *callDatafromJS(const Scope &scope, const Args *args, const FunctionObject *f);
};

struct ScopedValue
{
    ScopedValue(const ScopedValue &) = default;
    ScopedValue(ScopedValue &&) = default;

    ScopedValue(const Scope &scope)
    {
        ptr = scope.construct(1, quint64(0));
    }

    ScopedValue(const Scope &scope, const Value &v)
    {
        ptr = scope.construct(1, v);
    }

    ScopedValue(const Scope &scope, Heap::Base *o)
    {
        ptr = scope.construct(1, o);
    }

    ScopedValue(const Scope &scope, Managed *m)
    {
        ptr = scope.construct(1, m->asReturnedValue());
    }

    ScopedValue(const Scope &scope, const ReturnedValue &v)
    {
        ptr = scope.construct(1, v);
    }

    ScopedValue &operator=(const Value &v) {
        *ptr = v;
        return *this;
    }

    ScopedValue &operator=(Heap::Base *o) {
        ptr->setM(o);
        return *this;
    }

    ScopedValue &operator=(Managed *m) {
        *ptr = *m;
        return *this;
    }

    ScopedValue &operator=(const ReturnedValue &v) {
        ptr->setRawValue(v);
        return *this;
    }

    ScopedValue &operator=(const ScopedValue &other) {
        *ptr = *other.ptr;
        return *this;
    }

    Value *operator->() {
        return ptr;
    }

    const Value *operator->() const {
        return ptr;
    }

    operator Value *() { return ptr; }
    operator const Value &() const { return *ptr; }

    Value *ptr;
};


struct ScopedPropertyKey
{
    ScopedPropertyKey(const Scope &scope)
    {
        ptr = scope.construct(1, PropertyKey::invalid());
    }

    ScopedPropertyKey(const Scope &scope, const PropertyKey &v)
    {
        ptr = scope.construct(1, v);
    }

    ScopedPropertyKey &operator=(const PropertyKey &other) {
        *ptr = other;
        return *this;
    }

    PropertyKey *operator->() {
        return ptr;
    }
    operator PropertyKey() const {
        return *ptr;
    }

    bool operator==(const PropertyKey &other) const {
        return *ptr == other;
    }
    bool operator==(const ScopedPropertyKey &other) const {
        return *ptr == *other.ptr;
    }
    bool operator!=(const PropertyKey &other) const {
        return *ptr != other;
    }
    bool operator!=(const ScopedPropertyKey &other) const {
        return *ptr != *other.ptr;
    }

    PropertyKey *ptr;
};


template<typename T>
struct Scoped
{
    enum ConvertType { Convert };

    QML_NEARLY_ALWAYS_INLINE Heap::Base *underlying(const Managed *p) {
        return p ? p->m() : nullptr;
    }

    QML_NEARLY_ALWAYS_INLINE void setPointer(const Managed *p) {
        ptr->setM(underlying(p));
    }

    QML_NEARLY_ALWAYS_INLINE Scoped(const Scope &scope)
    {
        ptr = scope.constructUndefined(1);
    }

    QML_NEARLY_ALWAYS_INLINE Scoped(const Scope &scope, const Value &v)
    {
        ptr = scope.construct(1, underlying(v.as<T>()));
    }

    QML_NEARLY_ALWAYS_INLINE Scoped(const Scope &scope, Heap::Base *o)
    {
        Value v;
        v = o;
        ptr = scope.construct(1, underlying(v.as<T>()));
    }
    QML_NEARLY_ALWAYS_INLINE Scoped(const Scope &scope, const ScopedValue &v)
    {
        ptr = scope.construct(1, underlying(v.ptr->as<T>()));
    }

    QML_NEARLY_ALWAYS_INLINE Scoped(const Scope &scope, const Value &v, ConvertType)
    {
        ptr = scope.construct(1, value_convert<T>(scope.engine, v));
    }

    QML_NEARLY_ALWAYS_INLINE Scoped(const Scope &scope, const Value *v)
    {
        ptr = scope.construct(1, underlying(v ? v->as<T>() : nullptr));
    }

    QML_NEARLY_ALWAYS_INLINE Scoped(const Scope &scope, T *t)
    {
        ptr = scope.construct(1, underlying(t));
    }

    QML_NEARLY_ALWAYS_INLINE Scoped(const Scope &scope, const T *t)
    {
        ptr = scope.construct(1, underlying(t));
    }

    QML_NEARLY_ALWAYS_INLINE Scoped(const Scope &scope, typename T::Data *t)
    {
        ptr = scope.construct(1, t);
    }

    QML_NEARLY_ALWAYS_INLINE Scoped(const Scope &scope, const ReturnedValue &v)
    {
        ptr = scope.construct(1, underlying(QV4::Value::fromReturnedValue(v).as<T>()));
    }

    QML_NEARLY_ALWAYS_INLINE Scoped(const Scope &scope, const ReturnedValue &v, ConvertType)
    {
        ptr = scope.construct(1, value_convert<T>(scope.engine, QV4::Value::fromReturnedValue(v)));
    }

    Scoped<T> &operator=(Heap::Base *o) {
        setPointer(Value::fromHeapObject(o).as<T>());
        return *this;
    }
    Scoped<T> &operator=(typename T::Data *t) {
        *ptr = t;
        return *this;
    }
    Scoped<T> &operator=(const Value &v) {
        setPointer(v.as<T>());
        return *this;
    }
    Scoped<T> &operator=(Value *v) {
        setPointer(v ? v->as<T>() : nullptr);
        return *this;
    }

    Scoped<T> &operator=(const ReturnedValue &v) {
        setPointer(QV4::Value::fromReturnedValue(v).as<T>());
        return *this;
    }

    Scoped<T> &operator=(T *t) {
        setPointer(t);
        return *this;
    }

    operator T *() {
        return static_cast<T *>(ptr->managed());
    }
    operator const Value &() const {
        return *ptr;
    }

    T *operator->() {
        return getPointer();
    }

    const T *operator->() const {
        return getPointer();
    }

    explicit operator bool() const {
        return ptr->m();
    }

    T *getPointer() {
        return reinterpret_cast<T *>(ptr);
    }

    const T *getPointer() const {
        return reinterpret_cast<T *>(ptr);
    }

    Value *getRef() {
        return ptr;
    }

    QML_NEARLY_ALWAYS_INLINE ReturnedValue asReturnedValue() const {
        return ptr->rawValue();
    }

    Value *ptr;
};

inline Value &Value::operator =(const ScopedValue &v)
{
    _val = v.ptr->rawValue();
    return *this;
}

template<typename T>
inline Value &Value::operator=(const Scoped<T> &t)
{
    _val = t.ptr->rawValue();
    return *this;
}

struct ScopedProperty
{
    ScopedProperty(Scope &scope)
    {
        property = reinterpret_cast<Property*>(scope.constructUndefined(int(sizeof(Property) / sizeof(Value))));
    }

    Property *operator->() { return property; }
    operator const Property *() const { return property; }
    operator Property *() { return property; }

    Property *property;
};

}

QT_END_NAMESPACE

#endif
