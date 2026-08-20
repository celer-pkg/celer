// Copyright (C) 2024 The Qt Company Ltd.
// Copyright (C) 2024 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtTest/private/qtestcrashhandler_p.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qstring.h>

#include <QtTest/qtestcase.h>
#include <QtTest/private/qtestlog_p.h>

#include <stdio.h>
#include <stdlib.h>

#if !defined(Q_CC_MINGW) || (defined(Q_CC_MINGW) && defined(__MINGW64_VERSION_MAJOR))
#  include <crtdbg.h>
#endif
#include <qt_windows.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace QTest {
namespace CrashHandler {
bool alreadyDebugging()
{
    return IsDebuggerPresent();
}

void maybeDisableCoreDump()
{
}

static DebuggerProgram debugger = None;
void prepareStackTrace()
{

    bool ok = false;
    const int disableStackDump = qEnvironmentVariableIntValue("QTEST_DISABLE_STACK_DUMP", &ok);
    if (ok && disableStackDump)
        return;

    // ### Implement finding a debugger on Windows
}

void printTestRunTime()
{
    const int msecsFunctionTime = qRound(QTestLog::msecsFunctionTime());
    const int msecsTotalTime = qRound(QTestLog::msecsTotalTime());
    const char *const name = QTest::currentTestFunction();

    // Windows doesn't have the concept of async-safety, so fprintf() is
    // probably as good as WriteFile() and WriteConsole().
    fprintf(stderr, "\n         %s function time: %dms, total time: %dms\n",
            name ? name : "[Non-test]", msecsFunctionTime, msecsTotalTime);
}

void generateStackTrace(quintptr ip)
{
    if (debugger == None || alreadyDebugging())
        return;

    // ### Implement starting a debugger on Windows
    Q_UNUSED(ip);
}

void blockUnixSignals()
{
  // Windows does have C signals, but doesn't use them for the purposes we're
  // talking about here
}

namespace {
// Helper class for resolving symbol names by dynamically loading "dbghelp.dll".
class DebugSymbolResolver
{
    Q_DISABLE_COPY_MOVE(DebugSymbolResolver)
public:
    struct Symbol
    {
        Symbol() : name(nullptr), address(0) {}

        const char *name; // Must be freed by caller.
        DWORD64 address;
    };

    explicit DebugSymbolResolver(HANDLE process);
    ~DebugSymbolResolver() { cleanup(); }

    bool isValid() const { return m_symFromAddr; }

    Symbol resolveSymbol(DWORD64 address) const;

private:
    // typedefs from DbgHelp.h/.dll
    struct DBGHELP_SYMBOL_INFO { // SYMBOL_INFO
        ULONG       SizeOfStruct;
        ULONG       TypeIndex;        // Type Index of symbol
        ULONG64     Reserved[2];
        ULONG       Index;
        ULONG       Size;
        ULONG64     ModBase;          // Base Address of module comtaining this symbol
        ULONG       Flags;
        ULONG64     Value;            // Value of symbol, ValuePresent should be 1
        ULONG64     Address;          // Address of symbol including base address of module
        ULONG       Register;         // register holding value or pointer to value
        ULONG       Scope;            // scope of the symbol
        ULONG       Tag;              // pdb classification
        ULONG       NameLen;          // Actual length of name
        ULONG       MaxNameLen;
        CHAR        Name[1];          // Name of symbol
    };

    typedef BOOL (__stdcall *SymInitializeType)(HANDLE, PCSTR, BOOL);
    typedef BOOL (__stdcall *SymFromAddrType)(HANDLE, DWORD64, PDWORD64, DBGHELP_SYMBOL_INFO *);

    void cleanup();

