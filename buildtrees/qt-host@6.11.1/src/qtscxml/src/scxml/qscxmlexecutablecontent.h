// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSCXMLEXECUTABLECONTENT_H
#define QSCXMLEXECUTABLECONTENT_H

#include <QtScxml/qscxmlglobals.h>

QT_BEGIN_NAMESPACE

namespace QScxmlExecutableContent {

typedef qint32 ContainerId;
enum { NoContainer = -1 };
typedef qint32 StringId;
enum { NoString = -1 };
typedef qint32 InstructionId;
enum { NoInstruction = -1 };
typedef qint32 EvaluatorId;
enum { NoEvaluator = -1 };

#if defined(Q_CC_MSVC) || defined(Q_CC_GNU)
#pragma pack(push, 4) // 4 == sizeof(qint32)
#endif
struct EvaluatorInfo {
    StringId expr;
    StringId context;
};

struct AssignmentInfo {
    StringId dest;
    StringId expr;
    StringId context;
};

struct ForeachInfo {
    StringId array;
    StringId item;
    StringId index;
    StringId context;
};

struct ParameterInfo {
    StringId name;
    EvaluatorId expr;
    StringId location;
};

struct InvokeInfo {
    StringId id;
    StringId prefix;
    StringId location;
    StringId context;
    EvaluatorId expr;
    ContainerId finalize;
    bool autoforward;
};
#if defined(Q_CC_MSVC) || defined(Q_CC_GNU)
#pragma pack(pop)
#endif

} // QScxmlExecutableContent namespace

QT_END_NAMESPACE

#endif // QSCXMLEXECUTABLECONTENT_H
