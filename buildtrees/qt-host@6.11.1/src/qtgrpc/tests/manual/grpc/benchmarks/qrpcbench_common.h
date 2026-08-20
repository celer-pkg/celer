// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QRPCBENCH_COMMON_H
#define QRPCBENCH_COMMON_H

#if defined QTGRPCCLIENT
#  include "google/protobuf/timestamp.qpb.h"
#else
#  include <google/protobuf/timestamp.pb.h>
#endif

#include <QtCore/qcommandlineoption.h>
#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcompilerdetection.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qsysinfo.h>

#include <algorithm>
#include <chrono>
#include <concepts>
#include <cstdlib>
#include <format>
#include <iomanip>
#include <iostream>
#include <random>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace Bench {

inline std::string getTransportAddress(QAnyStringView transport)
{
    if (transport == "http")
        return "localhost:65002";
    if (transport == "https")
        return "localhost:65003";

#ifndef Q_OS_WINDOWS
    if (transport == "unix") {
#  if defined(Q_OS_LINUX)
        return "unix-abstract:bench";
#  elif defined(Q_OS_BSD4)
        return "unix:///tmp/bench.sock";
#  else
        std::cerr << "Unix transport not supported on this platform!" << std::endl;
        std::exit(EXIT_FAILURE);
#  endif
    }
#endif

    std::cerr << "Invalid transport specified: " << transport.toString().toStdString() << std::endl;
    std::exit(EXIT_FAILURE);
}

// Valid for the next 100 years.
static constexpr std::string_view SslCert = R"(
-----BEGIN CERTIFICATE-----
MIID+jCCAuKgAwIBAgIUVJNsgxX2GVzFdU8yC0xwETtnHWwwDQYJKoZIhvcNAQEL
BQAwgZMxCzAJBgNVBAYTAkRFMQ8wDQYDVQQIDAZCZXJsaW4xDzANBgNVBAcMBkJl
cmxpbjEcMBoGA1UECgwTVGhlIFF0IENvbXBhbnkgR21iSDEMMAoGA1UECwwDUiZE
MRIwEAYDVQQDDAlsb2NhbGhvc3QxIjAgBgkqhkiG9w0BCQEWE2Rlbm5pcy5vYmVy
c3RAcXQuaW8wIBcNMjUwMTE2MTU1ODIzWhgPMjEyNDEyMjMxNTU4MjNaMIGTMQsw
CQYDVQQGEwJERTEPMA0GA1UECAwGQmVybGluMQ8wDQYDVQQHDAZCZXJsaW4xHDAa
BgNVBAoME1RoZSBRdCBDb21wYW55IEdtYkgxDDAKBgNVBAsMA1ImRDESMBAGA1UE
AwwJbG9jYWxob3N0MSIwIAYJKoZIhvcNAQkBFhNkZW5uaXMub2JlcnN0QHF0Lmlv
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAszG9ps195LVF9kUDbQXj
1/5JAemlRvLMAJcr28virbqcCqphnode/vYGYiy2pbnPMJKxN9MsH5NMZH1/sn1A
PHGQ3sD6vcp24BZkgitDXLQab3QCF00K3Kka7GO6wQ9eloobBTtAWZDU/Px2LkLx
bmlIsr9Lbic2ZbKVWL3oRUECIAppmjEn1La+QSsV5lI2vROy6/b84OrBcYRDHifp
q70ZgIGO7AcucIeRAJGpfJJHxx8cuguDf0bgXJjsnIfXiAXCQkwtTWCrvES9akP4
dIJUz8onKAe7TRS1jaNGnXpikezhRa++GJNJhFAY3m6/0+qVHa0wByk5lxRoWV40
xwIDAQABo0IwQDAdBgNVHQ4EFgQUd2BWbXCVfYO0kHX+Ea5DcIzUlWIwHwYDVR0j
BBgwFoAU8jBTKqrkQqH2H7Kgyt6jtJtlZgcwDQYJKoZIhvcNAQELBQADggEBAG2f
xwaTuwBQ7ldk/8u4vfu1H8cq/eMLnmrbUm+QaSMdK1LXlhPQPpFcGN+6no/0tUA/
LhhzVzK6SRUgrm+IoGwL8ojyxdO760LgUOHN/VpvxQDY6DmBLct2L1sIRrjMNLym
ZgkXSmmeoswNgeonOwN8hIJz9Kx6xil1mPASGHvUaBFBdWDFxB/RuEYyv7X1wPeX
G2WqJZ+hoJESgtJFoybcbvqRGYq+ftXY8phacC1IuoaBinEBFxPrOkC2pzSHrvx2
Awm0fIbfSml23kefPh19Kquhf4NPdyUJo+RHuNn86tadANOEVVQ5slzaSuVu86Ym
u2A5Ea7yqvthhAzHrbg=
-----END CERTIFICATE-----
)";

