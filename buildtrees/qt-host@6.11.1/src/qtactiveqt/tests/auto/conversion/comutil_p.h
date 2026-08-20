// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef COMUTIL_P_H
#define COMUTIL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QtGlobal>
#include <QtAxBase/private/qbstr_p.h>
#include <comdef.h>
#include <type_traits>
#include <oleauto.h>

template<typename T>
constexpr VARTYPE ValueType()
{
    using ValueType = std::remove_cv_t<std::remove_pointer_t<T>>;

    constexpr VARTYPE maybeByref = std::is_pointer_v<T> ? VT_BYREF : VT_EMPTY;
    if constexpr (std::is_same_v<ValueType, bool>)
        return VT_BOOL | maybeByref;
    else if constexpr (std::is_same_v<ValueType, char>)
        return VT_I1 | maybeByref;
    else if constexpr (std::is_same_v<ValueType, unsigned char>)
        return VT_UI1 | maybeByref;
    else if constexpr (std::is_same_v<ValueType, short>)
        return VT_I2 | maybeByref;
    else if constexpr (std::is_same_v<ValueType, unsigned short>)
        return VT_UI2 | maybeByref;
    else if constexpr (std::is_same_v<ValueType, int>)
        return VT_I4 | maybeByref;
    else if constexpr (std::is_same_v<ValueType, unsigned int>)
        return VT_UI4 | maybeByref;
    else if constexpr (std::is_same_v<ValueType, long long>)
        return VT_I8 | maybeByref;
    else if constexpr (std::is_same_v<ValueType, unsigned long long>)
        return VT_UI8 | maybeByref;
    else if constexpr (std::is_same_v<ValueType, float>)
        return VT_R4 | maybeByref;
    else if constexpr (std::is_same_v<ValueType, double>)
        return VT_R8 | maybeByref;
    else if constexpr (std::is_same_v<const ValueType *, const wchar_t *>)
        return VT_BSTR;
    else if constexpr (std::is_base_of_v<VARIANT, ValueType>)
        return VT_VARIANT;
    else if constexpr (std::is_base_of_v<IDispatch, ValueType>)
        return VT_DISPATCH;
    else if constexpr (std::is_base_of_v<IUnknown, ValueType>)
        return VT_UNKNOWN;
    else
        return VT_EMPTY;
};

template<typename T>
constexpr auto ValueField()
{
    using Type = std::remove_const_t<T>;
    if constexpr (std::is_same_v<Type, bool>)
        return &VARIANT::boolVal;
    else if constexpr (std::is_same_v<Type, bool *>)
        return &VARIANT::pboolVal;
    else if constexpr (std::is_same_v<Type, char>)
        return &VARIANT::cVal;
    else if constexpr (std::is_same_v<Type, char *>)
        return &VARIANT::pcVal;
    else if constexpr (std::is_same_v<Type, unsigned char>)
        return &VARIANT::bVal;
    else if constexpr (std::is_same_v<Type, unsigned char *>)
        return &VARIANT::pbVal;
    else if constexpr (std::is_same_v<Type, short>)
        return &VARIANT::iVal;
    else if constexpr (std::is_same_v<Type, short *>)
        return &VARIANT::piVal;
    else if constexpr (std::is_same_v<Type, unsigned short>)
        return &VARIANT::uiVal;
    else if constexpr (std::is_same_v<Type, unsigned short *>)
        return &VARIANT::puiVal;
    else if constexpr (std::is_same_v<Type, int>)
        return &VARIANT::intVal;
    else if constexpr (std::is_same_v<Type, int *>)
        return &VARIANT::pintVal;
    else if constexpr (std::is_same_v<Type, unsigned int>)
        return &VARIANT::uintVal;
    else if constexpr (std::is_same_v<Type, unsigned int *>)
        return &VARIANT::puintVal;
    else if constexpr (std::is_same_v<Type, long long>)
        return &VARIANT::llVal;
    else if constexpr (std::is_same_v<Type, long long *>)
        return &VARIANT::pllVal;
    else if constexpr (std::is_same_v<Type, unsigned long long>)
        return &VARIANT::ullVal;
    else if constexpr (std::is_same_v<Type, unsigned long long *>)
        return &VARIANT::pullVal;
    else if constexpr (std::is_same_v<Type, float>)
        return &VARIANT::fltVal;
    else if constexpr (std::is_same_v<Type, float *>)
        return &VARIANT::pfltVal;
    else if constexpr (std::is_same_v<Type, double>)
        return &VARIANT::dblVal;
    else if constexpr (std::is_same_v<Type, double *>)
        return &VARIANT::pdblVal;
    else if constexpr (std::is_same_v<T, const wchar_t *>)
        return &VARIANT::bstrVal;
    else if constexpr (std::is_base_of_v<IDispatch, std::remove_pointer_t<Type>>)
        return &VARIANT::pdispVal;
    else if constexpr (std::is_base_of_v<IUnknown, std::remove_pointer_t<Type>>)
        return &VARIANT::punkVal;
}

