// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <google/protobuf/compiler/plugin.h>

#include "qprotobufgenerator.h"
#include "options.h"
#include "utils.h"

#include <iostream>
#include <cstdlib>

using namespace ::QtProtobuf;
int main(int argc, char *argv[])
{
    char *optionsPtr = getenv("QT_PROTOBUF_OPTIONS");
    if (optionsPtr != nullptr) {
        QT_PROTOBUF_DEBUG("QT_PROTOBUF_OPTIONS: " << optionsPtr);
        std::string error;
        qtprotoccommon::Options::setFromString(optionsPtr, qtprotoccommon::Options::QtProtobufGen,
                                               &error);
        if (!error.empty()) {
            std::cerr << error << std::endl;
            return EXIT_FAILURE;
        }
    }
    QProtobufGenerator generator;
    return ::google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