static constexpr std::string_view SslKey = R"(
-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCzMb2mzX3ktUX2
RQNtBePX/kkB6aVG8swAlyvby+KtupwKqmGeh17+9gZiLLaluc8wkrE30ywfk0xk
fX+yfUA8cZDewPq9ynbgFmSCK0NctBpvdAIXTQrcqRrsY7rBD16WihsFO0BZkNT8
/HYuQvFuaUiyv0tuJzZlspVYvehFQQIgCmmaMSfUtr5BKxXmUja9E7Lr9vzg6sFx
hEMeJ+mrvRmAgY7sBy5wh5EAkal8kkfHHxy6C4N/RuBcmOych9eIBcJCTC1NYKu8
RL1qQ/h0glTPyicoB7tNFLWNo0ademKR7OFFr74Yk0mEUBjebr/T6pUdrTAHKTmX
FGhZXjTHAgMBAAECggEAN5khTNXJT+LmmCiFjZgcP3IIWO2TeFXw8eX1l7bE2D5k
F/MRYsyBrv3KsT9KVFU4ccux7K46rHlZZHyD2G+ANMDPwC2EHsro41JPUQv3VJYU
9au60lv3GMvnLJ0s3qXUJUUoaREfQCrtyqjSSjw/CJDmG3+6+ax09kzYhbY2kPW7
RfIWZFTq9n5x3e7Zd2wPDD7PUiqZy3TIHFOwxtpSD3TapVismIFiRtLA6nV84gKf
zKCOLNqzU0usgfHiLBWGXxKcFVjXiv6Nss0o+TFbKhVPgh9r22tEkSFbLp19x2Ii
jF1DU1M5oR1ew9uF22IAQDl13xgwQ/qAHqtQssAR0QKBgQDoelgfUKeFndypiDct
zMtd9AxXAmDl4HSA01eUH9m6yXw5PS3mtK14eInJtywq+PwIvCeJz8IkaioG+JVi
x7xrACEcxjud6Yq8MkPpi8omFSlGBzL0VnZfJpEXUdwlYMiyy5viYLiFPBm8Z94x
Putiob227E1WEooaPMo77Hd4rQKBgQDFUz3bHU5R0tI4bBiM5WlBRsB1gfk/Sc10
WQi9D9d5uFfdFn2bXBzeBtWR8PYB9RO+SMc+i81Vva1IAk/7XbMLYXtoH2rrC6Fz
8OarQARhTGG5gfnAV48YD2AJGH4uCt1snVoJ/bxDz15vNSR0ywzfuURVgvSLXkPe
51PGXz6NwwKBgQDZZf+eWSgvVW6SwyUGmWrcU2puu3Stw3ZvOjO9+wL7H4whYsrX
4cIO1HnVvot5LBlUec9nmnds4jKnDjN0il/yl85fQClkBI+OalsDvYuujT9pkzXd
NDXBySkJa6247ocAXFNMITKstYVDoMYxuysXszTcKKIxiWjIHGzqGLmoiQKBgQC8
kQO3dJX3k2PZD1OWsVSYUKhyorYxSLHR0ZOMOKtNYmB0op197dSYSCenw4ET9cPc
P2hH2QlsOkpxWeRc7fm/knR/2CYwX3j2duu4EwEcigWJZS/qIsJX17mKd6F9Flzr
AqOckKFsm6o+06X3BmNTGJS4suBGntp1FNL16ua4SQKBgC0Y+822m1CjxLMtU6rr
5OH7MSHC7szyo+yiuch3APOOg1jxqjx5Ru1dohLvzU2vLmbFIWezxKmLwRYFjnoH
xhA6h48p0zvZRJlFjOyQCRAT4w3Hm938g9o81F+QNFWI44USsX/htdaPOxqR0SkP
UKzk7CPqzQYR2xHUi+VcBI93
-----END PRIVATE KEY-----
)";

