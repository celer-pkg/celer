// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#ifndef QCODECMACROS_P_H
#define QCODECMACROS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of other Qt classes. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

#define InRange(c, lower, upper) (((c) >= (lower)) && ((c) <= (upper)))
#define IsLatin(c) ((c) <= 0x7F)
#define IsByteInGb2312(c) (InRange((c), 0xA1, 0xFE))
#define Is1stByte(c) (InRange((c), 0x81, 0xFE))
#define Is2ndByteIn2Bytes(c) (InRange((c), 0x40, 0xFE) && (c) != 0x7F)
#define Is2ndByteIn4Bytes(c) (InRange((c), 0x30, 0x39))
#define Is2ndByte(c) (Is2ndByteIn2Bytes(c) || Is2ndByteIn4Bytes(c))
#define Is3rdByte(c) (InRange((c), 0x81, 0xFE))
#define Is4thByte(c) (InRange((c), 0x30, 0x39))

#define qValidChar(u) ((u) ? (u) : static_cast<ushort>(QChar::ReplacementCharacter))

/* User-defined areas:      UDA 1: 0xAAA1 - 0xAFFE (564/0)
                            UDA 2: 0xF8A1 - 0xFEFE (658/0)
                            UDA 3: 0xA140 - 0xA7A0 (672/0) */
#define IsUDA1(a, b) (InRange((a), 0xAA, 0xAF) && InRange((b), 0xA1, 0xFE))
#define IsUDA2(a, b) (InRange((a), 0xF8, 0xFE) && InRange((b), 0xA1, 0xFE))
#define IsUDA3(a, b) (InRange((a), 0xA1, 0xA7) && InRange((b), 0x40, 0xA0) && ((b) != 0x7F))

#define IsSecondByteRange1(c) (InRange((c), 0x40, 0x7E))
#define IsSecondByteRange2(c) (InRange((c), 0xA1, 0xFE))
#define IsSecondByte(c) (IsSecondByteRange1(c) || IsSecondByteRange2(c))
#define QValidChar(u) ((u) ? QChar((ushort)(u)) : QChar(QChar::ReplacementCharacter))

#define IsKana(c) (((c) >= 0xa1) && ((c) <= 0xdf))

#define IsEucChar(c) (((c) >= 0xa1) && ((c) <= 0xfe))
#define IsCP949Char(c) (((c) >= 0x81) && ((c) <= 0xa0))

#define IsJisChar(c) (((c) >= 0x21) && ((c) <= 0x7e))
#define IsSjisUDC1(c) (((c) >= 0xf0) && ((c) <= 0xfc))
#define IsSjisIBMVDCChar1(c) (((c) >= 0xfa) && ((c) <= 0xfc))

#define IsSjisChar1(c) ((((c) >= 0x81) && ((c) <= 0x9f)) || (((c) >= 0xe0) && ((c) <= 0xfc)))
#define IsSjisChar2(c) (((c) >= 0x40) && ((c) != 0x7f) && ((c) <= 0xfc))
#define IsUserDefinedChar1(c) (((c) >= 0xf0) && ((c) <= 0xfc))

#define IsTSCIIChar(c) (((c) >= 0x80) && ((c) <= 0xfd))

QT_END_NAMESPACE

#endif // QCODECMACROS_P_H
