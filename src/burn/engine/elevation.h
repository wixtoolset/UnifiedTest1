#pragma once
// Copyright (c) .NET Foundation and contributors. All rights reserved. Licensed under the Microsoft Reciprocal License. See LICENSE.TXT file in the project root for full license information.


#ifdef __cplusplus
extern "C" {
#endif


// Parent (per-user process) side functions.
HRESULT ElevationElevate(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent
    );
HRESULT ElevationApplyInitialize(
    __in HANDLE hPipe,
    __in BURN_USER_EXPERIENCE* pBA,
    __in BURN_VARIABLES* pVariables,
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_AU_PAUSE_ACTION auAction,
    __in BOOL fTakeSystemRestorePoint
    );
HRESULT ElevationApplyUninitialize(
    __in HANDLE hPipe
    );
HRESULT ElevationSessionBegin(
    __in HANDLE hPipe,
    __in_z LPCWSTR wzEngineWorkingPath,
    __in_z LPCWSTR wzResumeCommandLine,
    __in BOOL fDisableResume,
    __in BURN_VARIABLES* pVariables,
    __in DWORD dwRegistrationOperations,
    __in BURN_DEPENDENCY_REGISTRATION_ACTION dependencyRegistrationAction,
    __in DWORD64 qwEstimatedSize
    );
HRESULT ElevationSessionResume(
    __in HANDLE hPipe,
    __in_z LPCWSTR wzResumeCommandLine,
    __in BOOL fDisableResume,
    __in BURN_VARIABLES* pVariables
    );
HRESULT ElevationSessionEnd(
    __in HANDLE hPipe,
    __in BURN_RESUME_MODE resumeMode,
    __in BOOTSTRAPPER_APPLY_RESTART restart,
    __in BURN_DEPENDENCY_REGISTRATION_ACTION dependencyRegistrationAction
    );
HRESULT ElevationSaveState(
    __in HANDLE hPipe,
    __in_bcount(cbBuffer) BYTE* pbBuffer,
    __in SIZE_T cbBuffer
    );
HRESULT ElevationCachePreparePackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGE* pPackage
    );
HRESULT ElevationCacheCompletePayload(
    __in HANDLE hPipe,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in_z LPCWSTR wzUnverifiedPath,
    __in BOOL fMove,
    __in PFN_BURNCACHEMESSAGEHANDLER pfnCacheMessageHandler,
    __in LPPROGRESS_ROUTINE pfnProgress,
    __in LPVOID pContext
    );
HRESULT ElevationCacheVerifyPayload(
    __in HANDLE hPipe,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in PFN_BURNCACHEMESSAGEHANDLER pfnCacheMessageHandler,
    __in LPPROGRESS_ROUTINE pfnProgress,
    __in LPVOID pContext
    );
HRESULT ElevationCacheCleanup(
    __in HANDLE hPipe
    );
HRESULT ElevationProcessDependentRegistration(
    __in HANDLE hPipe,
    __in const BURN_DEPENDENT_REGISTRATION_ACTION* pAction
    );
HRESULT ElevationExecuteExePackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_GENERICMESSAGEHANDLER pfnGenericExecuteProgress,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
HRESULT ElevationExecuteMsiPackage(
    __in HANDLE hPipe,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
HRESULT ElevationExecuteMspPackage(
    __in HANDLE hPipe,
    __in_opt HWND hwndParent,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
HRESULT ElevationExecuteMsuPackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BOOL fRollback,
    __in BOOL fStopWusaService,
    __in PFN_GENERICMESSAGEHANDLER pfnGenericExecuteProgress,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
HRESULT ElevationExecutePackageProviderAction(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction
    );
HRESULT ElevationExecutePackageDependencyAction(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction
    );
HRESULT ElevationLaunchElevatedChild(
    __in HANDLE hPipe,
    __in BURN_PACKAGE* pPackage,
    __in LPCWSTR wzPipeName,
    __in LPCWSTR wzPipeToken,
    __out DWORD* pdwChildPid
    );
HRESULT ElevationCleanPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGE* pPackage
    );
HRESULT ElevationLaunchApprovedExe(
    __in HANDLE hPipe,
    __in BURN_LAUNCH_APPROVED_EXE* pLaunchApprovedExe,
    __out DWORD* pdwProcessId
    );

// Child (per-machine process) side functions.
HRESULT ElevationChildPumpMessages(
    __in DWORD dwLoggingTlsId,
    __in HANDLE hPipe,
    __in HANDLE hCachePipe,
    __in BURN_APPROVED_EXES* pApprovedExes,
    __in BURN_CONTAINERS* pContainers,
    __in BURN_PACKAGES* pPackages,
    __in BURN_PAYLOADS* pPayloads,
    __in BURN_VARIABLES* pVariables,
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __out HANDLE* phLock,
    __out BOOL* pfDisabledAutomaticUpdates,
    __out DWORD* pdwChildExitCode,
    __out BOOL* pfRestart
    );
HRESULT ElevationChildResumeAutomaticUpdates();


HRESULT ElevationMsiBeginTransaction(
    __in HANDLE hPipe,
    __in BURN_ROLLBACK_BOUNDARY* pRollbackBoundary
    );
HRESULT ElevationMsiCommitTransaction(
    __in HANDLE hPipe,
    __in BURN_ROLLBACK_BOUNDARY* pRollbackBoundary
    );
HRESULT ElevationMsiRollbackTransaction(
    __in HANDLE hPipe,
    __in BURN_ROLLBACK_BOUNDARY* pRollbackBoundary
    );

#ifdef __cplusplus
}
#endif
