@setlocal
@pushd %~dp0

@set _C=Debug
@set _N=nuget_dev.config
@:parse_args
@if /i "%1"=="release" set _C=Release& shift
@if /i "%1"=="official" set _N=nuget_official.config& shift
@if not "%1"=="" shift & goto parse_args

@echo Building internal %_C% using %_N%

:: internal

nuget restore -ConfigFile ..\%_N% || exit /b

:: dotnet pack -c %_C% WixBuildTools.MsgGen\WixBuildTools.MsgGen.csproj || exit /b
dotnet pack --no-restore -c %_C% WixBuildTools.TestSupport\WixBuildTools.TestSupport.csproj || exit /b
:: dotnet pack -c %_C% WixBuildTools.XsdGen\WixBuildTools.XsdGen.csproj || exit /b

msbuild -p:Configuration=%_C% WixBuildTools.TestSupport.Native\WixBuildTools.TestSupport.Native.vcxproj || exit /b

msbuild -p:Configuration=%_C% -t:PackNative WixBuildTools.TestSupport.Native\WixBuildTools.TestSupport.Native.vcxproj || exit /b

@popd
@endlocal
