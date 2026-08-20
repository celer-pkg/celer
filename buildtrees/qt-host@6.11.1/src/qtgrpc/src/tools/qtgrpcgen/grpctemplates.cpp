// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:critical reason:data-parser

#include "grpctemplates.h"

using namespace ::QtGrpc;

const char *GrpcTemplates::ChildClassDeclarationTemplate()
{
    return "\nclass $export_macro$$classname$ : public $parent_class$\n"
           "{\n"
           "    Q_OBJECT\n";
}

const char *GrpcTemplates::ClientQmlDeclarationTemplate()
{
    return "    QML_NAMED_ELEMENT($service_name$Client)\n";
}

const char *GrpcTemplates::ClientMethodDeclarationAsyncTemplate()
{
    return "[[nodiscard]]\n"
           "std::unique_ptr<QGrpcCallReply> $method_name$(const $param_type$ &$param_name$);\n"
           "[[nodiscard]]\n"
           "std::unique_ptr<QGrpcCallReply> $method_name$(const $param_type$ &$param_name$, const "
           "QGrpcCallOptions &options);\n";
}

const char *GrpcTemplates::ClientMethodDeclarationQmlTemplate()
{
    return "Q_INVOKABLE void $method_name$(const $param_type$ &$param_name$, "
           "const QJSValue &callback, "
           "const QJSValue &errorCallback, "
           "const QtGrpcQuickPrivate::QQmlGrpcCallOptions *options = nullptr);\n";
}

const char *GrpcTemplates::ServerMethodDeclarationTemplate()
{
    return "Q_INVOKABLE virtual $return_type$ $method_name$(const $param_type$ &$param_name$) = "
           "0;\n";
}

const char *GrpcTemplates::ClientConstructorDeclarationTemplate()
{
    return "explicit $classname$(QObject *parent = nullptr);\n";
}

const char *GrpcTemplates::ClientConstructorDefinitionTemplate()
{
    return "\n$classname$::$classname$(QObject *parent)\n"
           "    : $parent_class$(\"$service_name$\"_L1, "
           "parent)\n"
           "{\n"
           "}\n\n";
}

const char *GrpcTemplates::ClientDestructorDeclarationTemplate()
{
    return "~$classname$() override;\n\n";
}

const char *GrpcTemplates::ClientDestructorDefinitionTemplate()
{
    return "$classname$::~$classname$() = default;\n";
}

const char *GrpcTemplates::ClientQmlConstructorDefinitionTemplate()
{
    return "\n$classname$::$classname$(QObject *parent)\n"
           "    : $parent_class$(parent)\n"
           "{\n"
           "}\n\n";
}

const char *GrpcTemplates::ClientMethodDefinitionAsyncTemplate()
{
    return "\nstd::unique_ptr<QGrpcCallReply> $classname$::$method_name$(const $param_type$ "
           "&$param_name$)\n"
           "{\n"
           "    return $method_name$($param_name$, {});\n"
           "}\n\n"
           "\nstd::unique_ptr<QGrpcCallReply> $classname$::$method_name$(const $param_type$ "
           "&$param_name$, const QGrpcCallOptions &options)\n"
           "{\n"
           "    auto reply = call(\"$method_name$\"_L1, $param_name$, options);\n"
           "    if (auto *replyPtr = reply.get(); replyPtr != nullptr) {\n"
           "        setOperationResponseMetaType(replyPtr,"
           " QMetaType::fromType<$return_type$>());\n"
           "    }\n"
           "    return reply;\n"
           "}\n\n";
}

const char *GrpcTemplates::ClientMethodDefinitionQmlTemplate()
{
    return "\nvoid $classname$::$method_name$(const $param_type$ &$param_name$,\n"
           "                const QJSValue &finishCallback,\n"
           "                const QJSValue &errorCallback,\n"
           "                const QtGrpcQuickPrivate::QQmlGrpcCallOptions *options)\n"
           "{\n"
           "    QJSEngine *jsEngine = qjsEngine(this);\n"
           "    if (jsEngine == nullptr) {\n"
           "        qWarning() << \"Unable to call $classname$::$method_name$, it's only callable "
           "from JS engine context\";\n"
           "        return;\n"
           "    }\n\n"
           "    auto reply = Client::$method_name$($param_name$,"
           " options ? options->options() : QGrpcCallOptions{});\n"
           "    QtGrpcQuickFunctional::makeCallConnections<$return_type$>(jsEngine,\n"
           "                        std::move(reply), finishCallback, errorCallback);\n"
           "}\n\n";
}