    const HANDLE m_process;
    HMODULE m_dbgHelpLib;
    SymFromAddrType m_symFromAddr;
};

void DebugSymbolResolver::cleanup()
{
    if (m_dbgHelpLib)
        FreeLibrary(m_dbgHelpLib);
    m_dbgHelpLib = 0;
    m_symFromAddr = nullptr;
}

DebugSymbolResolver::DebugSymbolResolver(HANDLE process)
    : m_process(process), m_dbgHelpLib(0), m_symFromAddr(nullptr)
{
    bool success = false;
    m_dbgHelpLib = LoadLibraryW(L"dbghelp.dll");
    if (m_dbgHelpLib) {
        SymInitializeType symInitialize = reinterpret_cast<SymInitializeType>(
            reinterpret_cast<QFunctionPointer>(GetProcAddress(m_dbgHelpLib, "SymInitialize")));
        m_symFromAddr = reinterpret_cast<SymFromAddrType>(
            reinterpret_cast<QFunctionPointer>(GetProcAddress(m_dbgHelpLib, "SymFromAddr")));
        success = symInitialize && m_symFromAddr && symInitialize(process, NULL, TRUE);
    }
    if (!success)
        cleanup();
}

DebugSymbolResolver::Symbol DebugSymbolResolver::resolveSymbol(DWORD64 address) const
{
    // reserve additional buffer where SymFromAddr() will store the name
    struct NamedSymbolInfo : public DBGHELP_SYMBOL_INFO {
        enum { symbolNameLength = 255 };

        char name[symbolNameLength + 1];
    };

    Symbol result;
    if (!isValid())
        return result;
    NamedSymbolInfo symbolBuffer;
    memset(&symbolBuffer, 0, sizeof(NamedSymbolInfo));
    symbolBuffer.MaxNameLen = NamedSymbolInfo::symbolNameLength;
    symbolBuffer.SizeOfStruct = sizeof(DBGHELP_SYMBOL_INFO);
    if (!m_symFromAddr(m_process, address, 0, &symbolBuffer))
        return result;
    result.name = qstrdup(symbolBuffer.Name);
    result.address = symbolBuffer.Address;
    return result;
}
} // unnamed namespace

static LONG WINAPI windowsFaultHandler(struct _EXCEPTION_POINTERS *exInfo);

WindowsFaultHandler::WindowsFaultHandler()
{
#  if !defined(Q_CC_MINGW)
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#  endif
    SetErrorMode(SetErrorMode(0) | SEM_NOGPFAULTERRORBOX);
    SetUnhandledExceptionFilter(windowsFaultHandler);
}

static LONG WINAPI windowsFaultHandler(struct _EXCEPTION_POINTERS *exInfo)
{
    enum { maxStackFrames = 100 };
    char appName[MAX_PATH];
    if (!GetModuleFileNameA(NULL, appName, MAX_PATH))
        appName[0] = 0;
    const int msecsFunctionTime = qRound(QTestLog::msecsFunctionTime());
    const int msecsTotalTime = qRound(QTestLog::msecsTotalTime());
    const void *exceptionAddress = exInfo->ExceptionRecord->ExceptionAddress;
    fprintf(stderr, "A crash occurred in %s.\n", appName);
    if (const char *name = QTest::currentTestFunction())
        fprintf(stderr, "While testing %s\n", name);
    fprintf(stderr, "Function time: %dms Total time: %dms\n\n"
                    "Exception address: 0x%p\n"
                    "Exception code   : 0x%lx\n",
            msecsFunctionTime, msecsTotalTime, exceptionAddress,
            exInfo->ExceptionRecord->ExceptionCode);

    DebugSymbolResolver resolver(GetCurrentProcess());
    if (resolver.isValid()) {
        DebugSymbolResolver::Symbol exceptionSymbol = resolver.resolveSymbol(DWORD64(exceptionAddress));
        if (exceptionSymbol.name) {
            fprintf(stderr, "Nearby symbol    : %s\n", exceptionSymbol.name);
            delete [] exceptionSymbol.name;
        }
        Q_DECL_UNINITIALIZED void *stack[maxStackFrames];
        fputs("\nStack:\n", stderr);
        const unsigned frameCount = CaptureStackBackTrace(0, DWORD(maxStackFrames), stack, NULL);
        for (unsigned f = 0; f < frameCount; ++f)     {
            DebugSymbolResolver::Symbol symbol = resolver.resolveSymbol(DWORD64(stack[f]));
            if (symbol.name) {
                fprintf(stderr, "#%3u: %s() - 0x%p\n", f + 1, symbol.name, (const void *)symbol.address);
                delete [] symbol.name;
            } else {
                fprintf(stderr, "#%3u: Unable to obtain symbol\n", f + 1);
            }
        }
    }

    fputc('\n', stderr);

    return EXCEPTION_EXECUTE_HANDLER;
}
} // namespace CrashHandler
} // namespace QTest

QT_END_NAMESPACE