class ComVariant : public tagVARIANT
{
public:
    ComVariant() noexcept : VARIANT{} { ::VariantInit(this); }

    ~ComVariant()
    {
        const HRESULT hr = ::VariantClear(this);
        Q_ASSERT(hr == S_OK);
        Q_UNUSED(hr)
    }

    ComVariant(const ComVariant &src) : ComVariant() { copy(&src); }
    ComVariant(const VARIANT &src) noexcept : ComVariant() { copy(&src); }
    ComVariant(const QBStr &src) noexcept : ComVariant() { copy(src.bstr()); }

    template<typename T>
    ComVariant(const T &value) : ComVariant()
    {
        assign(value);
    }

    ComVariant &operator=(const ComVariant &rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        clear();
        copy(&rhs);

        return *this;
    }

    template<typename T>
    ComVariant &operator=(const T &rhs)
    {
        assign(rhs);
        return *this;
    }

    void clear()
    {
        const HRESULT hr = ::VariantClear(this);

        Q_ASSERT(hr == S_OK);
        Q_UNUSED(hr)
    }

    void copy(const VARIANT *src)
    {
        vt = VT_EMPTY;
        const HRESULT hr = ::VariantCopy(this, const_cast<VARIANT *>(src));

        Q_ASSERT(hr == S_OK);
        Q_UNUSED(hr)
    }

    void copy(const BSTR &src)
    {
        vt = VT_EMPTY;
        if (!src)
            return;

        vt = VT_BSTR;
        bstrVal = ::SysAllocStringByteLen(reinterpret_cast<const char *>(src),
                                          ::SysStringByteLen(src));

        Q_ASSERT(bstrVal);
    }

    bool operator==(const ComVariant &rhs) const
    {
        auto *lhs = const_cast<tagVARIANT *>(static_cast<const tagVARIANT *>(this));
        auto *other = const_cast<tagVARIANT *>(static_cast<const tagVARIANT *>(&rhs));
        return compare(lhs, other, LOCALE_USER_DEFAULT, 0) == static_cast<HRESULT>(VARCMP_EQ);
    }

private:
    template<typename T>
    void assign(const T &value)
    {
        constexpr VARTYPE valueType = ValueType<T>();
        static_assert(valueType != VT_EMPTY, "Invalid type for ComVariant");

        vt = valueType;

        constexpr auto VARIANT::*field = ValueField<T>();
        if constexpr (valueType == VT_BSTR)
            this->*field = QBStr{ value }.release();
        else
            this->*field = value;

        if constexpr (valueType == VT_UNKNOWN || valueType == VT_DISPATCH)
            value->AddRef();
    }

    static HRESULT compare(VARIANT *lhs, VARIANT *rhs, LCID lcid, ULONG flags)
    {
        // Workaround missing support for VT_I1, VT_UI2, VT_UI4 and VT_UI8 in VarCmp
        switch (lhs->vt) {
        case VT_I1:
            if (lhs->cVal == rhs->cVal)
                return VARCMP_EQ;
            return lhs->cVal > rhs->cVal ? VARCMP_GT : VARCMP_LT;
        case VT_UI2:
            if (lhs->uiVal == rhs->uiVal)
                return VARCMP_EQ;
            return lhs->uiVal > rhs->uiVal ? VARCMP_GT : VARCMP_LT;
        case VT_UI4:
            if (lhs->uintVal == rhs->uintVal)
                return VARCMP_EQ;
            return lhs->uintVal > rhs->uintVal ? VARCMP_GT : VARCMP_LT;
        case VT_UI8:
            if (lhs->ullVal == rhs->ullVal)
                return VARCMP_EQ;
            return lhs->ullVal > rhs->ullVal ? VARCMP_GT : VARCMP_LT;
        }
        return ::VarCmp(lhs, rhs, lcid, flags);
    }
};

struct SafeArray
{
    explicit SafeArray(SAFEARRAY *array) noexcept
    {
        HRESULT hr = SafeArrayCopy(array, &m_array);
        Q_ASSERT(hr == S_OK);
        hr = SafeArrayLock(m_array);
        Q_ASSERT(hr == S_OK);
    }

    ~SafeArray() noexcept
    {
        if (!m_array)
            return;

        HRESULT hr = SafeArrayUnlock(m_array);
        Q_ASSERT(hr == S_OK);
        hr = SafeArrayDestroy(m_array);
        Q_ASSERT(hr == S_OK);
    }

    std::optional<VARTYPE> itemType() const noexcept
    {
        VARTYPE vt{};
        if (SafeArrayGetVartype(m_array, &vt) == S_OK)
            return vt;

        return {};
    }

    template <typename T>
    std::optional<QSpan<T>> data() const noexcept
    {
        constexpr VARTYPE vt = ValueType<T>();
        static_assert(vt != VT_EMPTY, "Unsupported type");

        if (vt != itemType())
            return {};

        auto data = static_cast<T *>(m_array->pvData);
        return QSpan{ data, m_array->rgsabound->cElements };
    }

    SAFEARRAY *m_array = nullptr;
};

#endif
