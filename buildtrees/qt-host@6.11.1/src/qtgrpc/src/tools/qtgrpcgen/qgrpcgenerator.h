// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QGRPCGENERATOR_H
#define QGRPCGENERATOR_H

#include "generatorbase.h"
#include <memory>
#include <set>

namespace google::protobuf {
class FileDescriptor;
class Descriptor;
namespace compiler {
class GeneratorContext;
} // namespace compiler
namespace io {
class Printer;
}
} // namespace google::protobuf

namespace QtGrpc {
class QGrpcGenerator : public qtprotoccommon::GeneratorBase
{
public:
    QGrpcGenerator();
    ~QGrpcGenerator();
    bool Generate(const ::google::protobuf::FileDescriptor *file,
                  const std::string &parameter,
                  ::google::protobuf::compiler::GeneratorContext *generatorContext,
                  std::string *error) const override;
    bool GenerateAll(const std::vector<const ::google::protobuf::FileDescriptor *> &files,
                     const std::string &parameter,
                     ::google::protobuf::compiler::GeneratorContext *generatorContext,
                     std::string *error) const override;
private:
    bool GenerateClientServices(
            const ::google::protobuf::FileDescriptor *file,
            ::google::protobuf::compiler::GeneratorContext *generatorContext) const;

    void GenerateQmlClientServices(
            const ::google::protobuf::FileDescriptor *file,
            ::google::protobuf::compiler::GeneratorContext *generatorContext) const;

    static std::set<std::string> GetInternalIncludes(
            const ::google::protobuf::FileDescriptor *file);

    template <typename ServicePrinterT>
    void RunPrinter(const ::google::protobuf::FileDescriptor *file,
                    std::shared_ptr<::google::protobuf::io::Printer> printer) const;
};
} // namespace QtGrpc

#endif // QGRPCGENERATOR_H
