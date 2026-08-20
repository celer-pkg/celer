// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPATH_P_H
#define QCANVASPATH_P_H

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

#include "qcanvaspath.h"
#include "engine/qcpainterengineutils_p.h"

QT_BEGIN_NAMESPACE

class QCanvasPathPrivate
{
public:
    static QCanvasPathPrivate *get(QCanvasPath *path) { return path->d_ptr; }
    static const QCanvasPathPrivate *get(const QCanvasPath *path) { return path->d_ptr; }

    static int dataSizeOf(QCCommand command) {
        switch (command) {
        case QCCommand::MoveTo:
        case QCCommand::LineTo:
            return 2;
        case QCCommand::BezierTo:
            return 6;
        case QCCommand::Close:
        case QCCommand::WindingCW:
        case QCCommand::WindingCCW:
        default:
            return 0;
        }
    };

    void appendCommand(QCCommand command);
    void appendCommands(const QCCommand commands[], int cCount);
    void appendCommandsData(const float commandsData[], int dCount);
    void ensureCommands(int addition);
    void ensureCommandsData(int addition);

    QCCommands commands;
    QCCommandsData commandsData;
    qsizetype commandsCount = 0;
    qsizetype commandsDataCount = 0;
    // Distance tolerance for consecutive points
    float distTol = 0.01f;
    // When this increases, path has been (re)created
    // so all commands need to be re-evaluated.
    int pathIterations = 0;
};

QT_END_NAMESPACE

#endif // QCANVASPATH_P_H
