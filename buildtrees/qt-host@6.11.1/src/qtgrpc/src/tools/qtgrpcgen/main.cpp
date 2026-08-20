// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <google/protobuf/compiler/plugin.h>

#include "qgrpcgenerator.h"
#include "options.h"
#include "utils.h"

#include <iostream>
#include <cstdlib>

using namespace QtGrpc;

int main(int argc, char *argv[])
{
    char *optionsPtr = getenv("QT_GRPC_OPTIONS");
    if (optionsPtr != nullptr) {
        QT_PROTOBUF_DEBUG("QT_GRPC_OPTIONS: " << optionsPtr);
        std::string error;
        qtprotoccommon::Options::setFromString(optionsPtr, qtprotoccommon::Options::QtGrpcGen,
                                               &error);
        if (!error.empty()) {
            std::cerr << error << std::endl;
            return EXIT_FAILURE;
        }
    }
    QGrpcGenerator generator;
    return ::google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
