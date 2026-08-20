// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_GLSLASTDUMP_H
#define QSSG_GLSLASTDUMP_H

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

#include <QtQuick3DGlslParser/private/glslastvisitor_p.h>

QT_BEGIN_NAMESPACE

class QTextStream;

namespace GLSL {

class Q_QUICK3DGLSLPARSER_EXPORT ASTDump: protected Visitor
{
public:
    ASTDump(QTextStream &out);

    void operator()(AST *ast);

protected:
    bool preVisit(AST *) override;
    void postVisit(AST *) override;

private:
    QTextStream &out;
    int _depth;
};

} // namespace GLSL

QT_END_NAMESPACE

#endif // QSSG_GLSLASTDUMP_H