const char *GrpcTemplates::ClientMethodStreamDeclarationTemplate()
{
    return "[[nodiscard]]\n"
           "std::unique_ptr<QGrpc$stream_type$Stream> $method_name$(const $param_type$ "
           "&$param_name$);\n"
           "[[nodiscard]]\n"
           "std::unique_ptr<QGrpc$stream_type$Stream> $method_name$(const $param_type$ "
           "&$param_name$, const QGrpcCallOptions &options);\n";
}

const char *GrpcTemplates::ClientMethodStreamDefinitionTemplate()
{
    return "std::unique_ptr<QGrpc$stream_type$Stream> $classname$::$method_name$("
           "const $param_type$ &$param_name$)\n"
           "{\n"
           "    return $method_name$($param_name$, {});"
           "}\n\n"
           "std::unique_ptr<QGrpc$stream_type$Stream> $classname$::$method_name$("
           "const $param_type$ &$param_name$, const QGrpcCallOptions &options)\n"
           "{\n"
           "    auto stream = $stream_type_lower$Stream(\"$method_name$\"_L1, $param_name$,"
           " options);\n"
           "    if (auto *streamPtr = stream.get(); streamPtr != nullptr) {\n"
           "        setOperationResponseMetaType(streamPtr,"
           " QMetaType::fromType<$return_type$>());\n"
           "    }\n"
           "    return stream;\n"
           "}\n\n";
}

const char *GrpcTemplates::StreamSenderDeclarationQmlTemplate()
{
    // We use the Q_MOC_RUN trick here to fool qmltyperegistrar. The template base class doesn't
    // have any meta-type extras, and it cannot have them. So from the meta-type information
    // perspective it behaves in exactly the same way as QObject.
    return "class $export_macro$$sender_class_name$ : public QQmlGrpc$stream_type$StreamSender\n"
           "{\n"
           "    Q_OBJECT\n"
           "    QML_NAMED_ELEMENT($sender_qml_name$)\n"
           "    QML_UNCREATABLE(\"$sender_qml_name$ can only be created by gRPC "
           "client instance\")\n"
           "\n"
           "    $sender_class_name$(QGrpc$stream_type$Stream *stream) : "
           "QQmlGrpc$stream_type$StreamSender(stream) {}\n\n"
           "public:\n"
           "    Q_INVOKABLE void writeMessage(const $param_type$ &$param_name$)\n"
           "    {\n"
           "        writeMessageImpl($param_name$);\n"
           "    }\n"
           "    friend class $service_name$::$classname$;\n"
           "};\n\n";
}

const char *GrpcTemplates::ClientMethodServerStreamDeclarationQmlTemplate()
{
    return "Q_INVOKABLE void $method_name$(const $param_type$ &$param_name$,\n"
           "                const QJSValue &messageCallback,\n"
           "                const QJSValue &finishCallback,\n"
           "                const QJSValue &errorCallback,\n"
           "                const QtGrpcQuickPrivate::QQmlGrpcCallOptions *options = nullptr);\n";
}

const char *GrpcTemplates::ClientMethodClientStreamDeclarationQmlTemplate()
{
    return "Q_INVOKABLE $sender_class_name$ *$method_name$("
           "const $param_type$ &$param_name$,\n"
           "                const QJSValue &finishCallback,\n"
           "                const QJSValue &errorCallback,\n"
           "                const QtGrpcQuickPrivate::QQmlGrpcCallOptions *options = nullptr);\n";
}

const char *GrpcTemplates::ClientMethodBidiStreamDeclarationQmlTemplate()
{
    return "Q_INVOKABLE $sender_class_name$ *$method_name$("
           "const $param_type$ &$param_name$,\n"
           "                const QJSValue &messageCallback,\n"
           "                const QJSValue &finishCallback,\n"
           "                const QJSValue &errorCallback,\n"
           "                const QtGrpcQuickPrivate::QQmlGrpcCallOptions *options = nullptr);\n";
}

