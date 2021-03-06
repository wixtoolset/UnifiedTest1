// Copyright (c) .NET Foundation and contributors. All rights reserved. Licensed under the Microsoft Reciprocal License. See LICENSE.TXT file in the project root for full license information.

#include "precomp.h"

// internal function declarations

static HRESULT LoadIfRelatedBundle(
    __in BOOL fPerMachine,
    __in HKEY hkUninstallKey,
    __in_z LPCWSTR sczRelatedBundleId,
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_RELATED_BUNDLES* pRelatedBundles
    );
static HRESULT DetermineRelationType(
    __in HKEY hkBundleId,
    __in BURN_REGISTRATION* pRegistration,
    __out BOOTSTRAPPER_RELATION_TYPE* pRelationType
    );
static HRESULT LoadRelatedBundleFromKey(
    __in_z LPCWSTR wzRelatedBundleId,
    __in HKEY hkBundleId,
    __in BOOL fPerMachine,
    __in BOOTSTRAPPER_RELATION_TYPE relationType,
    __inout BURN_RELATED_BUNDLE *pRelatedBundle
    );


// function definitions

extern "C" HRESULT RelatedBundlesInitializeForScope(
    __in BOOL fPerMachine,
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_RELATED_BUNDLES* pRelatedBundles
    )
{
    HRESULT hr = S_OK;
    HKEY hkRoot = fPerMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
    HKEY hkUninstallKey = NULL;
    LPWSTR sczRelatedBundleId = NULL;

    hr = RegOpen(hkRoot, BURN_REGISTRATION_REGISTRY_UNINSTALL_KEY, KEY_READ, &hkUninstallKey);
    if (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure(hr, "Failed to open uninstall registry key.");

    for (DWORD dwIndex = 0; /* exit via break below */; ++dwIndex)
    {
        hr = RegKeyEnum(hkUninstallKey, dwIndex, &sczRelatedBundleId);
        if (E_NOMOREITEMS == hr)
        {
            hr = S_OK;
            break;
        }
        ExitOnFailure(hr, "Failed to enumerate uninstall key for related bundles.");

        // If we did not find our bundle id, try to load the subkey as a related bundle.
        if (CSTR_EQUAL != ::CompareStringW(LOCALE_NEUTRAL, NORM_IGNORECASE, sczRelatedBundleId, -1, pRegistration->sczId, -1))
        {
            // Ignore failures here since we'll often find products that aren't actually
            // related bundles (or even bundles at all).
            HRESULT hrRelatedBundle = LoadIfRelatedBundle(fPerMachine, hkUninstallKey, sczRelatedBundleId, pRegistration, pRelatedBundles);
            UNREFERENCED_PARAMETER(hrRelatedBundle);
        }
    }

LExit:
    ReleaseStr(sczRelatedBundleId);
    ReleaseRegKey(hkUninstallKey);

    return hr;
}

extern "C" void RelatedBundlesUninitialize(
    __in BURN_RELATED_BUNDLES* pRelatedBundles
    )
{
    if (pRelatedBundles->rgRelatedBundles)
    {
        for (DWORD i = 0; i < pRelatedBundles->cRelatedBundles; ++i)
        {
            BURN_PACKAGE* pPackage = &pRelatedBundles->rgRelatedBundles[i].package;

            for (DWORD j = 0; j < pPackage->payloads.cItems; ++j)
            {
                PayloadUninitialize(pPackage->payloads.rgItems[j].pPayload);
            }

            PackageUninitialize(pPackage);
            ReleaseStr(pRelatedBundles->rgRelatedBundles[i].sczTag);
        }

        MemFree(pRelatedBundles->rgRelatedBundles);
    }

    memset(pRelatedBundles, 0, sizeof(BURN_RELATED_BUNDLES));
}


// internal helper functions

static HRESULT LoadIfRelatedBundle(
    __in BOOL fPerMachine,
    __in HKEY hkUninstallKey,
    __in_z LPCWSTR sczRelatedBundleId,
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_RELATED_BUNDLES* pRelatedBundles
    )
{
    HRESULT hr = S_OK;
    HKEY hkBundleId = NULL;
    BOOTSTRAPPER_RELATION_TYPE relationType = BOOTSTRAPPER_RELATION_NONE;

    hr = RegOpen(hkUninstallKey, sczRelatedBundleId, KEY_READ, &hkBundleId);
    ExitOnFailure(hr, "Failed to open uninstall key for potential related bundle: %ls", sczRelatedBundleId);

    hr = DetermineRelationType(hkBundleId, pRegistration, &relationType);
    if (FAILED(hr) || BOOTSTRAPPER_RELATION_NONE == relationType)
    {
        // Must not be a related bundle.
        hr = E_NOTFOUND;
    }
    else // load the related bundle.
    {
        hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pRelatedBundles->rgRelatedBundles), pRelatedBundles->cRelatedBundles + 1, sizeof(BURN_RELATED_BUNDLE), 5);
        ExitOnFailure(hr, "Failed to ensure there is space for related bundles.");

        BURN_RELATED_BUNDLE* pRelatedBundle = pRelatedBundles->rgRelatedBundles + pRelatedBundles->cRelatedBundles;

        hr = LoadRelatedBundleFromKey(sczRelatedBundleId, hkBundleId, fPerMachine, relationType, pRelatedBundle);
        ExitOnFailure(hr, "Failed to initialize package from related bundle id: %ls", sczRelatedBundleId);

        ++pRelatedBundles->cRelatedBundles;
    }

