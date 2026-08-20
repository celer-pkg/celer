// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef GRPCTEMPLATES_H
#define GRPCTEMPLATES_H

namespace QtGrpc {

class GrpcTemplates
{
public:
    // gRPC
    static const char *ChildClassDeclarationTemplate();

    static const char *ClientQmlDeclarationTemplate();

    static const char *GrpcClientFileSuffix();
    static const char *GrpcServiceFileSuffix();

    static const char *ClientConstructorDeclarationTemplate();
    static const char *ClientConstructorDefinitionTemplate();
    static const char *ClientDestructorDeclarationTemplate();
    static const char *ClientDestructorDefinitionTemplate();
    static const char *ClientQmlConstructorDefinitionTemplate();

    static const char *ClientMethodDeclarationAsyncTemplate();
    static const char *ClientMethodDeclarationQmlTemplate();

    static const char *ServerMethodDeclarationTemplate();

    static const char *ClientMethodDefinitionAsyncTemplate();
    static const char *ClientMethodDefinitionQmlTemplate();

    static const char *ClientMethodStreamDeclarationTemplate();
    static const char *ClientMethodStreamDefinitionTemplate();

    static const char *StreamSenderDeclarationQmlTemplate();

    static const char *ClientMethodServerStreamDeclarationQmlTemplate();
    static const char *ClientMethodServerStreamDefinitionQmlTemplate();

    static const char *ClientMethodClientStreamDeclarationQmlTemplate();
    static const char *ClientMethodClientStreamDefinitionQmlTemplate();

    static const char *ClientMethodBidiStreamDeclarationQmlTemplate();
    static const char *ClientMethodBidiStreamDefinitionQmlTemplate();
};

} // namespace QtGrpc

#endif // GRPCTEMPLATES_H
