// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QtGlobal>
#include <QtLottie/private/qlottieanimation_p.h>

class QLottieAnimationFuzzer : public QLottieAnimation
{
    Q_OBJECT
public:
    QLottieAnimationFuzzer(QQuickItem *parent = nullptr)
        : QLottieAnimation(parent)
    {
    }
    void fuzzParse(const QByteArray &jsonSource) { QLottieAnimation::parse(jsonSource); }
};

// silence warnings
static QtMessageHandler mh = qInstallMessageHandler([](QtMsgType, const QMessageLogContext &,
                                                       const QString &) {});

extern "C" int LLVMFuzzerTestOneInput(const char *Data, size_t Size) {
    static int argc = 3;
    static char arg1[] = "fuzzer";
    static char arg2[] = "-platform";
    static char arg3[] = "minimal";
    static char *argv[] = {arg1, arg2, arg3, nullptr};
    static QGuiApplication qga(argc, argv);
    QLottieAnimationFuzzer lottie;
    lottie.fuzzParse(QByteArray::fromRawData(Data, Size));
    return 0;
}

#include "main.moc"
