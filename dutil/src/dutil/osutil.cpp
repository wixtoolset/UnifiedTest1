// Copyright (c) .NET Foundation and contributors. All rights reserved. Licensed under the Microsoft Reciprocal License. See LICENSE.TXT file in the project root for full license information.

#include "precomp.h"


// Exit macros
#define OsExitOnLastError(x, s, ...) ExitOnLastErrorSource(DUTIL_SOURCE_OSUTIL, x, s, __VA_ARGS__)
#define OsExitOnLastErrorDebugTrace(x, s, ...) ExitOnLastErrorDebugTraceSource(DUTIL_SOURCE_OSUTIL, x, s, __VA_ARGS__)
#define OsExitWithLastError(x, s, ...) ExitWithLastErrorSource(DUTIL_SOURCE_OSUTIL, x, s, __VA_ARGS__)
#define OsExitOnFailure(x, s, ...) ExitOnFailureSource(DUTIL_SOURCE_OSUTIL, x, s, __VA_ARGS__)
#define OsExitOnRootFailure(x, s, ...) ExitOnRootFailureSource(DUTIL_SOURCE_OSUTIL, x, s, __VA_ARGS__)
#define OsExitOnFailureDebugTrace(x, s, ...) ExitOnFailureDebugTraceSource(DUTIL_SOURCE_OSUTIL, x, s, __VA_ARGS__)
#define OsExitOnNull(p, x, e, s, ...) ExitOnNullSource(DUTIL_SOURCE_OSUTIL, p, x, e, s, __VA_ARGS__)
#define OsExitOnNullWithLastError(p, x, s, ...) ExitOnNullWithLastErrorSource(DUTIL_SOURCE_OSUTIL, p, x, s, __VA_ARGS__)
#define OsExitOnNullDebugTrace(p, x, e, s, ...)  ExitOnNullDebugTraceSource(DUTIL_SOURCE_OSUTIL, p, x, e, s, __VA_ARGS__)
#define OsExitOnInvalidHandleWithLastError(p, x, s, ...) ExitOnInvalidHandleWithLastErrorSource(DUTIL_SOURCE_OSUTIL, p, x, s, __VA_ARGS__)
#define OsExitOnWin32Error(e, x, s, ...) ExitOnWin32ErrorSource(DUTIL_SOURCE_OSUTIL, e, x, s, __VA_ARGS__)
#define OsExitOnGdipFailure(g, x, s, ...) ExitOnGdipFailureSource(DUTIL_SOURCE_OSUTIL, g, x, s, __VA_ARGS__)

typedef NTSTATUS(NTAPI* PFN_RTL_GET_VERSION)(_Out_ PRTL_OSVERSIONINFOEXW lpVersionInformation);

OS_VERSION vOsVersion = OS_VERSION_UNKNOWN;
DWORD vdwOsServicePack = 0;
RTL_OSVERSIONINFOEXW vovix = { };

/********************************************************************
 OsGetVersion

********************************************************************/
extern "C" void DAPI OsGetVersion(
    __out OS_VERSION* pVersion,
    __out DWORD* pdwServicePack
    )
{
    OSVERSIONINFOEXW ovi = { };

    if (OS_VERSION_UNKNOWN == vOsVersion)
    {
        ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

#pragma warning (push)
#pragma warning(suppress: 4996) // deprecated
#pragma warning (push)
#pragma warning(suppress: 28159)// deprecated, use other function instead
        ::GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&ovi)); // only fails if version info size is set incorrectly.
#pragma warning (pop)
#pragma warning (pop)

        vdwOsServicePack = static_cast<DWORD>(ovi.wServicePackMajor) << 16 | ovi.wServicePackMinor;
        if (4 == ovi.dwMajorVersion)
        {
            vOsVersion = OS_VERSION_WINNT;
        }
        else if (5 == ovi.dwMajorVersion)
        {
            if (0 == ovi.dwMinorVersion)
            {
                vOsVersion = OS_VERSION_WIN2000;
            }
            else if (1 == ovi.dwMinorVersion)
            {
                vOsVersion = OS_VERSION_WINXP;
            }
            else if (2 == ovi.dwMinorVersion)
            {
                vOsVersion = OS_VERSION_WIN2003;
            }
            else
            {
                vOsVersion = OS_VERSION_FUTURE;
            }
        }
        else if (6 == ovi.dwMajorVersion)
        {
            if (0 == ovi.dwMinorVersion)
            {
                vOsVersion = (VER_NT_WORKSTATION == ovi.wProductType) ? OS_VERSION_VISTA : OS_VERSION_WIN2008;
            }
            else if (1 == ovi.dwMinorVersion)
            {
                vOsVersion = (VER_NT_WORKSTATION == ovi.wProductType) ? OS_VERSION_WIN7 : OS_VERSION_WIN2008_R2;
            }
            else
            {
                vOsVersion = OS_VERSION_FUTURE;
            }
        }
        else
        {
            vOsVersion = OS_VERSION_FUTURE;
        }
    }

    *pVersion = vOsVersion;
    *pdwServicePack = vdwOsServicePack;
}

