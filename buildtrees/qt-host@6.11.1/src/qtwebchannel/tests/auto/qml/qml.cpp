// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQuickTest/quicktest.h>
#include <QtQml/qqml.h>

#ifndef QUICK_TEST_SOURCE_DIR
#define QUICK_TEST_SOURCE_DIR nullptr
#endif

#include "testtransport.h"
#include "testwebchannel.h"
#include "testobject.h"

int main(int argc, char **argv)
{
    qmlRegisterType<TestTransport>("QtWebChannel.Tests", 1, 0, "TestTransport");
    qmlRegisterType<TestWebChannel>("QtWebChannel.Tests", 1, 0, "TestWebChannel");
    qmlRegisterType<TestObject>("QtWebChannel.Tests", 1, 0, "TestObject");

    return quick_test_main(argc, argv, "qml", QUICK_TEST_SOURCE_DIR);
}