const char *GrpcTemplates::ClientMethodServerStreamDefinitionQmlTemplate()
{
    return "\nvoid $classname$::$method_name$(const $param_type$ &$param_name$,\n"
           "            const QJSValue &messageCallback,\n"
           "            const QJSValue &finishCallback,\n"
           "            const QJSValue &errorCallback,\n"
           "            const QtGrpcQuickPrivate::QQmlGrpcCallOptions *options)\n"
           "{\n"
           "    QJSEngine *jsEngine = qjsEngine(this);\n"
           "    if (jsEngine == nullptr) {\n"
           "        qWarning() << \"Unable to call $classname$::$method_name$, it's only callable "
           "from JS engine context\";\n"
           "        return;\n"
           "    }\n\n"
           "    auto stream = Client::$method_name$($param_name$,"
           " options ? options->options() : QGrpcCallOptions{});\n"
           "    QtGrpcQuickFunctional::makeServerStreamConnections<$return_type$>(jsEngine,\n"
           "                        std::move(stream),\n"
           "                        messageCallback, finishCallback, errorCallback);\n"
           "}\n\n";
}

const char *GrpcTemplates::ClientMethodClientStreamDefinitionQmlTemplate()
{
    return "\n$sender_class_name$ *$classname$::$method_name$("
           "const $param_type$ &$param_name$,\n"
           "        const QJSValue &finishCallback,\n"
           "        const QJSValue &errorCallback,\n"
           "        const QtGrpcQuickPrivate::QQmlGrpcCallOptions *options)\n"
           "{\n"
           "    QJSEngine *jsEngine = qjsEngine(this);\n"
           "    if (jsEngine == nullptr) {\n"
           "        qWarning() << \"Unable to call $classname$::$method_name$, it's only callable "
           "from JS engine context\";\n"
           "        return nullptr;\n"
           "    }\n\n"
           "    auto stream = Client::$method_name$($param_name$,"
           " options ? options->options() : QGrpcCallOptions{});\n"
           "    auto *sender = new $sender_class_name$(stream.get());\n"
           "    QtGrpcQuickFunctional::makeClientStreamConnections<$return_type$>(jsEngine,\n"
           "                        std::move(stream), finishCallback, errorCallback);\n"
           "    QJSEngine::setObjectOwnership(sender, QJSEngine::JavaScriptOwnership);\n"
           "    return sender;\n"
           "}\n\n";
}

const char *GrpcTemplates::ClientMethodBidiStreamDefinitionQmlTemplate()
{
    return "\n$sender_class_name$ *$classname$::$method_name$("
           "const $param_type$ &$param_name$,\n"
           "    const QJSValue &messageCallback,\n"
           "    const QJSValue &finishCallback,\n"
           "    const QJSValue &errorCallback,\n"
           "    const QtGrpcQuickPrivate::QQmlGrpcCallOptions *options)\n"
           "{\n"
           "    QJSEngine *jsEngine = qjsEngine(this);\n"
           "    if (jsEngine == nullptr) {\n"
           "        qWarning() << \"Unable to call $classname$::$method_name$, it's only callable "
           "from JS engine context\";\n"
           "        return nullptr;\n"
           "    }\n\n"
           "    auto stream = Client::$method_name$($param_name$,"
           " options ? options->options() : QGrpcCallOptions {});\n"
           "    auto *sender = new $sender_class_name$(stream.get());\n"
           "    QtGrpcQuickFunctional::makeBidiStreamConnections<$return_type$>(jsEngine,\n"
           "                        std::move(stream), messageCallback, finishCallback, "
           "errorCallback);\n"
           "    QJSEngine::setObjectOwnership(sender, QJSEngine::JavaScriptOwnership);\n"
           "    return sender;\n"
           "}\n\n";
}

const char *GrpcTemplates::GrpcClientFileSuffix()
{
    return "_client.grpc";
}

const char *GrpcTemplates::GrpcServiceFileSuffix()
{
    return "_service.grpc";
}
