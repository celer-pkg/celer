// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "utils.h"

#include <cassert>
#include <cctype>
#include <filesystem>

namespace {
constexpr std::string_view AsciiSpacing = " \t\n\r\f\v";
}

namespace qtprotoccommon::utils {

bool isAsciiAlpha(char c)
{
    return (unsigned char)c <= 127 && ::isalpha(c);
}

bool isAsciiAlnum(char c)
{
    return (unsigned char)c <= 127 && std::isalnum(c);
}

std::vector<std::string> split(std::string_view s, std::string_view c, bool keepEmpty)
{
    assert(!c.empty());
    std::vector<std::string> out;
    std::string::size_type pos = 0;
    for (std::string::size_type posNext = 0; (posNext = s.find(c, pos)) != std::string::npos;
         pos = posNext + c.size()) {
        auto item = s.substr(pos, posNext - pos);
        if (keepEmpty || !item.empty())
            out.emplace_back(item);
    }
    auto item = s.substr(pos);
    if (keepEmpty || !item.empty())
        out.emplace_back(item);
    return out;
}

std::string replace(std::string_view where, std::string_view from, std::string_view to)
{
    assert(!from.empty());
    if (from == to)
        return std::string(where);

    std::string out;
    out.reserve(where.size());
    std::size_t pos = 0;
    for (std::string::size_type posNext = 0; (posNext = where.find(from, pos)) != std::string::npos;
         pos = posNext + from.size()) {
        out += where.substr(pos, posNext - pos);
        out += to;
    }
    out += where.substr(pos);

    return out;
}

void asciiToLower(std::string &str)
{
    std::transform(std::begin(str), std::end(str), std::begin(str), utils::toAsciiLower);
}

void asciiToUpper(std::string &str)
{
    std::transform(std::begin(str), std::end(str), std::begin(str), utils::toAsciiUpper);
}

std::string removeFileSuffix(std::string_view fileName)
{
    std::filesystem::path path(fileName);
    return (path.parent_path() / path.stem()).string();
}

std::string extractFileBasename(std::string_view fileName)
{
    return std::filesystem::path(fileName).stem().string();
}

std::string toValidIdentifier(std::string_view name)
{
    assert(!name.empty() && "empty names are not supported as identifier");
    std::string out;
    out.reserve(name.size() + 1);

    if (!isAsciiAlpha(name[0]) && name[0] != '_') // omitted Unicode with XID_Start
        out += '_';

    for (const auto c : name) {
        if (isAsciiAlnum(c) || c == '_') // omitted Unicode with XID_Continue
            out += c;
        else
            out += '_'; // TODO: a deterministic hex - ASCII mapping algorithm would be better
    }

    return out;
}

std::string capitalizeAsciiName(std::string_view name)
{
    std::string result(name);
    if (result.empty() || !isAsciiAlpha(result[0]))
        return result;
    result[0] &= ~char(0x20);
    return result;
}

std::string deCapitalizeAsciiName(std::string_view name)
{
    std::string result(name);
    if (result.empty() || !isAsciiAlpha(result[0]))
        return result;
    result[0] |= char(0x20);
    return result;
}

std::string &rtrim(std::string &s)
{
    const size_t cut = s.find_last_not_of(AsciiSpacing);
    s.erase(cut != std::string::npos ? cut + 1 : 0);
    return s;
}

std::string &ltrim(std::string &s)
{
    const size_t cut = s.find_first_not_of(AsciiSpacing);
    s.erase(0, cut == std::string::npos ? s.size() : cut);
    return s;
}

std::string &trim(std::string &s)
{
    return ltrim(rtrim(s));
}

bool HeaderComparator::operator()(const std::string &lhs, const std::string &rhs) const
{
    static constexpr std::string_view qtCorePrefix = "QtCore/";

    bool lhsStartsWithQtCore = utils::startsWith(lhs, qtCorePrefix);
    bool rhsStartsWithQtCore = utils::startsWith(rhs, qtCorePrefix);
    if (lhsStartsWithQtCore && !rhsStartsWithQtCore)
        return false;
    if (!lhsStartsWithQtCore && rhsStartsWithQtCore)
        return true;

    return lhs < rhs;
}


// TODO C++20: use the std::string(_view) methods directly
template<typename T>
bool containsImpl(std::string_view s, T c)
{
    return s.find(c) != std::string::npos;
}

// TODO C++20: use the std::string(_view) methods directly
bool contains(std::string_view s, char c)
{
    return containsImpl(s, c);
}

// TODO C++20: use the std::string(_view) methods directly
bool contains(std::string_view s, std::string_view c)
{
    return containsImpl(s, c);
}

// TODO C++20: use the std::string(_view) methods directly
bool endsWith(std::string_view s, char c)
{
    return !s.empty() && s.back() == c;
}

// TODO C++20: use the std::string(_view) methods directly
bool endsWith(std::string_view s, std::string_view c)
{
    return s.size() >= c.size() && s.compare(s.size() - c.size(), c.size(), c) == 0;
}

// TODO C++20: use the std::string(_view) methods directly
bool startsWith(std::string_view s, char c)
{
    return !s.empty() && s.front() == c;
}

// TODO C++20: use the std::string(_view) methods directly
bool startsWith(std::string_view s, std::string_view c)
{
    return s.size() >= c.size() && s.compare(0, c.size(), c) == 0;
}

size_t count(std::string_view s, char c)
{
    return std::count(s.begin(), s.end(), c);
}
} // namespace qtprotoccommon::utils