LExit:
    ReleaseRegKey(hkBundleId);

    return hr;
}

static HRESULT DetermineRelationType(
    __in HKEY hkBundleId,
    __in BURN_REGISTRATION* pRegistration,
    __out BOOTSTRAPPER_RELATION_TYPE* pRelationType
    )
{
    HRESULT hr = S_OK;
    LPWSTR* rgsczUpgradeCodes = NULL;
    DWORD cUpgradeCodes = 0;
    STRINGDICT_HANDLE sdUpgradeCodes = NULL;
    LPWSTR* rgsczAddonCodes = NULL;
    DWORD cAddonCodes = 0;
    STRINGDICT_HANDLE sdAddonCodes = NULL;
    LPWSTR* rgsczDetectCodes = NULL;
    DWORD cDetectCodes = 0;
    STRINGDICT_HANDLE sdDetectCodes = NULL;
    LPWSTR* rgsczPatchCodes = NULL;
    DWORD cPatchCodes = 0;
    STRINGDICT_HANDLE sdPatchCodes = NULL;

    *pRelationType = BOOTSTRAPPER_RELATION_NONE;

    // All remaining operations should treat all related bundles as non-vital.
    hr = RegReadStringArray(hkBundleId, BURN_REGISTRATION_REGISTRY_BUNDLE_UPGRADE_CODE, &rgsczUpgradeCodes, &cUpgradeCodes);
    if (HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE) == hr)
    {
        TraceError(hr, "Failed to read upgrade codes as REG_MULTI_SZ. Trying again as REG_SZ in case of older bundles.");

        rgsczUpgradeCodes = reinterpret_cast<LPWSTR*>(MemAlloc(sizeof(LPWSTR), TRUE));
        ExitOnNull(rgsczUpgradeCodes, hr, E_OUTOFMEMORY, "Failed to allocate list for a single upgrade code from older bundle.");

        hr = RegReadString(hkBundleId, BURN_REGISTRATION_REGISTRY_BUNDLE_UPGRADE_CODE, &rgsczUpgradeCodes[0]);
        if (SUCCEEDED(hr))
        {
            cUpgradeCodes = 1;
        }
    }

    // Compare upgrade codes.
    if (SUCCEEDED(hr))
    {
        hr = DictCreateStringListFromArray(&sdUpgradeCodes, rgsczUpgradeCodes, cUpgradeCodes, DICT_FLAG_CASEINSENSITIVE);
        ExitOnFailure(hr, "Failed to create string dictionary for %hs.", "upgrade codes");

        // Upgrade relationship: when their upgrade codes match our upgrade codes.
        hr = DictCompareStringListToArray(sdUpgradeCodes, const_cast<LPCWSTR*>(pRegistration->rgsczUpgradeCodes), pRegistration->cUpgradeCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for upgrade code match.");

            *pRelationType = BOOTSTRAPPER_RELATION_UPGRADE;
            ExitFunction();
        }

        // Detect relationship: when their upgrade codes match our detect codes.
        hr = DictCompareStringListToArray(sdUpgradeCodes, const_cast<LPCWSTR*>(pRegistration->rgsczDetectCodes), pRegistration->cDetectCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for detect code match.");

            *pRelationType = BOOTSTRAPPER_RELATION_DETECT;
            ExitFunction();
        }

        // Dependent relationship: when their upgrade codes match our addon codes.
        hr = DictCompareStringListToArray(sdUpgradeCodes, const_cast<LPCWSTR*>(pRegistration->rgsczAddonCodes), pRegistration->cAddonCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for addon code match.");

            *pRelationType = BOOTSTRAPPER_RELATION_DEPENDENT;
            ExitFunction();
        }

        // Dependent relationship: when their upgrade codes match our patch codes.
        hr = DictCompareStringListToArray(sdUpgradeCodes, const_cast<LPCWSTR*>(pRegistration->rgsczPatchCodes), pRegistration->cPatchCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for addon code match.");

            *pRelationType = BOOTSTRAPPER_RELATION_DEPENDENT;
            ExitFunction();
        }

        ReleaseNullDict(sdUpgradeCodes);
        ReleaseNullStrArray(rgsczUpgradeCodes, cUpgradeCodes);
    }

    // Compare addon codes.
    hr = RegReadStringArray(hkBundleId, BURN_REGISTRATION_REGISTRY_BUNDLE_ADDON_CODE, &rgsczAddonCodes, &cAddonCodes);
    if (SUCCEEDED(hr))
    {
        hr = DictCreateStringListFromArray(&sdAddonCodes, rgsczAddonCodes, cAddonCodes, DICT_FLAG_CASEINSENSITIVE);
        ExitOnFailure(hr, "Failed to create string dictionary for %hs.", "addon codes");

        // Addon relationship: when their addon codes match our detect codes.
        hr = DictCompareStringListToArray(sdAddonCodes, const_cast<LPCWSTR*>(pRegistration->rgsczDetectCodes), pRegistration->cDetectCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for addon code match.");

            *pRelationType = BOOTSTRAPPER_RELATION_ADDON;
            ExitFunction();
        }

        // Addon relationship: when their addon codes match our upgrade codes.
        hr = DictCompareStringListToArray(sdAddonCodes, const_cast<LPCWSTR*>(pRegistration->rgsczUpgradeCodes), pRegistration->cUpgradeCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for addon code match.");

            *pRelationType = BOOTSTRAPPER_RELATION_ADDON;
            ExitFunction();
        }

        ReleaseNullDict(sdAddonCodes);
        ReleaseNullStrArray(rgsczAddonCodes, cAddonCodes);
    }

    // Compare patch codes.
    hr = RegReadStringArray(hkBundleId, BURN_REGISTRATION_REGISTRY_BUNDLE_PATCH_CODE, &rgsczPatchCodes, &cPatchCodes);
    if (SUCCEEDED(hr))
    {
        hr = DictCreateStringListFromArray(&sdPatchCodes, rgsczPatchCodes, cPatchCodes, DICT_FLAG_CASEINSENSITIVE);
        ExitOnFailure(hr, "Failed to create string dictionary for %hs.", "patch codes");

        // Patch relationship: when their patch codes match our detect codes.
        hr = DictCompareStringListToArray(sdPatchCodes, const_cast<LPCWSTR*>(pRegistration->rgsczDetectCodes), pRegistration->cDetectCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for patch code match.");

            *pRelationType = BOOTSTRAPPER_RELATION_PATCH;
            ExitFunction();
        }

        // Patch relationship: when their patch codes match our upgrade codes.
        hr = DictCompareStringListToArray(sdPatchCodes, const_cast<LPCWSTR*>(pRegistration->rgsczUpgradeCodes), pRegistration->cUpgradeCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for patch code match.");

            *pRelationType = BOOTSTRAPPER_RELATION_PATCH;
            ExitFunction();
        }

        ReleaseNullDict(sdPatchCodes);
        ReleaseNullStrArray(rgsczPatchCodes, cPatchCodes);
    }

    // Compare detect codes.
    hr = RegReadStringArray(hkBundleId, BURN_REGISTRATION_REGISTRY_BUNDLE_DETECT_CODE, &rgsczDetectCodes, &cDetectCodes);
    if (SUCCEEDED(hr))
    {
        hr = DictCreateStringListFromArray(&sdDetectCodes, rgsczDetectCodes, cDetectCodes, DICT_FLAG_CASEINSENSITIVE);
        ExitOnFailure(hr, "Failed to create string dictionary for %hs.", "detect codes");

        // Detect relationship: when their detect codes match our detect codes.
        hr = DictCompareStringListToArray(sdDetectCodes, const_cast<LPCWSTR*>(pRegistration->rgsczDetectCodes), pRegistration->cDetectCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for detect code match.");

            *pRelationType = BOOTSTRAPPER_RELATION_DETECT;
            ExitFunction();
        }

        // Dependent relationship: when their detect codes match our addon codes.
        hr = DictCompareStringListToArray(sdDetectCodes, const_cast<LPCWSTR*>(pRegistration->rgsczAddonCodes), pRegistration->cAddonCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for addon code match.");

            *pRelationType = BOOTSTRAPPER_RELATION_DEPENDENT;
            ExitFunction();
        }

        // Dependent relationship: when their detect codes match our patch codes.
        hr = DictCompareStringListToArray(sdDetectCodes, const_cast<LPCWSTR*>(pRegistration->rgsczPatchCodes), pRegistration->cPatchCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for addon code match.");

            *pRelationType = BOOTSTRAPPER_RELATION_DEPENDENT;
            ExitFunction();
        }

        ReleaseNullDict(sdDetectCodes);
        ReleaseNullStrArray(rgsczDetectCodes, cDetectCodes);
    }

