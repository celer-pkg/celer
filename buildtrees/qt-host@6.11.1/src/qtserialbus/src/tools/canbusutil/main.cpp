// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:significant reason:default

#include "canbusutil.h"
#include "sigtermhandler.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>

#include <signal.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("canbusutil"));
    QCoreApplication::setApplicationVersion(QStringLiteral(QT_VERSION_STR));

    std::unique_ptr<SigTermHandler> s(SigTermHandler::instance());
    if (signal(SIGINT, SigTermHandler::handle) == SIG_ERR)
        return -1;
    QObject::connect(s.get(), &SigTermHandler::sigTermSignal, &app, &QCoreApplication::quit);

    QTextStream output(stdout);
    CanBusUtil util(output, app);

    QCommandLineParser parser;
    parser.setApplicationDescription(CanBusUtil::tr(
        "Sends arbitrary CAN bus frames.\n"
        "If the -l option is set, all received CAN bus frames are dumped."));
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument(QStringLiteral("plugin"),
            CanBusUtil::tr("Plugin name to use. See --list-plugins."));

    parser.addPositionalArgument(QStringLiteral("device"),
            CanBusUtil::tr("Device to use."));

    parser.addPositionalArgument(QStringLiteral("data"),
            CanBusUtil::tr(
                "Data to send if -l is not specified. Format:\n"
                "\t\t<id>#{payload}          (CAN 2.0 data frames),\n"
                "\t\t<id>#Rxx                (CAN 2.0 RTR frames with xx bytes data length),\n"
                "\t\t<id>##[flags]{payload}  (CAN FD data frames),\n"
                "where {payload} has 0..8 (0..64 CAN FD) ASCII hex-value pairs, "
                "and flags is one optional ASCII hex char for CAN FD flags: "
                "1 = Bitrate Switch, 2 = Error State Indicator\n"
                "e.g. 1#1a2b3c\n"), QStringLiteral("[data]"));

    const QCommandLineOption listeningOption({"l", "listen"},
            CanBusUtil::tr("Start listening CAN data on device."));
    parser.addOption(listeningOption);

    const QCommandLineOption listOption({"L", "list-plugins"},
            CanBusUtil::tr("List all available plugins."));
    parser.addOption(listOption);

    const QCommandLineOption showTimeStampOption({"t", "timestamp"},
            CanBusUtil::tr("Show timestamp for each received CAN bus frame."));
    parser.addOption(showTimeStampOption);

    const QCommandLineOption showFlagsOption({"i", "info"},
            CanBusUtil::tr("Show flags bitrate switch, error indicator, and local echo"
                           " for each received CAN bus frame."));
    parser.addOption(showFlagsOption);

    const QCommandLineOption listDevicesOption({"d", "devices"},
            CanBusUtil::tr("Show available CAN bus devices for the given plugin."));
    parser.addOption(listDevicesOption);

    const QCommandLineOption canFdOption({"f", "can-fd"},
            CanBusUtil::tr("Enable CAN FD functionality when listening."));
    parser.addOption(canFdOption);

    const QCommandLineOption loopbackOption({"c", "local-loopback"},
            CanBusUtil::tr("Transmits all sent frames to other local applications."));
    parser.addOption(loopbackOption);

    const QCommandLineOption receiveOwnOption({"o", "receive-own"},
            CanBusUtil::tr("Receive each sent frame on successful transmission."));
    parser.addOption(receiveOwnOption);

    const QCommandLineOption bitrateOption({"b", "bitrate"},
            CanBusUtil::tr("Set the CAN bus bitrate to the given value."),
            QStringLiteral("bitrate"));
    parser.addOption(bitrateOption);

    const QCommandLineOption dataBitrateOption({"a", "data-bitrate"},
            CanBusUtil::tr("Set the CAN FD data bitrate to the given value."),
            QStringLiteral("bitrate"));
    parser.addOption(dataBitrateOption);

    parser.process(app);

    if (parser.isSet(listOption))
        return util.printPlugins();

    QString data;
    const QStringList args = parser.positionalArguments();

    if (parser.isSet(canFdOption))
        util.setConfigurationParameter(QCanBusDevice::CanFdKey, true);
    if (parser.isSet(loopbackOption))
        util.setConfigurationParameter(QCanBusDevice::LoopbackKey, true);
    if (parser.isSet(receiveOwnOption))
        util.setConfigurationParameter(QCanBusDevice::ReceiveOwnKey, true);
    if (!parser.value(bitrateOption).isEmpty()) {
        util.setConfigurationParameter(QCanBusDevice::BitRateKey,
                                       parser.value(bitrateOption).toInt());
    }
    if (!parser.value(dataBitrateOption).isEmpty()) {
        util.setConfigurationParameter(QCanBusDevice::DataBitRateKey,
                                       parser.value(dataBitrateOption).toInt());
    }

    if (parser.isSet(listeningOption)) {
        util.setShowTimeStamp(parser.isSet(showTimeStampOption));
        util.setShowFlags(parser.isSet(showFlagsOption));
    } else if (args.size() == 3) {
        data = args.at(2);
    } else if (args.size() == 1 && parser.isSet(listDevicesOption)) {
        return util.printDevices(args.at(0));
    }

    if (args.size() < 2 || args.size() > 3) {
        output << CanBusUtil::tr("Invalid number of arguments (%1 given).").arg(args.size());
        output << Qt::endl << Qt::endl << parser.helpText();
        return 1;
    }

    if (!util.start(args.at(0), args.at(1), data))
        return -1;

    return app.exec();
}
