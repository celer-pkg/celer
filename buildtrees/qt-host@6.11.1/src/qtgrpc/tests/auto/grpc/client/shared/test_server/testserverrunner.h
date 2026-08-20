// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TEST_SERVER_H
#define TEST_SERVER_H

#include <QtTypes>

class TestServer
{
public:
    void run(qint64 latency);
};

#endif // TEST_SERVER_H