LExit:
    if (SUCCEEDED(hr) && BOOTSTRAPPER_RELATION_NONE == *pRelationType)
    {
        hr = E_NOTFOUND;
    }

    ReleaseDict(sdUpgradeCodes);
    ReleaseStrArray(rgsczUpgradeCodes, cUpgradeCodes);
    ReleaseDict(sdAddonCodes);
    ReleaseStrArray(rgsczAddonCodes, cAddonCodes);
    ReleaseDict(sdDetectCodes);
    ReleaseStrArray(rgsczDetectCodes, cDetectCodes);
    ReleaseDict(sdPatchCodes);
    ReleaseStrArray(rgsczPatchCodes, cPatchCodes);

    return hr;
}

static HRESULT LoadRelatedBundleFromKey(
    __in_z LPCWSTR wzRelatedBundleId,
    __in HKEY hkBundleId,
    __in BOOL fPerMachine,
    __in BOOTSTRAPPER_RELATION_TYPE relationType,
    __inout BURN_RELATED_BUNDLE* pRelatedBundle
    )
{
    HRESULT hr = S_OK;
    DWORD64 qwEngineVersion = 0;
    LPWSTR sczBundleVersion = NULL;
    LPWSTR sczCachePath = NULL;
    BOOL fCached = FALSE;
    DWORD64 qwFileSize = 0;
    BURN_DEPENDENCY_PROVIDER dependencyProvider = { };

    hr = RegReadVersion(hkBundleId, BURN_REGISTRATION_REGISTRY_ENGINE_VERSION, &qwEngineVersion);
    if (FAILED(hr))
    {
        qwEngineVersion = 0;
        hr = S_OK;
    }

    hr = RegReadString(hkBundleId, BURN_REGISTRATION_REGISTRY_BUNDLE_VERSION, &sczBundleVersion);
    ExitOnFailure(hr, "Failed to read version from registry for bundle: %ls", wzRelatedBundleId);

    hr = VerParseVersion(sczBundleVersion, 0, FALSE, &pRelatedBundle->pVersion);
    ExitOnFailure(hr, "Failed to parse pseudo bundle version: %ls", sczBundleVersion);

    if (pRelatedBundle->pVersion->fInvalid)
    {
        LogId(REPORT_WARNING, MSG_RELATED_PACKAGE_INVALID_VERSION, wzRelatedBundleId, sczBundleVersion);
    }

    hr = RegReadString(hkBundleId, BURN_REGISTRATION_REGISTRY_BUNDLE_CACHE_PATH, &sczCachePath);
    ExitOnFailure(hr, "Failed to read cache path from registry for bundle: %ls", wzRelatedBundleId);

    if (FileExistsEx(sczCachePath, NULL))
    {
        fCached = TRUE;
    }
    else
    {
        LogId(REPORT_STANDARD, MSG_DETECT_RELATED_BUNDLE_NOT_CACHED, wzRelatedBundleId, sczCachePath);
    }

    pRelatedBundle->fPlannable = fCached;

    hr = RegReadString(hkBundleId, BURN_REGISTRATION_REGISTRY_BUNDLE_PROVIDER_KEY, &dependencyProvider.sczKey);
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure(hr, "Failed to read provider key from registry for bundle: %ls", wzRelatedBundleId);

        dependencyProvider.fImported = TRUE;

        hr = StrAllocString(&dependencyProvider.sczVersion, pRelatedBundle->pVersion->sczVersion, 0);
        ExitOnFailure(hr, "Failed to copy version for bundle: %ls", wzRelatedBundleId);

        hr = RegReadString(hkBundleId, BURN_REGISTRATION_REGISTRY_BUNDLE_DISPLAY_NAME, &dependencyProvider.sczDisplayName);
        if (E_FILENOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to copy display name for bundle: %ls", wzRelatedBundleId);
        }
    }

    hr = RegReadString(hkBundleId, BURN_REGISTRATION_REGISTRY_BUNDLE_TAG, &pRelatedBundle->sczTag);
    if (E_FILENOTFOUND == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failed to read tag from registry for bundle: %ls", wzRelatedBundleId);

    pRelatedBundle->relationType = relationType;

    hr = PseudoBundleInitialize(qwEngineVersion, &pRelatedBundle->package, fPerMachine, wzRelatedBundleId, pRelatedBundle->relationType,
                                BOOTSTRAPPER_PACKAGE_STATE_PRESENT, fCached, sczCachePath, sczCachePath, NULL, qwFileSize, FALSE,
                                L"-quiet", L"-repair -quiet", L"-uninstall -quiet",
                                (dependencyProvider.sczKey && *dependencyProvider.sczKey) ? &dependencyProvider : NULL,
                                NULL, 0);
    ExitOnFailure(hr, "Failed to initialize related bundle to represent bundle: %ls", wzRelatedBundleId);

LExit:
    DependencyUninitializeProvider(&dependencyProvider);
    ReleaseStr(sczCachePath);
    ReleaseStr(sczBundleVersion);

    return hr;
}
