// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 Ivan Komissarov.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "ddsheader.h"

#ifndef QT_NO_DATASTREAM

QT_BEGIN_NAMESPACE

QDataStream &operator>>(QDataStream &s, DDSPixelFormat &pixelFormat)
{
    s >> pixelFormat.size;
    s >> pixelFormat.flags;
    s >> pixelFormat.fourCC;
    s >> pixelFormat.rgbBitCount;
    s >> pixelFormat.rBitMask;
    s >> pixelFormat.gBitMask;
    s >> pixelFormat.bBitMask;
    s >> pixelFormat.aBitMask;
    return s;
}

QDataStream &operator<<(QDataStream &s, const DDSPixelFormat &pixelFormat)
{
    s << pixelFormat.size;
    s << pixelFormat.flags;
    s << pixelFormat.fourCC;
    s << pixelFormat.rgbBitCount;
    s << pixelFormat.rBitMask;
    s << pixelFormat.gBitMask;
    s << pixelFormat.bBitMask;
    s << pixelFormat.aBitMask;
    return s;
}

QDataStream &operator>>(QDataStream &s, DDSHeader &header)
{
    s >> header.magic;
    s >> header.size;
    s >> header.flags;
    s >> header.height;
    s >> header.width;
    s >> header.pitchOrLinearSize;
    s >> header.depth;
    s >> header.mipMapCount;
    for (int i = 0; i < DDSHeader::ReservedCount; i++)
        s >> header.reserved1[i];
    s >> header.pixelFormat;
    s >> header.caps;
    s >> header.caps2;
    s >> header.caps3;
    s >> header.caps4;
    s >> header.reserved2;
    return s;
}

QDataStream &operator<<(QDataStream &s, const DDSHeader &header)
{
    s << header.magic;
    s << header.size;
    s << header.flags;
    s << header.height;
    s << header.width;
    s << header.pitchOrLinearSize;
    s << header.depth;
    s << header.mipMapCount;
    for (int i = 0; i < DDSHeader::ReservedCount; i++)
        s << header.reserved1[i];
    s << header.pixelFormat;
    s << header.caps;
    s << header.caps2;
    s << header.caps3;
    s << header.caps4;
    s << header.reserved2;
    return s;
}

QDataStream &operator>>(QDataStream &s, DDSHeaderDX10 &header)
{
    s >> header.dxgiFormat;
    s >> header.resourceDimension;
    s >> header.miscFlag;
    s >> header.arraySize;
    s >> header.reserved;
    return s;
}

QDataStream &operator<<(QDataStream &s, const DDSHeaderDX10 &header)
{
    s << header.dxgiFormat;
    s << header.resourceDimension;
    s << header.miscFlag;
    s << header.arraySize;
    s << header.reserved;
    return s;
}

QT_END_NAMESPACE

#endif // QT_NO_DATASTREAM
