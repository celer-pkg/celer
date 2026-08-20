// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qspirvshaderremap_p.h"

#include <SPIRV/SPVRemapper.h>

QT_BEGIN_NAMESPACE

void QSpirvShaderRemapper::remapErrorHandler(const std::string &s)
{
    if (!remapErrorMsg.isEmpty())
        remapErrorMsg.append(QLatin1Char('\n'));
    remapErrorMsg.append(QString::fromStdString(s));
}

void QSpirvShaderRemapper::remapLogHandler(const std::string &)
{
}

QByteArray QSpirvShaderRemapper::remap(const QByteArray &ir, QSpirvShader::RemapFlags flags)
{
    if (ir.isEmpty())
        return QByteArray();

    remapErrorMsg.clear();

    spv::spirvbin_t b;
    b.registerErrorHandler(std::bind(&QSpirvShaderRemapper::remapErrorHandler, this, std::placeholders::_1));
    b.registerLogHandler(std::bind(&QSpirvShaderRemapper::remapLogHandler, this, std::placeholders::_1));

    const uint32_t opts = flags.testFlag(QSpirvShader::RemapFlag::StripOnly) ? spv::spirvbin_t::STRIP
                                                                             : spv::spirvbin_t::DO_EVERYTHING;

    std::vector<uint32_t> v;
    v.resize(ir.size() / 4);
    memcpy(v.data(), ir.constData(), v.size() * 4);

    b.remap(v, opts);

    if (!remapErrorMsg.isEmpty())
        return QByteArray();

    return QByteArray(reinterpret_cast<const char *>(v.data()), int(v.size()) * 4);
}

QT_END_NAMESPACE