extern "C" HRESULT DAPI OsCouldRunPrivileged(
    __out BOOL* pfPrivileged
    )
{
    HRESULT hr = S_OK;
    BOOL fUacEnabled = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup = NULL;

    // Do a best effort check to see if UAC is enabled on this machine.
    OsIsUacEnabled(&fUacEnabled);

    // If UAC is enabled then the process could run privileged by asking to elevate.
    if (fUacEnabled)
    {
        *pfPrivileged = TRUE;
    }
    else // no UAC so only privilged if user is in administrators group.
    {
        *pfPrivileged = ::AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);
        if (*pfPrivileged)
        {
            if (!::CheckTokenMembership(NULL, AdministratorsGroup, pfPrivileged))
            {
                 *pfPrivileged = FALSE;
            }
        }
    }

    ReleaseSid(AdministratorsGroup);
    return hr;
}

extern "C" HRESULT DAPI OsIsRunningPrivileged(
    __out BOOL* pfPrivileged
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;
    HANDLE hToken = NULL;
    TOKEN_ELEVATION_TYPE elevationType = TokenElevationTypeDefault;
    DWORD dwSize = 0;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup = NULL;

    if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        OsExitOnLastError(hr, "Failed to open process token.");
    }

    if (::GetTokenInformation(hToken, TokenElevationType, &elevationType, sizeof(TOKEN_ELEVATION_TYPE), &dwSize))
    {
        *pfPrivileged = (TokenElevationTypeFull == elevationType);
        ExitFunction1(hr = S_OK);
    }

    // If it's invalid argument, this means they don't support TokenElevationType, and we should fallback to another check
    er = ::GetLastError();
    if (ERROR_INVALID_FUNCTION == er)
    {
        er = ERROR_SUCCESS;
    }
    OsExitOnWin32Error(er, hr, "Failed to get process token information.");

    // Fallback to this check for some OS's (like XP)
    *pfPrivileged = ::AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);
    if (*pfPrivileged)
    {
        if (!::CheckTokenMembership(NULL, AdministratorsGroup, pfPrivileged))
        {
             *pfPrivileged = FALSE;
        }
    }

LExit:
    ReleaseSid(AdministratorsGroup);

    if (hToken)
    {
        ::CloseHandle(hToken);
    }

    return hr;
}

extern "C" HRESULT DAPI OsIsUacEnabled(
    __out BOOL* pfUacEnabled
    )
{
    HRESULT hr = S_OK;
    HKEY hk = NULL;
    DWORD dwUacEnabled = 0;

    *pfUacEnabled = FALSE; // assume UAC not enabled.

    hr = RegOpen(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", KEY_READ, &hk);
    if (E_FILENOTFOUND == hr)
    {
        ExitFunction1(hr = S_OK);
    }
    OsExitOnFailure(hr, "Failed to open system policy key to detect UAC.");

    hr = RegReadNumber(hk, L"EnableLUA", &dwUacEnabled);
    if (E_FILENOTFOUND == hr)
    {
        ExitFunction1(hr = S_OK);
    }
    OsExitOnFailure(hr, "Failed to read registry value to detect UAC.");

    *pfUacEnabled = (0 != dwUacEnabled);

LExit:
    ReleaseRegKey(hk);

    return hr;
}

HRESULT DAPI OsRtlGetVersion(
    __inout RTL_OSVERSIONINFOEXW* pOvix
    )
{
    HRESULT hr = S_OK;
    HMODULE hNtdll = NULL;
    PFN_RTL_GET_VERSION pfnRtlGetVersion = NULL;

    if (vovix.dwOSVersionInfoSize)
    {
        ExitFunction();
    }

    vovix.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

    hr = LoadSystemLibrary(L"ntdll.dll", &hNtdll);
    if (E_MODNOTFOUND == hr)
    {
        OsExitOnRootFailure(hr = E_NOTIMPL, "Failed to load ntdll.dll");
    }
    OsExitOnFailure(hr, "Failed to load ntdll.dll.");

    pfnRtlGetVersion = reinterpret_cast<PFN_RTL_GET_VERSION>(::GetProcAddress(hNtdll, "RtlGetVersion"));
    OsExitOnNullWithLastError(pfnRtlGetVersion, hr, "Failed to locate RtlGetVersion.");

    hr = static_cast<HRESULT>(pfnRtlGetVersion(&vovix));

LExit:
    memcpy(pOvix, &vovix, sizeof(RTL_OSVERSIONINFOEXW));

    if (hNtdll)
    {
        ::FreeLibrary(hNtdll);
    }

    return hr;
}
