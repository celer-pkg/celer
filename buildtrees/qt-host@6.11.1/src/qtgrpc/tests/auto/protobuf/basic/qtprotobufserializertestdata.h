// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QStringLiteral>
#include <QtCore/qsize.h>
#include <limits>
#include <optional>

namespace qtprotobufnamespace::tests {

using namespace Qt::Literals::StringLiterals;

#define INT_MIN_SDATA(type, size, data) \
  std::numeric_limits<type>::min(), size, data, QString("%1_min").arg(#type)
#define INT_MAX_SDATA(type, size, data) \
  std::numeric_limits<type>::max(), size, data, QString("%1_max").arg(#type)
#define BELOW_INT_MAX_SDATA(type, size, data) \
  std::numeric_limits<type>::min() - 1, size, data, QString("below_%1_min").arg(#type)
#define OVER_INT_MAX_SDATA(type, size, data) \
  std::numeric_limits<type>::max() + 1, size, data, QString("over_%1_max").arg(#type)

constexpr qsizetype Fixed32IntSize = 5;
constexpr qsizetype Fixed64IntSize = 9;

enum class IntTypes : uint8_t {
    Int = 0,
    UInt,
    SInt,
    Fixed,
    SFixed,
};

template <typename T>
struct SerializeData
{
    T value;
    qsizetype size;
    QByteArray hexData;
    std::optional<QString> name = std::nullopt;
};

template <typename T, IntTypes type>
QList<SerializeData<T>> getCommonIntValues()
{
    QList<SerializeData<T>> data{ { (T)0, 0, ""_ba, u"empty_data"_s } };
    switch (type) {
    case IntTypes::Int:
        data.append({ { (T)-1, 11ll, "08ffffffffffffffffff01"_ba },
                      { (T)-462, 11ll, "08b2fcffffffffffffff01"_ba },
                      { (T)-63585, 11ll, "089f8ffcffffffffffff01"_ba } });
        Q_FALLTHROUGH(); // Got negative values, fallthrough to positive
    case IntTypes::UInt:
        data.append({ { (T)15, 2ll, "080f"_ba },
                      { (T)300, 3ll, "08ac02"_ba },
                      { (T)65545, 4ll, "08898004"_ba } });
        break;
    case IntTypes::SInt:
        data.append({ { (T)15, 2ll, "081e"_ba },
                      { (T)300, 3ll, "08d804"_ba },
                      { (T)65545, 4ll, "08928008"_ba },
                      { (T)-1, 2ll, "0801"_ba },
                      { (T)-462, 3ll, "089b07"_ba },
                      { (T)-63585, 4ll, "08c1e107"_ba } });
        break;
    case IntTypes::SFixed:
        data.append({ { (T)-1, Fixed32IntSize, "0dffffffff"_ba },
                      { (T)-462, Fixed32IntSize, "0d32feffff"_ba },
                      { (T)-63585, Fixed32IntSize, "0d9f07ffff"_ba } });
        Q_FALLTHROUGH(); // Got negative values, fallthrough to positive
    case IntTypes::Fixed:
        data.append({ { (T)15, Fixed32IntSize, "0d0f000000"_ba },
                      { (T)300, Fixed32IntSize, "0d2c010000"_ba },
                      { (T)65545, Fixed32IntSize, "0d09000100"_ba } });
        break;
    }
    if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>) {
        if constexpr (type == IntTypes::Fixed || type == IntTypes::SFixed) {
            // Do not merge with 32-bit version
            data = { { (T)0, 0, ""_ba, u"empty_data"_s },
                     { (T)15, Fixed64IntSize, "090f00000000000000"_ba },
                     { (T)300, Fixed64IntSize, "092c01000000000000"_ba },
                     { (T)65545, Fixed64IntSize, "090900010000000000"_ba } };
        }
        if constexpr (type == IntTypes::SFixed) {
            data.append({ { (T)-1, Fixed64IntSize, "09ffffffffffffffff"_ba },
                          { (T)-462, Fixed64IntSize, "0932feffffffffffff"_ba },
                          { (T)-63585, Fixed64IntSize, "099f07ffffffffffff"_ba } });
        }
    }
    return data;
}

template <typename T, IntTypes type>
constexpr QList<SerializeData<T>> getInt32LimitValues()
{
    switch (type) {
    case IntTypes::Int:
        return { { (T)INT_MAX_SDATA(int8_t, 2ll, "087f"_ba) },
                 { (T)INT_MAX_SDATA(int16_t, 4ll, "08ffff01"_ba) },
                 { (T)INT_MAX_SDATA(int32_t, 6ll, "08ffffffff07"_ba) },
                 { (T)OVER_INT_MAX_SDATA(int8_t, 3ll, "088001"_ba) },
                 { (T)OVER_INT_MAX_SDATA(int16_t, 4ll, "08808002"_ba) },
                 { (T)INT_MIN_SDATA(int8_t, 11ll, "0880ffffffffffffffff01"_ba) },
                 { (T)INT_MIN_SDATA(int16_t, 11ll, "088080feffffffffffff01"_ba) },
                 { (T)INT_MIN_SDATA(int32_t, 11ll, "0880808080f8ffffffff01"_ba) },
                 { (T)BELOW_INT_MAX_SDATA(int8_t, 11ll, "08fffeffffffffffffff01"_ba) },
                 { (T)BELOW_INT_MAX_SDATA(int16_t, 11ll, "08fffffdffffffffffff01"_ba) } };
    case IntTypes::UInt:
        return { { (T)INT_MAX_SDATA(uint8_t, 3ll, "08ff01"_ba) },
                 { (T)INT_MAX_SDATA(uint16_t, 4ll, "08ffff03"_ba) },
                 { (T)INT_MAX_SDATA(uint32_t, 6ll, "08ffffffff0f"_ba) },
                 { (T)OVER_INT_MAX_SDATA(uint8_t, 3ll, "088002"_ba) },
                 { (T)OVER_INT_MAX_SDATA(uint16_t, 4ll, "08808004"_ba) } };
    case IntTypes::SInt:
        return { { (T)INT_MAX_SDATA(int8_t, 3ll, "08fe01"_ba) },
                 { (T)INT_MAX_SDATA(int16_t, 4ll, "08feff03"_ba) },
                 { (T)INT_MAX_SDATA(int32_t, 6ll, "08feffffff0f"_ba) },
                 { (T)OVER_INT_MAX_SDATA(int8_t, 3ll, "088002"_ba) },
                 { (T)OVER_INT_MAX_SDATA(int16_t, 4ll, "08808004"_ba) },
                 { (T)INT_MIN_SDATA(int8_t, 3ll, "08ff01"_ba) },
                 { (T)INT_MIN_SDATA(int16_t, 4ll, "08ffff03"_ba) },
                 { (T)INT_MIN_SDATA(int32_t, 6ll, "08ffffffff0f"_ba) },
                 { (T)BELOW_INT_MAX_SDATA(int8_t, 3ll, "088102"_ba) },
                 { (T)BELOW_INT_MAX_SDATA(int16_t, 4ll, "08818004"_ba) } };
    case IntTypes::Fixed:
        if constexpr (std::is_same_v<T, uint32_t>) {
            return { { (T)INT_MAX_SDATA(uint8_t, Fixed32IntSize, "0dff000000"_ba) },
                     { (T)INT_MAX_SDATA(uint16_t, Fixed32IntSize, "0dffff0000"_ba) },
                     { (T)INT_MAX_SDATA(uint32_t, Fixed32IntSize, "0dffffffff"_ba) },
                     { (T)OVER_INT_MAX_SDATA(uint8_t, Fixed32IntSize, "0d00010000"_ba) },
                     { (T)OVER_INT_MAX_SDATA(uint16_t, Fixed32IntSize, "0d00000100"_ba) } };
        }
    case IntTypes::SFixed:
        if constexpr (std::is_same_v<T, int32_t>) {
            return { { (T)INT_MAX_SDATA(int8_t, Fixed32IntSize, "0d7f000000"_ba) },
                     { (T)INT_MAX_SDATA(int16_t, Fixed32IntSize, "0dff7f0000"_ba) },
                     { (T)INT_MAX_SDATA(int32_t, Fixed32IntSize, "0dffffff7f"_ba) },
                     { (T)OVER_INT_MAX_SDATA(int8_t, Fixed32IntSize, "0d80000000"_ba) },
                     { (T)OVER_INT_MAX_SDATA(int16_t, Fixed32IntSize, "0d00800000"_ba) },
                     { (T)INT_MIN_SDATA(int8_t, Fixed32IntSize, "0d80ffffff"_ba) },
                     { (T)INT_MIN_SDATA(int16_t, Fixed32IntSize, "0d0080ffff"_ba) },
                     { (T)INT_MIN_SDATA(int32_t, Fixed32IntSize, "0d00000080"_ba) },
                     { (T)BELOW_INT_MAX_SDATA(int8_t, Fixed32IntSize, "0d7fffffff"_ba) },
                     { (T)BELOW_INT_MAX_SDATA(int16_t, Fixed32IntSize, "0dff7fffff"_ba) } };
        }
    }
    return {};
}

template <typename T, IntTypes type>
constexpr QList<SerializeData<T>> getInt64LimitValues()
{
    switch (type) {
    case IntTypes::Int:
        return { { (T)INT_MAX_SDATA(int64_t, 10ll, "08ffffffffffffffff7f"_ba) },
                 { (T)OVER_INT_MAX_SDATA(int32_t, 6ll, "088080808008"_ba) },
                 { (T)INT_MIN_SDATA(int64_t, 11ll, "0880808080808080808001"_ba) },
                 { (T)BELOW_INT_MAX_SDATA(int32_t, 11ll, "08fffffffff7ffffffff01"_ba) } };
    case IntTypes::UInt:
        return { { (T)INT_MAX_SDATA(int64_t, 10ll, "08ffffffffffffffff7f"_ba) },
                 { (T)OVER_INT_MAX_SDATA(int32_t, 6ll, "088080808008"_ba) } };
    case IntTypes::SInt:
        return { { (T)INT_MAX_SDATA(int64_t, 11ll, "08feffffffffffffffff01"_ba) },
                 { (T)OVER_INT_MAX_SDATA(int32_t, 6ll, "088080808010"_ba) },
                 { (T)INT_MIN_SDATA(int64_t, 11ll, "08ffffffffffffffffff01"_ba) },
                 { (T)BELOW_INT_MAX_SDATA(int32_t, 6ll, "088180808010"_ba) } };
    case IntTypes::Fixed:
        return { { (T)INT_MAX_SDATA(uint8_t, Fixed64IntSize, "09ff00000000000000"_ba) },
                 { (T)INT_MAX_SDATA(uint16_t, Fixed64IntSize, "09ffff000000000000"_ba) },
                 { (T)INT_MAX_SDATA(uint32_t, Fixed64IntSize, "09ffffffff00000000"_ba) },
                 { (T)INT_MAX_SDATA(uint64_t, Fixed64IntSize, "09ffffffffffffffff"_ba) },
                 { (T)OVER_INT_MAX_SDATA(uint8_t, Fixed64IntSize, "090001000000000000"_ba) },
                 { (T)OVER_INT_MAX_SDATA(uint16_t, Fixed64IntSize, "090000010000000000"_ba) },
                 { (T)OVER_INT_MAX_SDATA(uint32_t, Fixed64IntSize, "090000000001000000"_ba) } };
    case IntTypes::SFixed:
        return { { (T)INT_MAX_SDATA(int8_t, Fixed64IntSize, "097f00000000000000"_ba) },
                 { (T)INT_MAX_SDATA(int16_t, Fixed64IntSize, "09ff7f000000000000"_ba) },
                 { (T)INT_MAX_SDATA(int32_t, Fixed64IntSize, "09ffffff7f00000000"_ba) },
                 { (T)INT_MAX_SDATA(int64_t, Fixed64IntSize, "09ffffffffffffff7f"_ba) },
                 { (T)OVER_INT_MAX_SDATA(int8_t, Fixed64IntSize, "098000000000000000"_ba) },
                 { (T)OVER_INT_MAX_SDATA(int16_t, Fixed64IntSize, "090080000000000000"_ba) },
                 { (T)OVER_INT_MAX_SDATA(int32_t, Fixed64IntSize, "090000008000000000"_ba) },
                 { (T)INT_MIN_SDATA(int8_t, Fixed64IntSize, "0980ffffffffffffff"_ba) },
                 { (T)INT_MIN_SDATA(int16_t, Fixed64IntSize, "090080ffffffffffff"_ba) },
                 { (T)INT_MIN_SDATA(int32_t, Fixed64IntSize, "0900000080ffffffff"_ba) },
                 { (T)INT_MIN_SDATA(int64_t, Fixed64IntSize, "090000000000000080"_ba) },
                 { (T)BELOW_INT_MAX_SDATA(int8_t, Fixed64IntSize, "097fffffffffffffff"_ba) },
                 { (T)BELOW_INT_MAX_SDATA(int16_t, Fixed64IntSize, "09ff7fffffffffffff"_ba) },
                 { (T)BELOW_INT_MAX_SDATA(int32_t, Fixed64IntSize, "09ffffff7fffffffff"_ba) } };
    }
    return {};
}

template <typename T, IntTypes type, typename Enable = void>
struct SerializeDataGenerator
{
    static QList<SerializeData<T>> getSerializeData()
    {
        return getCommonIntValues<T, type>() + getInt32LimitValues<T, type>();
    };
};

template <typename T, IntTypes type>
struct SerializeDataGenerator<
        T, type,
        std::enable_if_t<std::disjunction_v<std::is_same<T, int64_t>, std::is_same<T, uint64_t>>>>
{
    static QList<SerializeData<T>> getSerializeData()
    {
        return getCommonIntValues<T, type>() + getInt32LimitValues<T, type>()
                + getInt64LimitValues<T, type>();
    }
};

} // namespace qtprotobufnamespace::tests
