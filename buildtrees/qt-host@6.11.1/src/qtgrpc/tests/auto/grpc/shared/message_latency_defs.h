// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef MESSAGE_LATENCY_DEFS_H
#define MESSAGE_LATENCY_DEFS_H

#ifndef QT_GRPC_TEST_MESSAGE_LATENCY
#ifdef Q_OS_WINDOWS
#  define QT_GRPC_TEST_MESSAGE_LATENCY 1000
#else
#  define QT_GRPC_TEST_MESSAGE_LATENCY 300
#endif
#endif

constexpr int MessageLatency = QT_GRPC_TEST_MESSAGE_LATENCY;
#ifdef Q_OS_WINDOWS
constexpr int MessageLatencyThreshold = MessageLatency / 3; // we also need ~300 for WIN
#else
constexpr int MessageLatencyThreshold = MessageLatency / 5;
#endif
constexpr int MessageLatencyWithThreshold = MessageLatency + MessageLatencyThreshold;
constexpr int FailTimeout = QT_GRPC_TEST_MESSAGE_LATENCY * 5;

#endif // MESSAGE_LATENCY_DEFS_H
