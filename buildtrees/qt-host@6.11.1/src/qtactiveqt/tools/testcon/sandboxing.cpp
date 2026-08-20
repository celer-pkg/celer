// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "sandboxing.h"
#include <memory>
#include <windows.h>
#include <sddl.h>
#include <userenv.h>
#include <QProcess>
#include <QSettings>


/** RAII wrapper of STARTUPINFOEX. */
struct StartupInfoExWrap
{
    STARTUPINFOEX si = {};

    StartupInfoExWrap ()
    {
        si.StartupInfo.cb = sizeof(STARTUPINFOEX);

        const DWORD attr_count = 1; // SECURITY_CAPABILITIES
        SIZE_T attr_size = 0;
        InitializeProcThreadAttributeList(NULL, attr_count, 0, &attr_size);
        si.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)new BYTE[attr_size]();
        if (!InitializeProcThreadAttributeList(si.lpAttributeList, attr_count, 0, &attr_size))
            qFatal("InitializeProcThreadAttributeList failed");
    }

    void SetSecurity(SECURITY_CAPABILITIES *sc)
    {
        if (!UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES,
                                       sc, sizeof(SECURITY_CAPABILITIES), NULL, NULL))
            qFatal("UpdateProcThreadAttribute failed");
    }

    ~StartupInfoExWrap()
    {
        DeleteProcThreadAttributeList(si.lpAttributeList);
        delete [] (BYTE*)si.lpAttributeList;
    }
};

/** RAII wrapper of PROCESS_INFORMATION. */
struct ProcessInformationWrap
{
    PROCESS_INFORMATION pi = {};

    ~ProcessInformationWrap()
    {
        CloseHandle(pi.hThread);
        pi.hThread = nullptr;
        CloseHandle(pi.hProcess);
        pi.hProcess = nullptr;
    }
};

/** RAII class for temporarily impersonating an AppContainer-isolated process
 *  for the current thread. Intended to be used together with CLSCTX_ENABLE_CLOAKING
 *  when creating COM objects. There's no direct support for AppContainer
 *  impersonation in Windows, so the impl. will instead create a suspended throw-away
 *  process within the AppContainer to use as basis for AppContainer impersonation.
 *  This seem kind of weird, but is the approach recommended by Microsoft when opening
 *  a support case on the matter. Based on "AppContainer Isolation"
 *  https://learn.microsoft.com/en-us/windows/win32/secauthz/appcontainer-isolation */
struct AppContainer : public Sandboxing
{
    AppContainer(const QString &clsid)
    {
        // Create AppContainer sandbox without any special capabilities
        static const wchar_t name[] = L"Qt_testcon";
        static const wchar_t desc[] = L"Qt ActiveQt Test Container";
        HRESULT hr = CreateAppContainerProfile(name, name, desc, nullptr, 0, &m_sid);
        if (HRESULT_CODE(hr) == ERROR_ALREADY_EXISTS)
            hr = DeriveAppContainerSidFromAppContainerName(name, &m_sid); // fallback to existing container
        if (FAILED(hr))
            qFatal("CreateAppContainerProfile and DeriveAppContainerSidFromAppContainerName failed");

        SECURITY_CAPABILITIES sec_cap = {};
        sec_cap.AppContainerSid = m_sid;

        StartupInfoExWrap si;
        si.SetSecurity(&sec_cap);

        // Create suspended COM server process in AppContainer
        QString exe_path = GetExePath(clsid);
        ProcessInformationWrap pi;
        DWORD flags = EXTENDED_STARTUPINFO_PRESENT | CREATE_SUSPENDED; // don't start main thread
        if (!CreateProcess(exe_path.toStdWString().data(), nullptr, nullptr, nullptr,
                           FALSE, flags, nullptr, nullptr, (STARTUPINFO*)&si.si, &pi.pi))
            qFatal("CreateProcess failed");

        // Kill process since we're only interested in the handle for now.
        // The COM runtime will later recreate the process when calling CoCreateInstance.
        TerminateProcess(pi.pi.hProcess, 0);

        // Create AppContainer impersonation token
        HANDLE cur_token = nullptr;
        if (!OpenProcessToken(pi.pi.hProcess,
                              TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY,
                              &cur_token))
            qFatal("OpenProcessToken failed");

        if (!DuplicateTokenEx(cur_token, 0, NULL, SecurityImpersonation, TokenPrimary, &m_token))
            qFatal("DuplicateTokenEx failed");

        CloseHandle(cur_token);
        cur_token = nullptr;

        // Impersonate AppContainer on current thread
        if (!ImpersonateLoggedOnUser(m_token))
            qFatal("ImpersonateLoggedOnUser failed");
    }

