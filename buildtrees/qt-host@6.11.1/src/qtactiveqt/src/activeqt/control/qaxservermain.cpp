// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#include <qstringlist.h>
#include <qlist.h>
#include <private/qcoreapplication_p.h>

#include "qaxfactory.h"

#include <string.h>

#include <qt_windows.h>

#include <QtCore/private/qcomptr_p.h>

QT_BEGIN_NAMESPACE

static DWORD *classRegistration = nullptr;
static DWORD dwThreadID;
static bool qAxActivity = false;
static HANDLE hEventShutdown;

#ifdef QT_DEBUG
static const DWORD dwTimeOut = 1000;
static const DWORD dwPause = 500;
#else
static const DWORD dwTimeOut = 5000; // time for EXE to be idle before shutting down
static const DWORD dwPause = 1000; // time to wait for threads to finish up
#endif

extern bool qAxIsServer;
extern bool qAxOutProcServer;
extern wchar_t qAxModuleFilename[MAX_PATH];
extern QString qAxInit();
extern void qAxCleanup();
extern HRESULT UpdateRegistry(bool bRegister, bool perUser);
extern HRESULT GetClassObject(const GUID &clsid, const GUID &iid, void **ppUnk);
extern ulong qAxLockCount();
extern bool qax_winEventFilter(void *message);

STDAPI DumpIDL(const QString &outfile, const QString &ver);

// Monitors the shutdown event
static DWORD WINAPI MonitorProc(void* /* pv */)
{
    while (true) {
        WaitForSingleObject(hEventShutdown, INFINITE);
        DWORD dwWait=0;
        do {
            qAxActivity = false;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);
        } while (dwWait == WAIT_OBJECT_0);
        // timed out
        if (!qAxActivity && !qAxLockCount()) // if no activity let's really bail
            break;
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
    PostQuitMessage(0);

    return 0;
}

// Starts the monitoring thread
static bool StartMonitor()
{
    dwThreadID = GetCurrentThreadId();
    hEventShutdown = CreateEvent(nullptr, false, false, nullptr);
    if (hEventShutdown == nullptr)
        return false;
    DWORD dwThreadID;
    HANDLE h = CreateThread(nullptr, 0, MonitorProc, nullptr, 0, &dwThreadID);
    return (h != nullptr);
}

void qax_shutDown()
{
    qAxActivity = true;
    if (hEventShutdown)
        SetEvent(hEventShutdown); // tell monitor that we transitioned to zero
}

/*
    Start the COM server (if necessary).
*/
bool qax_startServer(QAxFactory::ServerType type)
{
    if (qAxIsServer)
        return true;

    const QStringList keys = qAxFactory()->featureList();
    const qsizetype keyCount = keys.size();
    if (!keyCount)
        return false;

    if (!qAxFactory()->isService())
        StartMonitor();

    classRegistration = new DWORD[keyCount];
    qsizetype object = 0;
    for (object = 0; object < keyCount; ++object) {
        ComPtr<IUnknown> p;
        CLSID clsid = qAxFactory()->classID(keys.at(object));

        // Create a QClassFactory (implemented in qaxserverbase.cpp)
        HRESULT hRes = GetClassObject(clsid, IID_IClassFactory, &p);
        if (SUCCEEDED(hRes))
            hRes = CoRegisterClassObject(clsid, p.Get(), CLSCTX_LOCAL_SERVER,
                type == QAxFactory::MultipleInstances ? REGCLS_MULTIPLEUSE : REGCLS_SINGLEUSE,
                classRegistration+object);
    }

    qAxIsServer = true;
    return true;
}

/*
    Stop the COM server (if necessary).
*/
bool qax_stopServer()
{
    if (!qAxIsServer || !classRegistration)
        return true;

    qAxIsServer = false;

    const int keyCount = qAxFactory()->featureList().size();
    for (int object = 0; object < keyCount; ++object)
        CoRevokeClassObject(classRegistration[object]);

    delete []classRegistration;
    classRegistration = nullptr;

    Sleep(dwPause); //wait for any threads to finish

    return true;
}

QT_END_NAMESPACE

#if defined(QT_NEEDS_QMAIN)
int qMain(int, char **);
#define main qMain
#else
extern "C" int main(int, char **);
#endif