static constexpr std::string_view SslRootKey = R"(
-----BEGIN CERTIFICATE-----
MIIECzCCAvOgAwIBAgIUBraeFSR5B//R/M22CKLtFNwf6XswDQYJKoZIhvcNAQEL
BQAwgZMxCzAJBgNVBAYTAkRFMQ8wDQYDVQQIDAZCZXJsaW4xDzANBgNVBAcMBkJl
cmxpbjEcMBoGA1UECgwTVGhlIFF0IENvbXBhbnkgR21iSDEMMAoGA1UECwwDUiZE
MRIwEAYDVQQDDAlsb2NhbGhvc3QxIjAgBgkqhkiG9w0BCQEWE2Rlbm5pcy5vYmVy
c3RAcXQuaW8wIBcNMjUwMTE2MTU1ODIzWhgPMjEyNDEyMjMxNTU4MjNaMIGTMQsw
CQYDVQQGEwJERTEPMA0GA1UECAwGQmVybGluMQ8wDQYDVQQHDAZCZXJsaW4xHDAa
BgNVBAoME1RoZSBRdCBDb21wYW55IEdtYkgxDDAKBgNVBAsMA1ImRDESMBAGA1UE
AwwJbG9jYWxob3N0MSIwIAYJKoZIhvcNAQkBFhNkZW5uaXMub2JlcnN0QHF0Lmlv
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAq2n9SNDLpd59+wV2AgOy
KYLMCcZE719w67cnYDAdFyBsVS5/Us2AiQtPYbESNDRX8SxOLM0b9FfTrFH3EU70
EIuClNqHq9OhtPoRoPo7bDULlEqOQeCFSMOrY2OW1m5ngvSqrY7LyVViH3QHk02g
afnkuCdTbAkNNfNyHWCkyVrad7RMk28t2dptSaH4SFripiaO2f59l8//lZ/6bdze
m2HhP5wcx88X02JdskmqcVuOOSlE03UoWGKG2O5TuRY0M48piypSuLz5Ajz72Erg
g3yFh98DKojW60EepM6Tgcq5l3UNlY4rzXBzCV2X+EO/VzvVKSc9u2k12cJmHZxh
lwIDAQABo1MwUTAdBgNVHQ4EFgQU8jBTKqrkQqH2H7Kgyt6jtJtlZgcwHwYDVR0j
BBgwFoAU8jBTKqrkQqH2H7Kgyt6jtJtlZgcwDwYDVR0TAQH/BAUwAwEB/zANBgkq
hkiG9w0BAQsFAAOCAQEARZWMphTnpwhUKS5/Z52MZOBTeqQgZ6vJ8SfRi1LIoDw/
tVrJQm1yLEvwZUY3NKGSOy6Rseuz68L6VOj9RHVc9xD5e8Si3gh/3Yyhjdv2g+1V
fJNaMEqvVA4sGnkXk21kbnloYaVpojyyBU5nBEivvFoSyYhlT3V+6gWqfFI6+F/o
FW9KC07WXcLcM41sQN7DsOx5q+6Qfc3VYl4z+k6X8rDEe3s1zFXZu2nPgYHD/New
9/h7mAQbil06MwuPOYbousc1uY95EVRLN4XrKHDkm3TxM8wHDipoanwyPnOqcpTy
Esh/Nj3INbL1yljh4Fi447mtfgg+ERh+BgAv61ZwsA==
-----END CERTIFICATE-----
)";

struct BenchmarkData
{
    explicit BenchmarkData(uint64_t expected)
    {
        requestLatenciesNanos.reserve(expected);
        responseLatenciesNanos.reserve(expected);
    }

    int64_t callCount = {};
    std::vector<uint64_t> requestLatenciesNanos;
    std::vector<uint64_t> responseLatenciesNanos;
    uint64_t receivedBytes = {};
    uint64_t sendBytes = {};
    int64_t elapsedNanos = {};
};