    ~AppContainer()
    {
        if (!RevertToSelf())
            qFatal("RevertToSelf failed");

        CloseHandle(m_token);
        m_token = nullptr;

        FreeSid(m_sid);
        m_sid = nullptr;
    }

private:
    /** Get EXE path for a COM class. Input is on "{hex-guid}" format.
     *  Returns empty string if the COM class is DLL-based and on failure. */
    static QString GetExePath (const QString &clsid)
    {
        // extract COM class
        QSettings cls_folder("HKEY_LOCAL_MACHINE\\Software\\Classes\\CLSID\\" + clsid, QSettings::NativeFormat);
        QString command = cls_folder.value("LocalServer32/.").toString();
        if (command.isEmpty())
            return ""; // not exe path found

        // remove any quotes and "/automation" or "-activex" arguments
        QStringList arguments = QProcess::splitCommand(command);
        return arguments[0]; // executable is first argument
    }

    PSID   m_sid = nullptr;
    HANDLE m_token = nullptr;
};

/** RAII class for temporarily impersonating low-integrity level for the current thread.
    Intended to be used together with CLSCTX_ENABLE_CLOAKING when creating COM objects.
    Based on "Designing Applications to Run at a Low Integrity Level" https://msdn.microsoft.com/en-us/library/bb625960.aspx */
class LowIntegrity : public Sandboxing
{
public:
    LowIntegrity()
    {
        HANDLE cur_token = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY, &cur_token))
            qFatal("OpenProcessToken failed");

        if (!DuplicateTokenEx(cur_token, 0, nullptr, SecurityImpersonation, TokenPrimary, &m_token))
            qFatal("DuplicateTokenEx failed");

        CloseHandle(cur_token);

        PSID li_sid = nullptr;
        if (!ConvertStringSidToSid(L"S-1-16-4096", &li_sid)) // low integrity SID
            qFatal("ConvertStringSidToSid failed");

        // reduce process integrity level
        TOKEN_MANDATORY_LABEL TIL = {};
        TIL.Label.Attributes = SE_GROUP_INTEGRITY;
        TIL.Label.Sid = li_sid;
        if (!SetTokenInformation(m_token, TokenIntegrityLevel, &TIL, sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(li_sid)))
            qFatal("SetTokenInformation failed");

        if (!ImpersonateLoggedOnUser(m_token)) // change current thread integrity
            qFatal("ImpersonateLoggedOnUser failed");

        LocalFree(li_sid);
        li_sid = nullptr;
    }

    ~LowIntegrity()
    {
        if (!RevertToSelf())
            qFatal("RevertToSelf failed");

        CloseHandle(m_token);
        m_token = nullptr;
    }

private:
    HANDLE m_token = nullptr;
};

std::unique_ptr<Sandboxing> Sandboxing::Create(QAxSelect::SandboxingLevel level, const QString &clsid)
{
    if (level == QAxSelect::SandboxingLowIntegrity)
        return std::make_unique<LowIntegrity>();
    else if (level == QAxSelect::SandboxingAppContainer)
        return std::make_unique<AppContainer>(clsid);

    Q_ASSERT_X(false, "Sandboxing::Create", "unknown sandboxing level");
    return {};
}