static inline QStringList commandLineArguments()
{
    QStringList result;
    int size;
    if (wchar_t **argv = CommandLineToArgvW(GetCommandLine(), &size)) {
        result.reserve(size);
        wchar_t **argvEnd = argv + size;
        for (wchar_t **a = argv; a < argvEnd; ++a)
            result.append(QString::fromWCharArray(*a));
        LocalFree(argv);
    }
    return result;
}

static inline bool matchesOption(const QString &arg, const char *option)
{
    return (arg.startsWith(QLatin1Char('/')) || arg.startsWith(QLatin1Char('-')))
        && arg.right(arg.size() - 1).compare(QLatin1String(option), Qt::CaseInsensitive) == 0;
}

namespace {
struct Arg {
    explicit Arg(const QStringList &args)
    {
        argv.reserve(args.size() + 1);
        for (const QString &a : args)
            argv.append(_strdup(a.toLocal8Bit().constData()));
        argv.append(nullptr);
    }

    ~Arg()
    {
        // Note: QGuiApplication may shift the array when removing Qt arguments
        for (int i = 0, last = argv.size() - 1; i < last && argv.at(i) != nullptr; ++i)
            free(argv.at(i));
    }

    QList<char *> argv;
};
} // namespace

EXTERN_C int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /* hPrevInstance */, LPSTR, int /* nShowCmd */)
{
    QT_USE_NAMESPACE

    qAxOutProcServer = true;
    GetModuleFileName(nullptr, qAxModuleFilename, MAX_PATH);
    QCoreApplicationPrivate::mainInstanceHandle = hInstance;

    const QStringList cmds = commandLineArguments();
    QStringList unprocessed;
    unprocessed.reserve(cmds.size());

    int nRet = 0;
    bool run = true;
    bool runServer = false;
    for (qsizetype i = 0; i < cmds.size(); ++i) {
        const QString &cmd = cmds.at(i);
        if (matchesOption(cmd, "activex") || matchesOption(cmd, "embedding")) {
            runServer = true;
        } else if (matchesOption(cmd, "unregserver")) {
            nRet = UpdateRegistry(false, false);
            run = false;
            break;
        } else if (matchesOption(cmd, "regserver")) {
            nRet = UpdateRegistry(true, false);
            run = false;
            break;
        } else if (matchesOption(cmd, "unregserverperuser")) {
            nRet = UpdateRegistry(false, true);
            run = false;
            break;
        } else if (matchesOption(cmd, "regserverperuser")) {
            nRet = UpdateRegistry(true, true);
            run = false;
            break;
        } else if (matchesOption(cmd, "dumpidl")) {
            ++i;
            if (i < cmds.size()) {
                const QString &outfile = cmds.at(i);
                ++i;
                QString version;
                if (i < cmds.size() && matchesOption(cmds.at(i), "version")) {
                    ++i;
                    if (i < cmds.size())
                        version = cmds.at(i);
                    else
                        version = QStringLiteral("1.0");
                }

                nRet = DumpIDL(outfile, version);
            } else {
                qWarning("Wrong commandline syntax: <app> -dumpidl <idl file> [-version <x.y.z>]");
            }
            run = false;
            break;
        } else {
            unprocessed.append(cmds.at(i));
        }
    }

    if (run) {
        if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
#ifdef Q_CC_MINGW
            // define GlobalOptions class ID locally for MinGW, since it's missing from the distribution
            static const CLSID CLSID_GlobalOptions =
                { 0x0000034B, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
#endif
            // Disable C++ & SEH exception handling by the COM runtime for out-of-process COM controls.
            // Done to prevent silent crashes and enable crash dump generation.
            ComPtr<IGlobalOptions> globalOptions;
            if (SUCCEEDED(CoCreateInstance(CLSID_GlobalOptions, nullptr, CLSCTX_INPROC_SERVER,
                                           IID_IGlobalOptions, &globalOptions))) {
                globalOptions->Set(COMGLB_EXCEPTION_HANDLING, COMGLB_EXCEPTION_DONOT_HANDLE_ANY);
            }

            {
                Arg args(unprocessed);
                qAxInit();
                if (runServer)
                    QAxFactory::startServer();
                nRet = ::main(args.argv.size() - 1, args.argv.data());
                QAxFactory::stopServer();
                qAxCleanup();
            }
            CoUninitialize();
        } else {
            qErrnoWarning("CoInitializeEx() failed.");
            nRet = -1;
        }
    }

    return nRet;
}
