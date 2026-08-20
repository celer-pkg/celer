// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "wasmbinary.h"

#include <QFile>

#include <iostream>

WasmBinary::WasmBinary(QString filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "ERROR: Cannot open the file " << filepath.toStdString() << std::endl;
        std::cout << file.errorString().toStdString() << std::endl;
        type = WasmBinary::Type::INVALID;
        return;
    }
    auto bytes = file.readAll();
    if (!parsePreambule(bytes)) {
        type = WasmBinary::Type::INVALID;
    }
}

bool WasmBinary::parsePreambule(QByteArrayView data)
{
    const auto preambuleSize = 24;
    if (data.size() < preambuleSize) {
        std::cout << "ERROR: Preambule of binary shorter than expected!" << std::endl;
        return false;
    }
    uint32_t int32View[6];
    std::memcpy(int32View, data.data(), sizeof(int32View));
    if (int32View[0] != 0x6d736100) {
        std::cout << "ERROR: Magic WASM number not found in binary. Binary corrupted?" << std::endl;
        return false;
    }
    if (data[8] != 0) {
        type = WasmBinary::Type::STATIC;
        return true;
    } else {
        type = WasmBinary::Type::SHARED;
    }
    const auto sectionStart = 9;
    size_t offset = sectionStart;
    auto sectionSize = getLeb(data, offset);
    auto sectionEnd = sectionStart + sectionSize;
    auto name = getString(data, offset);
    if (name != "dylink.0") {
        type = WasmBinary::Type::INVALID;
        std::cout << "ERROR: dylink.0 was not found in supposedly dynamically linked module"
                  << std::endl;
        return false;
    }

    const auto WASM_DYLINK_NEEDED = 0x2;
    while (offset < sectionEnd) {
        auto subsectionType = data[offset++];
        auto subsectionSize = getLeb(data, offset);
        if (subsectionType == WASM_DYLINK_NEEDED) {
            auto neededDynlibsCount = getLeb(data, offset);
            while (neededDynlibsCount--) {
                dependencies.append(getString(data, offset));
            }
        } else {
            offset += subsectionSize;
        }
    }
    return true;
}

size_t WasmBinary::getLeb(QByteArrayView data, size_t &offset)
{
    auto ret = 0;
    auto mul = 1;
    while (true) {
        auto byte = data[offset++];
        ret += (byte & 0x7f) * mul;
        mul *= 0x80;
        if (!(byte & 0x80))
            break;
    }
    return ret;
}

QString WasmBinary::getString(QByteArrayView data, size_t &offset)
{
    auto length = getLeb(data, offset);
    offset += length;
    return QString::fromUtf8(data.sliced(offset - length, length));
}