inline google::protobuf::Timestamp getTimestamp()
{
    google::protobuf::Timestamp ts;
    auto now = std::chrono::system_clock::now();
    auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    auto nanos = static_cast<
        int32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(now - seconds).count());
#if defined QTGRPCCLIENT
    ts.setSeconds(seconds.time_since_epoch().count());
    ts.setNanos(nanos);
#else
    ts.set_seconds(seconds.time_since_epoch().count());
    ts.set_nanos(nanos);
#endif
    return ts;
}

[[maybe_unused]] inline int64_t calculateLatencyNanos(const google::protobuf::Timestamp &start,
                                                      const google::protobuf::Timestamp &end)
{
    int64_t startNanos = (start.seconds() * 1'000'000'000LL) + start.nanos();
    int64_t endNanos = (end.seconds() * 1'000'000'000LL) + end.nanos();
    return endNanos - startNanos;
}

[[maybe_unused]] inline int64_t calculateLatencyNanosNow(const google::protobuf::Timestamp &start)
{
    int64_t startNanos = (start.seconds() * 1'000'000'000LL) + start.nanos();
    int64_t endNanos = std::chrono::time_point_cast<
                           std::chrono::nanoseconds>(std::chrono::system_clock::now())
                           .time_since_epoch()
                           .count();
    return endNanos - startNanos;
}

template <typename Buffer>
[[nodiscard]] inline std::vector<Buffer> generatePayloads(std::size_t size, std::size_t count = 8)
{
    std::mt19937_64 rng(42);
    std::vector<Buffer> payloads(count);
    for (auto &buf : payloads) {
        buf.resize(size);
        std::generate(buf.begin(), buf.end(),
                      [&] { return static_cast<typename Buffer::value_type>(rng()); });
    }
    return payloads;
}

[[nodiscard]] inline const auto &nextPayload(const auto &payloads) noexcept
{
    static std::size_t cursor{ 0 };
    return payloads[(cursor += 1) % payloads.size()];
}

} // namespace Bench

namespace Client {

template <typename T>
concept ClientConcept = requires(T t) {
    { T(std::string(), uint64_t()) };
    { t.unaryCall() } -> std::same_as<void>;
    { t.serverStreaming() } -> std::same_as<void>;
    { t.clientStreaming() } -> std::same_as<void>;
    { t.bidiStreaming() } -> std::same_as<void>;
};

template <ClientConcept C>
inline void benchmarkMain(std::string_view name, int argc, char *argv[])
{
    QStringList args;
    for (int i = 0; i < argc; ++i)
        args.push_back(argv[i]);

    QCommandLineParser parser;
    parser.setApplicationDescription(name.data());
    parser.addHelpOption();

    QCommandLineOption transport({ "t", "transport" }, "Use Transport", "http|https", "http");
#ifndef Q_OS_WINDOWS
    transport.setValueName("http|https|unix");
#endif

    QCommandLineOption calls({ "c", "calls" }, "Amount of calls made.", "size", "1000");
    QCommandLineOption payload({ "p", "payload" }, "Payload size in bytes", "size", "0");
    QCommandLineOption uniqueRpc({ "u", "unique" }, "Make each RPC on a fresh client");

    QCommandLineOption enableUnary("U", "Enable UnaryCalls");
    QCommandLineOption enableSStream("S", "Enable ServerStream");
    QCommandLineOption enableCStream("C", "Enable ClientStream");
    QCommandLineOption enableBStream("B", "Enable BiDiStream");

    parser.addOptions({
        transport,

        calls,
        payload,
        uniqueRpc,

        enableUnary,
        enableSStream,
        enableCStream,
        enableBStream,
    });

    parser.process(args);

    bool defaultRun = !parser.isSet(enableUnary) && !parser.isSet(enableSStream)
        && !parser.isSet(enableCStream) && !parser.isSet(enableBStream);
    uint64_t amountCalls = parser.value(calls).toULong();
    qsizetype payloadSize = parser.value(payload).toLong();
    const auto transportValue = parser.value(transport).toStdString();

    std::cout << std::format("#### Start of {} benchmark ####\n", name);
    std::cout << std::format("  cpu-arch: {}\n", QSysInfo::currentCpuArchitecture().toStdString());
    std::cout << std::format("  kernel: {}, {}\n", QSysInfo::kernelType().toStdString(),
                             QSysInfo::kernelVersion().toStdString());
    std::cout << std::format("  host URI: {}, {}\n\n", transportValue,
                             Bench::getTransportAddress(transportValue));

    if (parser.isSet(payload))
        std::cout << std::format("  Option: payload per message {} bytes\n", payloadSize);
    if (parser.isSet(uniqueRpc))
        std::cout << std::format("  Option: unique client per RPC {}\n", parser.isSet(uniqueRpc));

    if (parser.isSet(uniqueRpc)) {
        {
            C client(transportValue, amountCalls, payloadSize);
            if (defaultRun || parser.isSet(enableUnary))
                client.unaryCall();
        }
        {
            C client(transportValue, amountCalls, payloadSize);
            if (defaultRun || parser.isSet(enableSStream))
                client.serverStreaming();
        }
        {
            C client(transportValue, amountCalls, payloadSize);
            if (defaultRun || parser.isSet(enableCStream))
                client.clientStreaming();
        }
        {
            C client(transportValue, amountCalls, payloadSize);
            if (defaultRun || parser.isSet(enableBStream))
                client.bidiStreaming();
        }
    } else {
        C client(transportValue, amountCalls, payloadSize);
        if (defaultRun || parser.isSet(enableUnary))
            client.unaryCall();
        if (defaultRun || parser.isSet(enableSStream))
            client.serverStreaming();
        if (defaultRun || parser.isSet(enableCStream))
            client.clientStreaming();
        if (defaultRun || parser.isSet(enableBStream))
            client.bidiStreaming();
    }

    std::cout << std::format("\n#### End of {} benchmark ####\n", name);
}

inline std::string formatTime(uint64_t ns)
{
    std::ostringstream oss;
    if (ns < 1000) {
        oss << ns << " ns";
    } else if (ns < 1000000) { // less than 1e6 ns -> microseconds
        oss << std::fixed << std::setprecision(2) << (ns / 1000.0) << " us";
    } else if (ns < 1000000000) { // less than 1e9 ns -> milliseconds
        oss << std::fixed << std::setprecision(2) << (ns / 1000000.0) << " ms";
    } else { // one second or more -> seconds
        oss << std::fixed << std::setprecision(2) << (ns / 1000000000.0) << " s";
    }
    return oss.str();
}

inline std::string formatBytes(double bytes, uint8_t p)
{
    std::ostringstream oss;
    constexpr uint32_t D = 1000;
    if (bytes < D) {
        oss << std::fixed << std::setprecision(p) << bytes << " B";
    } else if (bytes < D * D) {
        oss << std::fixed << std::setprecision(p) << (bytes / D) << " KB";
    } else if (bytes < D * D * D) {
        oss << std::fixed << std::setprecision(p) << (bytes / (D * D)) << " MB";
    } else {
        oss << std::fixed << std::setprecision(p) << (bytes / (D * D * D)) << " GB";
    }
    return oss.str();
}

inline void printLatencyStats(const std::vector<uint64_t> &latencies, const std::string &label)
{
    if (latencies.empty())
        return;

    std::vector<uint64_t> sorted = latencies;
    std::sort(sorted.begin(), sorted.end());
    size_t n = sorted.size();

    double sum = std::accumulate(sorted.begin(), sorted.end(), 0.0);
    double mean = sum / n;

    // Compute the p-th percentile of latency
    auto percentile = [n, &sorted](double p) -> uint64_t {
        size_t idx = (n == 0) ? 0 : (static_cast<size_t>(std::ceil(p * n)) - 1);
        if (idx >= n)
            idx = n - 1;
        return sorted[idx];
    };

    uint64_t p80 = percentile(0.80);
    uint64_t p95 = percentile(0.95);
    uint64_t minVal = sorted.front();
    uint64_t maxVal = sorted.back();

    double variance = std::accumulate(sorted.begin(), sorted.end(), 0.0,
                                      [mean](double acc, uint64_t val) {
                                          double diff = static_cast<double>(val) - mean;
                                          return acc + diff * diff;
                                      })
        / n;
    double stddev = std::sqrt(variance);

    std::vector<std::string> l = {
        formatTime(minVal), formatTime(static_cast<uint64_t>(mean)),
        formatTime(maxVal), formatTime(p80),
        formatTime(p95),    formatTime(static_cast<uint64_t>(stddev)),
    };
    constexpr int keyWidth = 6;
    auto valWidth = std::ranges::max(l | std::views::transform(&std::string::size)) + 1;
    std::cout << label << " Latencies:\n";
    std::cout << std::format("  {:<{}}: {:<{}}| {:<{}}: {:<{}}| {:<{}}: {:<{}}\n", "Min", keyWidth,
                             l[0], valWidth, "Mean", keyWidth, l[1], valWidth, "Max", keyWidth,
                             l[2], valWidth);
    std::cout << std::format("  {:<{}}: {:<{}}| {:<{}}: {:<{}}| {:<{}}: {:<{}}\n", "P80", keyWidth,
                             l[3], valWidth, "P95", keyWidth, l[4], valWidth, "StdDev", keyWidth,
                             l[5], valWidth);
}

inline void printBenchmarkResult(const std::string &title, const Bench::BenchmarkData &data)
{
    assert(data.callCount > 0);
    std::string titleString = "========== Benchmark Results: " + title + " ==========";
    std::cout << std::format("\n{}\n", titleString);

    std::cout << "Calls: " << data.callCount << "\n";

    auto avgCallTime = data.elapsedNanos / data.callCount;
    std::cout << "Total Time: " << formatTime(data.elapsedNanos);
    std::cout << " | Avg per call: " << formatTime(avgCallTime);
    std::cout << "\n";

    printLatencyStats(data.requestLatenciesNanos, "Request  (Client → Server)");
    printLatencyStats(data.responseLatenciesNanos, "Response (Server → Client)");

    if (data.sendBytes > 0 || data.receivedBytes > 0) {
        std::cout << "Total Sent: " << formatBytes(double(data.sendBytes), 0)
                  << " | Total Recv: " << formatBytes(double(data.receivedBytes), 0) << '\n';
    }

    double totalTimeSec = double(data.elapsedNanos) / 1e9;
    double totalBytes = static_cast<double>(data.sendBytes + data.receivedBytes);
    double throughput = (totalTimeSec > 0) ? (totalBytes / totalTimeSec) : 0.0;
    if (throughput > 0.0)
        std::cout << "Throughput: " << formatBytes(throughput, 2) << "/s\n";
    double qps = totalTimeSec > 0.0 ? (data.callCount / totalTimeSec) : 0.0;
    std::cout << std::format("QPS: {:.0f}{}\n", qps > 10000.0 ? qps / 1'000 : qps,
                             qps > 10'000 ? " k" : "");

    std::cout << std::string(titleString.size(), '=') << '\n';
}

inline void printRpcResult(std::string benched, int64_t elapsedNs, uint64_t amountCalls,
                           uint64_t recvBytes = 0, uint64_t sendBytes = 0)
{
    using namespace std::chrono;

    std::cout << std::format("Finished benchmark: {}\n", benched);
    const auto ns = nanoseconds(elapsedNs);
    const auto us = duration_cast<microseconds>(ns);
    std::cout << std::format("  Completed calls: {}\n", amountCalls);
    std::cout << std::format("  Total time: {}, {}\n", us, ns);
    const auto avgNs = nanoseconds(ns / amountCalls);
    const auto avgUs = duration_cast<microseconds>(avgNs);
    std::cout << std::format("  Average time: {}, {}\n", avgUs, avgNs);
    if (recvBytes > 0 && sendBytes > 0) {
        std::cout << std::format("  Send bytes: {}, Recv bytes: {}\n", sendBytes, recvBytes);
        const auto sec = duration_cast<duration<double>>(ns);
        double totalTimeSec = sec.count();
        double totalBytes = static_cast<double>(recvBytes + sendBytes);
        double throughputKBPerSec = (totalBytes / 1024) / totalTimeSec;
        std::cout << std::format("  Throughput (KB/sec): {:.2f}\n", throughputKBPerSec);
    }
}

} // namespace Client

#endif // QRPCBENCH_COMMON_H
