@setlocal
@pushd %~dp0

@set _C=Debug
@set _N=nuget_dev.config
@:parse_args
@if /i "%1"=="release" set _C=Release& shift
@if /i "%1"=="official" set _N=nuget_official.config& shift
@if not "%1"=="" shift & goto parse_args

@set _P_OBJ=%~dp0..\..\build\wix\obj\publish\%_C%\
@set _P=%~dp0..\..\build\wix\%_C%\publish\
@set _RCO=/S /R:1 /W:1 /NP /XO  /NS /NC /NFL /NDL /NJH /NJS

@echo Building wix %_C% using %_N%

:: Restore
msbuild -t:Restore -p:Configuration=%_C% -p:RestoreConfigFile=..\%_N% wix.sln || exit /b

:: Build
msbuild -p:Configuration=%_C%;Platform=x86 wixnative\wixnative.vcxproj || exit /b
msbuild -p:Configuration=%_C%;Platform=x64 wixnative\wixnative.vcxproj || exit /b
msbuild -p:Configuration=%_C%;Platform=ARM64 wixnative\wixnative.vcxproj || exit /b

msbuild -p:Configuration=%_C% || exit /b

:: Publish
msbuild -t:Publish -p:Configuration=%_C%;TargetFramework=netcoreapp3.1;PublishDir=%_P%wix\ wix\wix.csproj || exit /b

msbuild -t:Publish -p:Configuration=%_C%;TargetFramework=net461;RuntimeIdentifier=win-x86;PublishDir=%_P_OBJ%WixToolset.Sdk\separate\net461\x86\buildtasks\ WixToolset.BuildTasks\WixToolset.BuildTasks.csproj || exit /b
msbuild -t:Publish -p:Configuration=%_C%;TargetFramework=net461;RuntimeIdentifier=win-x86;PublishDir=%_P_OBJ%WixToolset.Sdk\separate\net461\x86\heat\ heat\heat.csproj || exit /b
msbuild -t:Publish -p:Configuration=%_C%;TargetFramework=net461;RuntimeIdentifier=win-x86;PublishDir=%_P_OBJ%WixToolset.Sdk\separate\net461\x86\wix\ wix\wix.csproj || exit /b
robocopy %_P_OBJ%\WixToolset.Sdk\separate\net461\x86\buildtasks %_P%\WixToolset.Sdk\tools\net461\x86 %_RCO% /XF Microsoft.Build.*.dll
robocopy %_P_OBJ%\WixToolset.Sdk\separate\net461\x86\heat %_P%\WixToolset.Sdk\tools\net461\x86 %_RCO%
robocopy %_P_OBJ%\WixToolset.Sdk\separate\net461\x86\wix %_P%\WixToolset.Sdk\tools\net461\x86 %_RCO%

msbuild -t:Publish -p:Configuration=%_C%;TargetFramework=net461;RuntimeIdentifier=win-x64;PublishDir=%_P_OBJ%WixToolset.Sdk\separate\net461\x64\buildtasks\ WixToolset.BuildTasks\WixToolset.BuildTasks.csproj || exit /b
msbuild -t:Publish -p:Configuration=%_C%;TargetFramework=net461;RuntimeIdentifier=win-x64;PublishDir=%_P_OBJ%WixToolset.Sdk\separate\net461\x64\heat\ heat\heat.csproj || exit /b
msbuild -t:Publish -p:Configuration=%_C%;TargetFramework=net461;RuntimeIdentifier=win-x64;PublishDir=%_P_OBJ%WixToolset.Sdk\separate\net461\x64\wix\ wix\wix.csproj || exit /b
robocopy %_P_OBJ%\WixToolset.Sdk\separate\net461\x64\buildtasks %_P%\WixToolset.Sdk\tools\net461\x64 %_RCO% /XF Microsoft.Build.*.dll
robocopy %_P_OBJ%\WixToolset.Sdk\separate\net461\x64\heat %_P%\WixToolset.Sdk\tools\net461\x64 %_RCO%
robocopy %_P_OBJ%\WixToolset.Sdk\separate\net461\x64\wix %_P%\WixToolset.Sdk\tools\net461\x64 %_RCO%

msbuild -t:Publish -p:Configuration=%_C%;TargetFramework=netcoreapp3.1;UseAppHost=false;PublishDir=%_P_OBJ%WixToolset.Sdk\separate\netcoreapp3.1\buildtasks\ WixToolset.BuildTasks\WixToolset.BuildTasks.csproj || exit /b
msbuild -t:Publish -p:Configuration=%_C%;TargetFramework=netcoreapp3.1;UseAppHost=false;PublishDir=%_P_OBJ%WixToolset.Sdk\separate\netcoreapp3.1\heat\ heat\heat.csproj || exit /b
msbuild -t:Publish -p:Configuration=%_C%;TargetFramework=netcoreapp3.1;UseAppHost=false;PublishDir=%_P_OBJ%WixToolset.Sdk\separate\netcoreapp3.1\wix\ wix\wix.csproj || exit /b
robocopy %_P_OBJ%\WixToolset.Sdk\separate\netcoreapp3.1\buildtasks %_P%\WixToolset.Sdk\tools\netcoreapp3.1 %_RCO% /XF Microsoft.Build.*.dll
robocopy %_P_OBJ%\WixToolset.Sdk\separate\netcoreapp3.1\heat %_P%\WixToolset.Sdk\tools\netcoreapp3.1 %_RCO%
robocopy %_P_OBJ%\WixToolset.Sdk\separate\netcoreapp3.1\wix %_P%\WixToolset.Sdk\tools\netcoreapp3.1 %_RCO%

msbuild -t:Publish -p:Configuration=%_C%;PublishDir=%_P%WixToolset.Sdk\ WixToolset.Sdk\WixToolset.Sdk.csproj || exit /b

:: TODO - used by MsbuildFixture.ReportsInnerExceptionForUnexpectedExceptions test
:: msbuild -t:Publish -Restore -p:Configuration=%_C%;TargetFramework=net461;RuntimeIdentifier=linux-x86;PublishDir=%_P%WixToolset.Sdk\broken\net461\ wix\wix.csproj || exit /b

:: Test
dotnet test -c %_C% --no-build test\WixToolsetTest.BuildTasks || exit /b
dotnet test -c %_C% --no-build test\WixToolsetTest.Sdk || exit /b

:: Pack
dotnet pack --no-build -c %_C% -f netcoreapp3.1 wix || exit /b
dotnet pack -c %_C% WixToolset.Sdk || exit /b


@popd
@endlocal
